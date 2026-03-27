/* Copyright 2021 Aristocratos (jakob@qvantnet.com)
   Copyright 2024 IRIX port

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

	   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

indent = tab
tab-size = 4
*/

//? IRIX platform backend for btop++
//? Uses sysmp(), /proc (binary prpsinfo), statvfs, swapctl, getmntent

#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <mntent.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pwd.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/swap.h>
#include <sys/sysmp.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <sys/procfs.h>
#include <sys/times.h>
#include <utmpx.h>
#include <unistd.h>

#include <stdexcept>
#include <cmath>
#include <fstream>
#include <numeric>
#include <ranges>  // provided by mogrix polyfill
#include <regex>
#include <string>
#include <memory>
#include <utility>

// IRIX lacks getloadavg() — stub that returns zeros
// (could parse uptime output or use sysmp, but load average
// is a cosmetic metric and not worth the complexity)
static int getloadavg(double loadavg[], int nelem) {
    for (int i = 0; i < nelem; i++) loadavg[i] = 0.0;
    return nelem;
}

#include "../btop_config.hpp"
#include "../btop_shared.hpp"
#include "../btop_tools.hpp"

using std::clamp, std::string_literals::operator""s, std::cmp_equal, std::cmp_less, std::cmp_greater;
using std::ifstream, std::numeric_limits, std::streamsize, std::round, std::max, std::min;
namespace fs = std::filesystem;
namespace rng = std::ranges;
using namespace Tools;

//? --------------------------------------------------- FUNCTIONS -----------------------------------------------------

namespace Cpu {
	vector<long long> core_old_totals;
	vector<long long> core_old_idles;
	vector<string> available_fields = {"Auto", "total"};
	vector<string> available_sensors = {"Auto"};
	cpu_info current_cpu;
	bool got_sensors = false, cpu_temp_only = false;

	//* Populate found_sensors map
	bool get_sensors();

	//* Get current cpu clock speed
	string get_cpuHz();

	//* Get CPU name from uname or hinv
	string get_cpuName();

	struct Sensor {
		fs::path path;
		string label;
		int64_t temp = 0;
		int64_t high = 0;
		int64_t crit = 0;
	};

	string cpu_sensor;
	vector<string> core_sensors;
	std::unordered_map<int, int> core_mapping;
}  // namespace Cpu

namespace Mem {
	double old_uptime;
}

namespace Shared {

	fs::path passwd_path;
	uint64_t totalMem;
	long pageSize, clkTck, coreCount, physicalCoreCount, arg_max;
	int totalMem_len;
	long bootTime;

	void init() {
		//? Get number of CPUs via sysmp
		coreCount = sysmp(MP_NPROCS);
		if (coreCount < 1) {
			Logger::warning("Could not determine number of cores, defaulting to 1.");
			coreCount = 1;
		}

		pageSize = sysconf(_SC_PAGE_SIZE);
		if (pageSize <= 0) {
			pageSize = 4096;
			Logger::warning("Could not get system page size. Defaulting to 4096, processes memory usage might be incorrect.");
		}

		clkTck = sysconf(_SC_CLK_TCK);
		if (clkTck <= 0) {
			clkTck = 100;
			Logger::warning("Could not get system clock ticks per second. Defaulting to 100, processes cpu usage might be incorrect.");
		}

		//? Get total physical memory via sysmp MPSA_RMINFO
		struct rminfo rmi;
		if (sysmp(MP_SAGET, MPSA_RMINFO, &rmi, sizeof(rmi)) == -1) {
			Logger::warning("Could not get memory info via sysmp(MPSA_RMINFO)");
			totalMem = 256ULL * 1024 * 1024; // fallback 256MB
		} else {
			totalMem = (uint64_t)rmi.physmem * pageSize;
		}

		//? Get boot time from /proc/0 (sched process) or utmpx
		//? IRIX doesn't have a simple sysctl for this; use utmpx or estimate
		{
			struct utmpx *ut;
			setutxent();
			bootTime = 0;
			while ((ut = getutxent()) != nullptr) {
				if (ut->ut_type == BOOT_TIME) {
					bootTime = ut->ut_tv.tv_sec;
					break;
				}
			}
			endutxent();
			if (bootTime == 0) {
				//? Fallback: estimate from current time and uptime
				bootTime = time(nullptr) - (long)system_uptime();
			}
		}

		physicalCoreCount = coreCount;

		//* Get maximum length of process arguments
		arg_max = sysconf(_SC_ARG_MAX);
		if (arg_max <= 0) arg_max = 4096;

		//? Init for namespace Cpu
		Cpu::current_cpu.core_percent.insert(Cpu::current_cpu.core_percent.begin(), Shared::coreCount, {});
		Cpu::current_cpu.temp.insert(Cpu::current_cpu.temp.begin(), Shared::coreCount + 1, {});
		Cpu::core_old_totals.insert(Cpu::core_old_totals.begin(), Shared::coreCount, 0);
		Cpu::core_old_idles.insert(Cpu::core_old_idles.begin(), Shared::coreCount, 0);
		Logger::debug("Init -> Cpu::collect()");
		Cpu::collect();
		for (auto &[field, vec] : Cpu::current_cpu.cpu_percent) {
			if (not vec.empty() and not v_contains(Cpu::available_fields, field)) Cpu::available_fields.push_back(field);
		}
		Logger::debug("Init -> Cpu::get_cpuName()");
		Cpu::cpuName = Cpu::get_cpuName();
		Logger::debug("Init -> Cpu::get_sensors()");
		Cpu::got_sensors = Cpu::get_sensors();
		Logger::debug("Init -> Cpu::get_core_mapping()");
		Cpu::core_mapping = Cpu::get_core_mapping();

		//? Init for namespace Mem
		Mem::old_uptime = system_uptime();
		Logger::debug("Init -> Mem::collect()");
		Mem::collect();
	}
}  // namespace Shared

namespace Cpu {
	string cpuName;
	string cpuHz;
	bool has_battery = false;
	tuple<int, float, long, string> current_bat;

	const array<string, 10> time_names = {"user", "nice", "system", "idle", "iowait"};

	std::unordered_map<string, long long> cpu_old = {
		{"totals", 0},
		{"idles", 0},
		{"user", 0},
		{"nice", 0},
		{"system", 0},
		{"idle", 0},
		{"iowait", 0}
	};

	string get_cpuName() {
		string name;
		struct utsname uts;
		if (uname(&uts) == 0) {
			name = string(uts.machine);
		}

		//? Try to get more detail from hinv-style info via sysmp
		//? On IRIX, /hw/cpunum/0 or hinv output gives CPU type
		//? Fall back to uname machine field (e.g. "IP35", "IP53")
		if (name.empty()) {
			name = "MIPS";
		}

		//? Try reading /proc/cpuinfo-style data from IRIX inventory
		//? IRIX inventory is accessed via getinvent() but that requires
		//? libinvent headers; use uname output as reasonable fallback
		string arch = name;

		//? Map common IRIX platform names to CPU names
		if (name.find("IP") == 0) {
			//? IP22=Indy/Indigo2 (R4x00/R5000), IP27=Origin (R10000/R12000),
			//? IP28=Indigo2 (R10000), IP30=Octane (R10000/R12000/R14000),
			//? IP32=O2 (R5000/R10000/R12000), IP35=Tezro/Fuel/Origin3k
			name = "MIPS (" + arch + ")";
		}

		return name;
	}

	bool get_sensors() {
		//? IRIX does not expose per-CPU temperature sensors via standard APIs.
		//? Some machines have environmental monitoring but it requires
		//? proprietary interfaces. Return false.
		got_sensors = false;
		return false;
	}

	void update_sensors() {
		//? No-op on IRIX — no temperature sensors available via standard API
	}

	string get_cpuHz() {
		//? IRIX doesn't have a portable way to query CPU frequency at runtime.
		//? Some platforms support it via inventory, but for safety return empty.
		return "";
	}

	auto get_core_mapping() -> std::unordered_map<int, int> {
		std::unordered_map<int, int> core_map;
		if (cpu_temp_only) return core_map;

		for (long i = 0; i < Shared::coreCount; i++) {
			core_map[i] = i;
		}

		//? Apply user set custom mapping if any
		const auto &custom_map = Config::getS("cpu_core_map");
		if (not custom_map.empty()) {
			try {
				for (const auto &split : ssplit(custom_map)) {
					const auto vals = ssplit(split, ':');
					if (vals.size() != 2) continue;
					int change_id = std::stoi(vals.at(0));
					int new_id = std::stoi(vals.at(1));
					if (not core_map.contains(change_id) or cmp_greater(new_id, core_sensors.size())) continue;
					core_map.at(change_id) = new_id;
				}
			} catch (...) {
			}
		}

		return core_map;
	}

	auto get_battery() -> tuple<int, float, long, string> {
		//? IRIX machines (SGI workstations/servers) do not have batteries
		has_battery = false;
		return {0, 0, 0, ""};
	}

	auto collect(bool no_update) -> cpu_info & {
		if (Runner::stopping or (no_update and not current_cpu.cpu_percent.at("total").empty()))
			return current_cpu;
		auto &cpu = current_cpu;

		if (getloadavg(cpu.load_avg.data(), cpu.load_avg.size()) < 0) {
			Logger::error("failed to get load averages");
		}

		//? Collect per-CPU stats using sysmp(MP_SAGET, MPSA_SINFO)
		//? IRIX struct sysinfo has cpu[] array with indices:
		//?   CPU_IDLE, CPU_USER, CPU_KERNEL, CPU_WAIT, CPU_SXBRK
		long long global_totals = 0;
		long long global_idles = 0;
		//? time_names: user, nice, system, idle, iowait
		//? IRIX maps: user=CPU_USER, nice=0(no nice tracking), system=CPU_KERNEL, idle=CPU_IDLE, iowait=CPU_WAIT
		vector<long long> times_summed(5, 0);

		for (long i = 0; i < Shared::coreCount; i++) {
			struct sysinfo si;
			memset(&si, 0, sizeof(si));

			//? IRIX sysmp can get per-CPU info using MPSA_SINFO with a CPU number
			//? The per-CPU variant: sysmp(MP_SAGET1, MPSA_SINFO, &si, sizeof(si), cpu_id)
			if (sysmp(MP_SAGET1, MPSA_SINFO, &si, sizeof(si), i) == -1) {
				//? Fallback: if MP_SAGET1 is not available, get aggregate only
				if (i == 0) {
					if (sysmp(MP_SAGET, MPSA_SINFO, &si, sizeof(si)) == -1) {
						Logger::error("sysmp(MPSA_SINFO) failed");
						continue;
					}
				} else {
					continue;
				}
			}

			long long user_t  = si.cpu[CPU_USER];
			long long nice_t  = 0;  // IRIX doesn't track nice separately
			long long sys_t   = si.cpu[CPU_KERNEL];
			long long idle_t  = si.cpu[CPU_IDLE];
			long long wait_t  = si.cpu[CPU_WAIT];

			vector<long long> times = {user_t, nice_t, sys_t, idle_t, wait_t};

			times_summed[0] += user_t;
			times_summed[1] += nice_t;
			times_summed[2] += sys_t;
			times_summed[3] += idle_t;
			times_summed[4] += wait_t;

			try {
				//? All values
				const long long totals = std::accumulate(times.begin(), times.end(), 0ll);

				//? Idle time (idle + wait)
				const long long idles = idle_t + wait_t;

				global_totals += totals;
				global_idles += idles;

				//? Calculate cpu total for each core
				if (i > Shared::coreCount) break;
				const long long calc_totals = max(0ll, totals - core_old_totals.at(i));
				const long long calc_idles = max(0ll, idles - core_old_idles.at(i));
				core_old_totals.at(i) = totals;
				core_old_idles.at(i) = idles;

				if (calc_totals > 0) {
					cpu.core_percent.at(i).push_back(clamp((long long)round((double)(calc_totals - calc_idles) * 100 / calc_totals), 0ll, 100ll));
				} else {
					cpu.core_percent.at(i).push_back(0ll);
				}

				//? Reduce size if there are more values than needed for graph
				if (cpu.core_percent.at(i).size() > 40) cpu.core_percent.at(i).pop_front();

			} catch (const std::exception &e) {
				Logger::error("Cpu::collect() : " + (string)e.what());
				throw std::runtime_error("collect() : " + (string)e.what());
			}
		}

		const long long calc_totals = max(1ll, global_totals - cpu_old.at("totals"));
		const long long calc_idles = max(1ll, global_idles - cpu_old.at("idles"));

		//? Populate cpu.cpu_percent with all fields
		for (int ii = 0; ii < 5; ii++) {
			const auto &tname = time_names.at(ii);
			long long val = times_summed[ii];
			long long old_val = cpu_old.count(tname) ? cpu_old.at(tname) : 0;

			if (calc_totals > 0) {
				cpu.cpu_percent.at(tname).push_back(clamp((long long)round((double)(val - old_val) * 100 / calc_totals), 0ll, 100ll));
			} else {
				cpu.cpu_percent.at(tname).push_back(0ll);
			}
			cpu_old[tname] = val;

			//? Reduce size if there are more values than needed for graph
			while (cmp_greater(cpu.cpu_percent.at(tname).size(), width * 2)) cpu.cpu_percent.at(tname).pop_front();
		}

		cpu_old.at("totals") = global_totals;
		cpu_old.at("idles") = global_idles;

		//? Total usage of cpu
		if (calc_totals > 0) {
			cpu.cpu_percent.at("total").push_back(clamp((long long)round((double)(calc_totals - calc_idles) * 100 / calc_totals), 0ll, 100ll));
		} else {
			cpu.cpu_percent.at("total").push_back(0ll);
		}

		//? Reduce size if there are more values than needed for graph
		while (cmp_greater(cpu.cpu_percent.at("total").size(), width * 2)) cpu.cpu_percent.at("total").pop_front();

		if (Config::getB("show_cpu_freq")) {
			auto hz = get_cpuHz();
			if (hz != "") {
				cpuHz = hz;
			}
		}

		if (Config::getB("check_temp") and got_sensors)
			update_sensors();

		if (Config::getB("show_battery") and has_battery)
			current_bat = get_battery();

		return cpu;
	}
}  // namespace Cpu

namespace Mem {
	bool has_swap = false;
	vector<string> fstab;
	fs::file_time_type fstab_time;
	int disk_ios = 0;
	vector<string> last_found;

	mem_info current_mem{};

	uint64_t get_totalMem() {
		return Shared::totalMem;
	}

	auto collect(bool no_update) -> mem_info & {
		if (Runner::stopping or (no_update and not current_mem.percent.at("used").empty()))
			return current_mem;

		auto show_swap = Config::getB("show_swap");
		auto show_disks = Config::getB("show_disks");
		auto swap_disk = Config::getB("swap_disk");
		auto &mem = current_mem;

		//? Get memory stats via sysmp(MP_SAGET, MPSA_RMINFO)
		struct rminfo rmi;
		if (sysmp(MP_SAGET, MPSA_RMINFO, &rmi, sizeof(rmi)) != -1) {
			uint64_t physmem  = (uint64_t)rmi.physmem * Shared::pageSize;
			uint64_t freemem  = (uint64_t)rmi.freemem * Shared::pageSize;
			uint64_t bufmem   = (uint64_t)rmi.bufmem * Shared::pageSize;
			// availsmem is available swap+memory pages
			// uint64_t availsmem = (uint64_t)rmi.availsmem * Shared::pageSize;

			mem.stats.at("free") = freemem;
			mem.stats.at("cached") = bufmem;
			mem.stats.at("used") = physmem - freemem - bufmem;
			mem.stats.at("available") = freemem + bufmem;
		} else {
			Logger::warning("Failed to get memory info via sysmp(MPSA_RMINFO)");
		}

		//? Get swap info via swapctl
		if (show_swap) {
			int nswap = swapctl(SC_GETNSWP, nullptr);
			if (nswap > 0) {
				//? Allocate space for swaptable
				size_t tbl_size = sizeof(swaptbl_t) + (nswap - 1) * sizeof(swapent_t);
				auto tbl = (swaptbl_t *)malloc(tbl_size);
				if (tbl) {
					//? Each swapent needs a path buffer
					char **paths = (char **)malloc(nswap * sizeof(char *));
					for (int i = 0; i < nswap; i++) {
						paths[i] = (char *)malloc(PATH_MAX);
						tbl->swt_ent[i].ste_path = paths[i];
					}
					tbl->swt_n = nswap;

					int ret = swapctl(SC_LIST, tbl);
					if (ret >= 0) {
						int64_t total_pages = 0, free_pages = 0;
						for (int i = 0; i < ret; i++) {
							total_pages += tbl->swt_ent[i].ste_pages;
							free_pages += tbl->swt_ent[i].ste_free;
						}
						mem.stats.at("swap_total") = total_pages * Shared::pageSize;
						mem.stats.at("swap_used") = (total_pages - free_pages) * Shared::pageSize;
						mem.stats.at("swap_free") = free_pages * Shared::pageSize;
					}

					for (int i = 0; i < nswap; i++) free(paths[i]);
					free(paths);
					free(tbl);
				}
			} else if (nswap == 0) {
				mem.stats.at("swap_total") = 0;
				mem.stats.at("swap_used") = 0;
				mem.stats.at("swap_free") = 0;
			}
		}

		if (show_swap and mem.stats.at("swap_total") > 0) {
			for (const auto &name : swap_names) {
				mem.percent.at(name).push_back(round((double)mem.stats.at(name) * 100 / mem.stats.at("swap_total")));
				while (cmp_greater(mem.percent.at(name).size(), width * 2))
					mem.percent.at(name).pop_front();
			}
			has_swap = true;
		} else
			has_swap = false;

		//? Calculate percentages
		for (const auto &name : mem_names) {
			if (Shared::totalMem > 0) {
				mem.percent.at(name).push_back(round((double)mem.stats.at(name) * 100 / Shared::totalMem));
			} else {
				mem.percent.at(name).push_back(0);
			}
			while (cmp_greater(mem.percent.at(name).size(), width * 2))
				mem.percent.at(name).pop_front();
		}

		if (show_disks) {
			double uptime = system_uptime();
			auto &disks_filter = Config::getS("disks_filter");
			bool filter_exclude = false;
			auto &disks = mem.disks;
			vector<string> filter;
			if (not disks_filter.empty()) {
				filter = ssplit(disks_filter);
				if (filter.at(0).starts_with("exclude=")) {
					filter_exclude = true;
					filter.at(0) = filter.at(0).substr(8);
				}
			}

			//? Read mount table using getmntent (IRIX uses /etc/mtab or /etc/fstab)
			vector<string> found;
			found.reserve(last_found.size());

			FILE *mtab = setmntent("/etc/mtab", "r");
			if (mtab == nullptr) {
				//? Fallback to /etc/fstab
				mtab = setmntent("/etc/fstab", "r");
			}

			if (mtab != nullptr) {
				struct mntent *ent;
				while ((ent = getmntent(mtab)) != nullptr) {
					string fstype = ent->mnt_type;
					string mountpoint = ent->mnt_dir;
					string dev = ent->mnt_fsname;

					//? Skip virtual/pseudo filesystems
					if (fstype == "proc" || fstype == "procfs" || fstype == "tmpfs" ||
						fstype == "devfs" || fstype == "fd" || fstype == "swap" ||
						fstype == "ignore" || fstype == "autofs" || fstype == "hwgfs" ||
						mountpoint == "/proc" || mountpoint == "/dev/fd") {
						continue;
					}

					//? Match filter if not empty
					if (not filter.empty()) {
						bool match = v_contains(filter, mountpoint);
						if ((filter_exclude and match) or (not filter_exclude and not match))
							continue;
					}

					found.push_back(mountpoint);
					if (not disks.contains(mountpoint)) {
						std::error_code ec;
						disks[mountpoint] = disk_info{fs::canonical(dev, ec), fs::path(mountpoint).filename()};

						if (disks.at(mountpoint).dev.empty())
							disks.at(mountpoint).dev = dev;

						if (disks.at(mountpoint).name.empty())
							disks.at(mountpoint).name = (mountpoint == "/" ? "root" : mountpoint);
					}

					if (not v_contains(last_found, mountpoint))
						redraw = true;
				}
				endmntent(mtab);
			}

			//? Remove disks no longer mounted or filtered out
			if (swap_disk and has_swap) found.push_back("swap");
			for (auto it = disks.begin(); it != disks.end();) {
				if (not v_contains(found, it->first))
					it = disks.erase(it);
				else
					it++;
			}
			if (found.size() != last_found.size()) redraw = true;
			last_found = std::move(found);

			//? Get disk/partition stats via statvfs
			for (auto &[mountpoint, disk] : disks) {
				if (mountpoint == "swap") continue;
				if (std::error_code ec; not fs::exists(mountpoint, ec))
					continue;
				struct statvfs vfs;
				if (statvfs(mountpoint.c_str(), &vfs) < 0) {
					Logger::warning("Failed to get disk/partition stats with statvfs() for: " + mountpoint);
					continue;
				}
				disk.total = (int64_t)vfs.f_blocks * vfs.f_frsize;
				disk.free = (int64_t)vfs.f_bfree * vfs.f_frsize;
				disk.used = disk.total - disk.free;
				if (disk.total > 0) {
					disk.used_percent = round((double)disk.used * 100 / disk.total);
					disk.free_percent = 100 - disk.used_percent;
				}
			}

			//? Setup disks order in UI and add swap if enabled
			mem.disks_order.clear();
			if (disks.contains("/"))
				mem.disks_order.push_back("/");
			if (swap_disk and has_swap) {
				mem.disks_order.push_back("swap");
				if (not disks.contains("swap"))
					disks["swap"] = {"", "swap"};
				disks.at("swap").total = mem.stats.at("swap_total");
				disks.at("swap").used = mem.stats.at("swap_used");
				disks.at("swap").free = mem.stats.at("swap_free");
				disks.at("swap").used_percent = mem.percent.at("swap_used").back();
				disks.at("swap").free_percent = mem.percent.at("swap_free").back();
			}
			for (const auto &name : last_found)
				if (not is_in(name, "/", "swap"))
					mem.disks_order.push_back(name);

			//? No disk I/O stats on IRIX via standard APIs — push zero
			disk_ios = 0;
			for (auto &[mountpoint, disk] : disks) {
				if (disk.io_read.empty()) disk.io_read.push_back(0);
				if (disk.io_write.empty()) disk.io_write.push_back(0);
				if (disk.io_activity.empty()) disk.io_activity.push_back(0);
			}

			old_uptime = uptime;
		}
		return mem;
	}

}  // namespace Mem

namespace Net {
	std::unordered_map<string, net_info> current_net;
	net_info empty_net = {};
	vector<string> interfaces;
	string selected_iface;
	int errors = 0;
	std::unordered_map<string, uint64_t> graph_max = {{"download", {}}, {"upload", {}}};
	std::unordered_map<string, array<int, 2>> max_count = {{"download", {}}, {"upload", {}}};
	bool rescale = true;
	uint64_t timestamp = 0;

	auto collect(bool no_update) -> net_info & {
		auto &net = current_net;
		auto &config_iface = Config::getS("net_iface");
		auto net_sync = Config::getB("net_sync");
		auto net_auto = Config::getB("net_auto");
		auto new_timestamp = time_ms();

		if (not no_update and errors < 3) {
			//? Get interface list using getifaddrs() wrapper
			IfAddrsPtr if_addrs {};
			if (if_addrs.get_status() != 0) {
				errors++;
				Logger::error("Net::collect() -> getifaddrs() failed with id " + to_string(if_addrs.get_status()));
				redraw = true;
				return empty_net;
			}
			int family = 0;
			char ip[INET6_ADDRSTRLEN];
			interfaces.clear();

			//? Iteration over all items in getifaddrs() list
			for (auto *ifa = if_addrs.get(); ifa != nullptr; ifa = ifa->ifa_next) {
				if (ifa->ifa_addr == nullptr) continue;
				family = ifa->ifa_addr->sa_family;
				const auto &iface = ifa->ifa_name;

				//? Update available interfaces vector and get status of interface
				if (not v_contains(interfaces, iface)) {
					interfaces.push_back(iface);
					net[iface].connected = (ifa->ifa_flags & IFF_RUNNING);
					net[iface].ipv4.clear();
					net[iface].ipv6.clear();
				}

				//? Get IPv4 address
				if (family == AF_INET) {
					if (net[iface].ipv4.empty()) {
						if (nullptr != inet_ntop(family, &(reinterpret_cast<struct sockaddr_in*>(ifa->ifa_addr)->sin_addr), ip, sizeof(ip))) {
							net[iface].ipv4 = ip;
						}
					}
				}
				//? Get IPv6 address
				else if (family == AF_INET6) {
					if (net[iface].ipv6.empty()) {
						if (nullptr != inet_ntop(family, &(reinterpret_cast<struct sockaddr_in6*>(ifa->ifa_addr)->sin6_addr), ip, sizeof(ip))) {
							net[iface].ipv6 = ip;
						}
					}
				}
			}

			//? IRIX doesn't provide per-interface byte counters via getifaddrs or sysctl
			//? like FreeBSD does. Network bandwidth tracking is limited.
			//? We push zero-speed entries to keep the UI from crashing.
			for (const auto &iface : interfaces) {
				for (const string dir : {"download", "upload"}) {
					auto &saved_stat = net.at(iface).stat.at(dir);
					auto &bandwidth = net.at(iface).bandwidth.at(dir);

					//? Without byte counters, report zero speed
					saved_stat.speed = 0;
					saved_stat.total = 0;

					//? Add values to graph
					bandwidth.push_back(saved_stat.speed);
					while (cmp_greater(bandwidth.size(), width * 2)) bandwidth.pop_front();
				}
			}

			//? Clean up net map if needed
			if (net.size() > interfaces.size()) {
				for (auto it = net.begin(); it != net.end();) {
					if (not v_contains(interfaces, it->first))
						it = net.erase(it);
					else
						it++;
				}
			}

			timestamp = new_timestamp;
		}

		//? Return empty net_info struct if no interfaces was found
		if (net.empty())
			return empty_net;

		//? Find an interface to display if selected isn't set or valid
		if (selected_iface.empty() or not v_contains(interfaces, selected_iface)) {
			max_count["download"][0] = max_count["download"][1] = max_count["upload"][0] = max_count["upload"][1] = 0;
			redraw = true;
			if (net_auto) rescale = true;
			if (not config_iface.empty() and v_contains(interfaces, config_iface))
				selected_iface = config_iface;
			else {
				//? Sort interfaces by total upload + download bytes
				auto sorted_interfaces = interfaces;
				rng::sort(sorted_interfaces, [&](const auto &a, const auto &b) {
					return cmp_greater(net.at(a).stat["download"].total + net.at(a).stat["upload"].total,
									   net.at(b).stat["download"].total + net.at(b).stat["upload"].total);
				});
				selected_iface.clear();
				//? Try to set to a connected interface
				for (const auto &iface : sorted_interfaces) {
					if (net.at(iface).connected) selected_iface = iface;
					break;
				}
				//? If no interface is connected set to first available
				if (selected_iface.empty() and not sorted_interfaces.empty())
					selected_iface = sorted_interfaces.at(0);
				else if (sorted_interfaces.empty())
					return empty_net;
			}
		}

		//? Calculate max scale for graphs if needed
		if (net_auto) {
			bool sync = false;
			for (const auto &dir : {"download", "upload"}) {
				for (const auto &sel : {0, 1}) {
					if (rescale or max_count[dir][sel] >= 5) {
						const long long avg_speed = (net[selected_iface].bandwidth[dir].size() > 5
														? std::accumulate(net.at(selected_iface).bandwidth.at(dir).rbegin(), net.at(selected_iface).bandwidth.at(dir).rbegin() + 5, 0ll) / 5
														: net[selected_iface].stat[dir].speed);
						graph_max[dir] = max(uint64_t(avg_speed * (sel == 0 ? 1.3 : 3.0)), (uint64_t)10 << 10);
						max_count[dir][0] = max_count[dir][1] = 0;
						redraw = true;
						if (net_sync) sync = true;
						break;
					}
				}
				//? Sync download/upload graphs if enabled
				if (sync) {
					const auto other = (string(dir) == "upload" ? "download" : "upload");
					graph_max[other] = graph_max[dir];
					max_count[other][0] = max_count[other][1] = 0;
					break;
				}
			}
		}

		rescale = false;
		return net.at(selected_iface);
	}
}  // namespace Net

namespace Proc {

	vector<proc_info> current_procs;
	std::unordered_map<string, string> uid_user;
	string current_sort;
	string current_filter;
	bool current_rev = false;

	fs::file_time_type passwd_time;

	uint64_t cputimes;
	int collapse = -1, expand = -1;
	uint64_t old_cputimes = 0;
	atomic<int> numpids = 0;
	int filter_found = 0;

	detail_container detailed;

	string get_status(char s) {
		//? IRIX process states from prpsinfo pr_sname / pr_state
		switch (s) {
			case 'R': return "Running";
			case 'S': return "Sleeping";
			case 'D': return "Waiting";
			case 'Z': return "Zombie";
			case 'T': return "Stopped";
			case 'I': return "Idle";
			case 'X': return "SXBRK";
			default:  return "Unknown";
		}
	}

	//* Get detailed info for selected process
	void _collect_details(const size_t pid, vector<proc_info> &procs) {
		if (pid != detailed.last_pid) {
			detailed = {};
			detailed.last_pid = pid;
			detailed.skip_smaps = not Config::getB("proc_info_smaps");
		}

		//? Copy proc_info for process from proc vector
		auto p_info = rng::find(procs, pid, &proc_info::pid);
		if (p_info == procs.end()) return;
		detailed.entry = *p_info;

		//? Update cpu percent deque for process cpu graph
		if (not Config::getB("proc_per_core")) detailed.entry.cpu_p *= Shared::coreCount;
		detailed.cpu_percent.push_back(clamp((long long)round(detailed.entry.cpu_p), 0ll, 100ll));
		while (cmp_greater(detailed.cpu_percent.size(), width)) detailed.cpu_percent.pop_front();

		//? Process runtime : current time - start time
		struct timeval currentTime;
		gettimeofday(&currentTime, nullptr);
		detailed.elapsed = sec_to_dhms(currentTime.tv_sec - detailed.entry.cpu_s);
		if (detailed.elapsed.size() > 8) detailed.elapsed.resize(detailed.elapsed.size() - 3);

		//? Get parent process name
		if (detailed.parent.empty()) {
			auto p_entry = rng::find(procs, detailed.entry.ppid, &proc_info::pid);
			if (p_entry != procs.end()) detailed.parent = p_entry->name;
		}

		//? Expand process status from single char to explanative string
		detailed.status = get_status(detailed.entry.state);

		detailed.mem_bytes.push_back(detailed.entry.mem);
		detailed.memory = floating_humanizer(detailed.entry.mem);

		if (detailed.first_mem == -1 or detailed.first_mem < detailed.mem_bytes.back() / 2 or detailed.first_mem > detailed.mem_bytes.back() * 4) {
			detailed.first_mem = min((uint64_t)detailed.mem_bytes.back() * 2, Mem::get_totalMem());
			redraw = true;
		}

		while (cmp_greater(detailed.mem_bytes.size(), width)) detailed.mem_bytes.pop_front();
	}

	//* Collects and sorts process information from IRIX /proc
	auto collect(bool no_update) -> vector<proc_info> & {
		const auto &sorting = Config::getS("proc_sorting");
		auto reverse = Config::getB("proc_reversed");
		const auto &filter = Config::getS("proc_filter");
		auto per_core = Config::getB("proc_per_core");
		auto tree = Config::getB("proc_tree");
		auto show_detailed = Config::getB("show_detailed");
		const size_t detailed_pid = Config::getI("detailed_pid");
		bool should_filter = current_filter != filter;
		if (should_filter) current_filter = filter;
		bool sorted_change = (sorting != current_sort or reverse != current_rev or should_filter);
		if (sorted_change) {
			current_sort = sorting;
			current_rev = reverse;
		}

		const int cmult = (per_core) ? Shared::coreCount : 1;
		bool got_detailed = false;

		static vector<size_t> found;

		//? Get total CPU time for percentage calculations
		struct sysinfo si;
		if (sysmp(MP_SAGET, MPSA_SINFO, &si, sizeof(si)) != -1) {
			cputimes = si.cpu[CPU_IDLE] + si.cpu[CPU_USER] + si.cpu[CPU_KERNEL] + si.cpu[CPU_WAIT] + si.cpu[CPU_SXBRK];
		}

		//* Use pids from last update if only changing filter, sorting or tree options
		if (no_update and not current_procs.empty()) {
			if (show_detailed and detailed_pid != detailed.last_pid) _collect_details(detailed_pid, current_procs);
		} else {
			//* ---------------------------------------------Collection start----------------------------------------------

			should_filter = true;
			found.clear();
			struct timeval currentTime;
			gettimeofday(&currentTime, nullptr);
			const double timeNow = currentTime.tv_sec + (currentTime.tv_usec / 1'000'000.0);

			//? Read IRIX /proc directory
			//? Each entry is a numeric PID directory
			//? Inside, we can open the file and read prpsinfo_t structure
			DIR *proc_dir = opendir("/proc");
			if (proc_dir == nullptr) {
				Logger::error("Failed to open /proc directory");
				numpids = 0;
				return current_procs;
			}

			struct dirent *entry;
			while ((entry = readdir(proc_dir)) != nullptr) {
				//? Skip non-numeric entries
				if (entry->d_name[0] < '0' || entry->d_name[0] > '9') continue;

				const size_t pid = (size_t)atol(entry->d_name);
				if (pid < 1) continue;

				//? Read prpsinfo_t from /proc/<pid>
				string proc_path = "/proc/" + string(entry->d_name);
				int fd = open(proc_path.c_str(), O_RDONLY);
				if (fd < 0) continue;

				prpsinfo_t psinfo;
				//? On IRIX, ioctl PIOCPSINFO reads process info
				if (ioctl(fd, PIOCPSINFO, &psinfo) < 0) {
					close(fd);
					continue;
				}
				close(fd);

				found.push_back(pid);

				//? Check if pid already exists in current_procs
				bool no_cache = false;
				auto find_old = rng::find(current_procs, pid, &proc_info::pid);
				if (find_old == current_procs.end()) {
					current_procs.push_back({pid});
					find_old = current_procs.end() - 1;
					no_cache = true;
				}

				auto &new_proc = *find_old;

				//? Get program name, command, username, parent pid, nice and status
				if (no_cache) {
					//? Ensure null termination for fixed-size buffers
					char fname_safe[33] = {};
					std::memcpy(fname_safe, psinfo.pr_fname, 32);
					new_proc.name = fname_safe;

					char psargs_safe[81] = {};
					std::memcpy(psargs_safe, psinfo.pr_psargs, 80);
					new_proc.cmd = psargs_safe;

					if (new_proc.cmd.empty()) new_proc.cmd = new_proc.name;
					if (new_proc.cmd.size() > 1000) {
						new_proc.cmd.resize(1000);
						new_proc.cmd.shrink_to_fit();
					}
					new_proc.ppid = psinfo.pr_ppid;
					new_proc.cpu_s = psinfo.pr_start.tv_sec;

					//? Use getpwuid_r for thread safety (getpwuid uses static buffer)
					struct passwd pwd_buf;
					char pw_strbuf[256];
					struct passwd *pwd_result = nullptr;
					if (getpwuid_r(psinfo.pr_uid, &pwd_buf, pw_strbuf, sizeof(pw_strbuf), &pwd_result) == 0 && pwd_result)
						new_proc.user = pwd_result->pw_name;
					else
						new_proc.user = to_string(psinfo.pr_uid);
				}
				new_proc.p_nice = psinfo.pr_nice;
				new_proc.state = psinfo.pr_sname;

				//? CPU time in clock ticks from pr_time (user + system)
				long long cpu_t = (long long)psinfo.pr_time.tv_sec * 1'000'000 + (long long)psinfo.pr_time.tv_nsec / 1000;

				//? Memory: pr_rssize is RSS in pages on IRIX, pr_size is virtual size in pages
				new_proc.mem = (uint64_t)psinfo.pr_rssize * Shared::pageSize;
				new_proc.threads = psinfo.pr_thds > 0 ? psinfo.pr_thds : 1;

				//? Process cpu usage — IRIX pr_cpu is pre-SVR4 CPU usage (0-255 range)
				double pct = (double)((unsigned char)psinfo.pr_cpu) / 255.0 * 100.0;
				new_proc.cpu_p = clamp(pct * cmult, 0.0, 100.0 * Shared::coreCount);

				//? Process cumulative cpu usage since process start
				new_proc.cpu_c = (double)(cpu_t * Shared::clkTck / 1'000'000) / max(1.0, timeNow - (double)new_proc.cpu_s);

				//? Update cached value with latest cpu times
				new_proc.cpu_t = cpu_t;

				if (show_detailed and not got_detailed and new_proc.pid == detailed_pid) {
					got_detailed = true;
				}
			}
			closedir(proc_dir);

			//? Clear dead processes from current_procs
			auto eraser = rng::remove_if(current_procs, [&](const auto &element) { return not v_contains(found, element.pid); });
			current_procs.erase(eraser.begin(), eraser.end());

			//? Update the details info box for process if active
			if (show_detailed and got_detailed) {
				_collect_details(detailed_pid, current_procs);
			} else if (show_detailed and not got_detailed and detailed.status != "Dead") {
				detailed.status = "Dead";
				redraw = true;
			}

			old_cputimes = cputimes;
		}

		//* ---------------------------------------------Collection done-----------------------------------------------

		//* Match filter if defined
		if (should_filter) {
			filter_found = 0;
			for (auto& p : current_procs) {
				if (not tree and not filter.empty()) {
						if (not s_contains_ic(to_string(p.pid), filter)
						and not s_contains_ic(p.name, filter)
						and not s_contains_ic(p.cmd, filter)
						and not s_contains_ic(p.user, filter)) {
							p.filtered = true;
							filter_found++;
							}
						else {
							p.filtered = false;
						}
					}
				else {
					p.filtered = false;
				}
			}
		}

		//* Sort processes
		if (sorted_change or not no_update) {
			proc_sorter(current_procs, sorting, reverse, tree);
		}

		//* Generate tree view if enabled
		if (tree and (not no_update or should_filter or sorted_change)) {
			bool locate_selection = false;
			if (auto find_pid = (collapse != -1 ? collapse : expand); find_pid != -1) {
				auto collapser = rng::find(current_procs, find_pid, &proc_info::pid);
				if (collapser != current_procs.end()) {
					if (collapse == expand) {
						collapser->collapsed = not collapser->collapsed;
					}
					else if (collapse > -1) {
						collapser->collapsed = true;
					}
					else if (expand > -1) {
						collapser->collapsed = false;
					}
					if (Config::ints.at("proc_selected") > 0) locate_selection = true;
				}
				collapse = expand = -1;
			}
			if (should_filter or not filter.empty()) filter_found = 0;

			vector<tree_proc> tree_procs;
			tree_procs.reserve(current_procs.size());

			for (auto& p : current_procs) {
				if (not v_contains(found, p.ppid)) p.ppid = 0;
			}

			//? Stable sort to retain selected sorting among processes with the same parent
			rng::stable_sort(current_procs, rng::less{}, & proc_info::ppid);

			//? Start recursive iteration over processes with the lowest shared parent pids
			if (not current_procs.empty()) {
				for (auto& p : rng::equal_range(current_procs, current_procs.at(0).ppid, rng::less{}, &proc_info::ppid)) {
					_tree_gen(p, current_procs, tree_procs, 0, false, filter, false, no_update, should_filter);
				}
			}

			//? Recursive sort over tree structure to account for collapsed processes in the tree
			int index = 0;
			tree_sort(tree_procs, sorting, reverse, index, current_procs.size());

			//? Add tree begin symbol to first item if childless
			if (not tree_procs.empty() and tree_procs.front().children.empty())
				tree_procs.front().entry.get().prefix.replace(tree_procs.front().entry.get().prefix.size() - 8, 8, " ┌─ ");

			//? Add tree terminator symbol to last item if childless
			if (not tree_procs.empty() and tree_procs.back().children.empty())
				tree_procs.back().entry.get().prefix.replace(tree_procs.back().entry.get().prefix.size() - 8, 8, " └─ ");

			//? Final sort based on tree index
			rng::sort(current_procs, rng::less{}, & proc_info::tree_index);

			//? Move current selection/view to the selected process when collapsing/expanding in the tree
			if (locate_selection) {
				int loc = rng::find(current_procs, Proc::selected_pid, &proc_info::pid)->tree_index;
				if (Config::ints.at("proc_start") >= loc or Config::ints.at("proc_start") <= loc - Proc::select_max)
					Config::ints.at("proc_start") = max(0, loc - 1);
				Config::ints.at("proc_selected") = loc - Config::ints.at("proc_start") + 1;
			}
		}

		numpids = (int)current_procs.size() - filter_found;
		return current_procs;
	}
}  // namespace Proc

namespace Tools {
	double system_uptime() {
		//? IRIX: read boot time from utmpx, compute delta
		struct utmpx *ut;
		time_t boot = 0;
		setutxent();
		while ((ut = getutxent()) != nullptr) {
			if (ut->ut_type == BOOT_TIME) {
				boot = ut->ut_tv.tv_sec;
				break;
			}
		}
		endutxent();

		if (boot > 0) {
			struct timeval currTime;
			gettimeofday(&currTime, nullptr);
			return (double)(currTime.tv_sec - boot);
		}

		//? Fallback: use /proc/0 (sched) start time or just return 0
		return 0.0;
	}
}  // namespace Tools
