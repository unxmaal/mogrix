//! IRIX 6.5 (MIPS N32 ABI) definitions
//!
//! All struct layouts and constants verified empirically on IRIX 6.5.30
//! via struct_sizer.c cross-compiled with irix-cc and run on hardware.
//!
//! N32 ABI: long=32b, ptr=32b, off_t=64b, ino_t=64b, time_t=32b

use crate::prelude::*;

// ===== Primitive type aliases =====
// Only types NOT already in unix/mod.rs

pub type dev_t = u32;
pub type mode_t = u32;
pub type nlink_t = u32;
pub type ino_t = u64;
pub type off_t = i64;
pub type blkcnt_t = i64;
pub type blksize_t = i32;
pub type time_t = i32; // IRIX N32: time_t is int (32-bit)
pub type clock_t = i32;
pub type suseconds_t = i32;
pub type fsblkcnt_t = u64;
pub type fsfilcnt_t = u64;
pub type rlim_t = u64;
pub type sa_family_t = u16;
pub type socklen_t = u32;
pub type pthread_t = u32;
pub type pthread_key_t = i32;
pub type nfds_t = u32;
pub type tcflag_t = u32;
pub type speed_t = u32;
pub type id_t = i32;
pub type key_t = i32;
pub type useconds_t = u32;
pub type clockid_t = c_int;
pub type nl_item = c_int;
pub type regoff_t = isize;
pub type pthread_once_t = c_int;
pub type wchar_t = i32;
pub type major_t = c_uint;
pub type minor_t = c_uint;

// ===== Structures =====
// Only structs NOT already in unix/mod.rs

s! {
    // --- struct stat (152 bytes) ---
    pub struct stat {
        pub st_dev: dev_t,
        __st_pad1: [c_long; 3],
        pub st_ino: ino_t,
        pub st_mode: mode_t,
        pub st_nlink: nlink_t,
        pub st_uid: crate::uid_t,
        pub st_gid: crate::gid_t,
        pub st_rdev: dev_t,
        __st_pad2: [c_long; 3],
        pub st_size: off_t,
        __st_pad3: c_long,
        pub st_atime: time_t,
        pub st_atime_nsec: c_long,
        pub st_mtime: time_t,
        pub st_mtime_nsec: c_long,
        pub st_ctime: time_t,
        pub st_ctime_nsec: c_long,
        pub st_blksize: blksize_t,
        pub st_blocks: blkcnt_t,
        pub st_fstype: [c_char; 16],
        pub st_projid: c_long,
        __st_pad4: [c_long; 7],
    }

    // --- struct dirent ---
    // d_name must be large enough for readdir_r which writes the full
    // filename into a caller-provided buffer. IRIX NAME_MAX = 255.
    pub struct dirent {
        pub d_ino: ino_t,
        pub d_off: off_t,
        pub d_reclen: c_ushort,
        pub d_name: [c_char; 256],
    }

    // --- struct sockaddr (16 bytes) ---
    pub struct sockaddr {
        pub sa_family: sa_family_t,
        pub sa_data: [c_char; 14],
    }

    // --- struct sockaddr_in (16 bytes) ---
    pub struct sockaddr_in {
        pub sin_family: sa_family_t,
        pub sin_port: crate::in_port_t,
        pub sin_addr: crate::in_addr,
        pub sin_zero: [c_char; 8],
    }

    // --- struct sockaddr_in6 (28 bytes without pad, 32 on IRIX) ---
    // No padding field — mio/std zero-fill before use
    pub struct sockaddr_in6 {
        pub sin6_family: sa_family_t,
        pub sin6_port: crate::in_port_t,
        pub sin6_flowinfo: u32,
        pub sin6_addr: crate::in6_addr,
        pub sin6_scope_id: u32,
    }

    // --- struct sockaddr_un (110 bytes) ---
    pub struct sockaddr_un {
        pub sun_family: sa_family_t,
        pub sun_path: [c_char; 108],
    }

    // --- struct sockaddr_storage (128 bytes) ---
    pub struct sockaddr_storage {
        pub ss_family: sa_family_t,
        __ss_pad1: [c_char; 6],
        __ss_align: i64,
        __ss_pad2: [c_char; 112],
    }

    // --- sigset_t (16 bytes) ---
    pub struct sigset_t {
        __sigbits: [u32; 4],
    }

    // --- siginfo_t (128 bytes, opaque on IRIX) ---
    pub struct siginfo_t {
        __data: [c_int; 32],
    }

    // --- struct sigaction (32 bytes) ---
    pub struct sigaction {
        pub sa_flags: c_int,
        pub sa_sigaction: crate::sighandler_t,
        pub sa_mask: sigset_t,
        __sa_resv: [c_int; 2],
    }

    // --- pthread types ---
    pub struct pthread_attr_t {
        __data: [c_long; 5],         // 20 bytes
    }

    pub struct pthread_mutex_t {
        __data: [c_long; 8],         // 32 bytes
    }

    pub struct pthread_mutexattr_t {
        __data: [c_long; 2],         // 8 bytes
    }

    pub struct pthread_cond_t {
        __data: [c_long; 8],         // 32 bytes
    }

    pub struct pthread_condattr_t {
        __data: [c_long; 2],         // 8 bytes
    }

    pub struct pthread_rwlock_t {
        __data: [c_long; 16],        // 64 bytes
    }

    pub struct pthread_rwlockattr_t {
        __data: [c_long; 4],         // 16 bytes
    }

    // --- struct termios (48 bytes) ---
    pub struct termios {
        pub c_iflag: tcflag_t,
        pub c_oflag: tcflag_t,
        pub c_cflag: tcflag_t,
        pub c_lflag: tcflag_t,
        __c_reserved: [c_char; 8],
        pub c_cc: [crate::cc_t; 23],
        __c_pad: c_char,
    }

    // --- struct statvfs (184 bytes) ---
    pub struct statvfs {
        pub f_bsize: c_ulong,
        pub f_frsize: c_ulong,
        pub f_blocks: fsblkcnt_t,
        pub f_bfree: fsblkcnt_t,
        pub f_bavail: fsblkcnt_t,
        pub f_files: fsfilcnt_t,
        pub f_ffree: fsfilcnt_t,
        pub f_favail: fsfilcnt_t,
        pub f_fsid: c_ulong,
        __f_pad1: [c_char; 16],
        pub f_flag: c_ulong,
        pub f_namemax: c_ulong,
        __f_pad2: [c_char; 100],
    }

    // --- struct addrinfo (32 bytes) ---
    pub struct addrinfo {
        pub ai_flags: c_int,
        pub ai_family: c_int,
        pub ai_socktype: c_int,
        pub ai_protocol: c_int,
        pub ai_addrlen: socklen_t,
        pub ai_canonname: *mut c_char,
        pub ai_addr: *mut sockaddr,
        pub ai_next: *mut addrinfo,
    }

    // --- struct tm (36 bytes) ---
    pub struct tm {
        pub tm_sec: c_int,
        pub tm_min: c_int,
        pub tm_hour: c_int,
        pub tm_mday: c_int,
        pub tm_mon: c_int,
        pub tm_year: c_int,
        pub tm_wday: c_int,
        pub tm_yday: c_int,
        pub tm_isdst: c_int,
    }

    // --- struct passwd (36 bytes) ---
    pub struct passwd {
        pub pw_name: *mut c_char,
        pub pw_passwd: *mut c_char,
        pub pw_uid: crate::uid_t,
        pub pw_gid: crate::gid_t,
        pub pw_age: *mut c_char,
        pub pw_comment: *mut c_char,
        pub pw_gecos: *mut c_char,
        pub pw_dir: *mut c_char,
        pub pw_shell: *mut c_char,
    }

    // --- struct in_addr ---
    pub struct in_addr {
        pub s_addr: crate::in_addr_t,
    }

    // --- struct ip_mreq ---
    pub struct ip_mreq {
        pub imr_multiaddr: in_addr,
        pub imr_interface: in_addr,
    }

    // --- struct ip_mreqn ---
    pub struct ip_mreqn {
        pub imr_multiaddr: in_addr,
        pub imr_address: in_addr,
        pub imr_ifindex: c_int,
    }

    // --- struct ip_mreq_source ---
    pub struct ip_mreq_source {
        pub imr_multiaddr: in_addr,
        pub imr_sourceaddr: in_addr,
        pub imr_interface: in_addr,
    }

    // --- struct flock ---
    pub struct flock {
        pub l_type: c_short,
        pub l_whence: c_short,
        pub l_start: off_t,
        pub l_len: off_t,
        pub l_sysid: c_long,
        pub l_pid: crate::pid_t,
        __l_pad: [c_long; 4],
    }

    // --- struct Dl_info ---
    pub struct Dl_info {
        pub dli_fname: *const c_char,
        pub dli_fbase: *mut c_void,
        pub dli_sname: *const c_char,
        pub dli_saddr: *mut c_void,
    }

    // --- struct msghdr ---
    pub struct msghdr {
        pub msg_name: *mut c_void,
        pub msg_namelen: socklen_t,
        pub msg_iov: *mut crate::iovec,
        pub msg_iovlen: c_int,
        pub msg_control: *mut c_void,
        pub msg_controllen: socklen_t,
        pub msg_flags: c_int,
    }

    // --- struct cmsghdr ---
    pub struct cmsghdr {
        pub cmsg_len: socklen_t,
        pub cmsg_level: c_int,
        pub cmsg_type: c_int,
    }

    // --- struct sem_t (64 bytes) ---
    pub struct sem_t {
        __data: [c_long; 16],
    }

    // --- struct utsname ---
    pub struct utsname {
        pub sysname: [c_char; 257],
        pub nodename: [c_char; 257],
        pub release: [c_char; 257],
        pub version: [c_char; 257],
        pub machine: [c_char; 257],
    }

    // --- struct sched_param ---
    pub struct sched_param {
        pub sched_priority: c_int,
    }

    // --- struct fd_set ---
    pub struct fd_set {
        fds_bits: [c_long; 256],
    }

    // --- struct lconv ---
    pub struct lconv {
        pub decimal_point: *mut c_char,
        pub thousands_sep: *mut c_char,
        pub grouping: *mut c_char,
        pub int_curr_symbol: *mut c_char,
        pub currency_symbol: *mut c_char,
        pub mon_decimal_point: *mut c_char,
        pub mon_thousands_sep: *mut c_char,
        pub mon_grouping: *mut c_char,
        pub positive_sign: *mut c_char,
        pub negative_sign: *mut c_char,
        pub int_frac_digits: c_char,
        pub frac_digits: c_char,
        pub p_cs_precedes: c_char,
        pub p_sep_by_space: c_char,
        pub n_cs_precedes: c_char,
        pub n_sep_by_space: c_char,
        pub p_sign_posn: c_char,
        pub n_sign_posn: c_char,
    }

    // --- struct fsid_t ---
    pub struct fsid_t {
        pub val: [c_uint; 2],
    }

    // --- struct statfs (IRIX SVR4-style) ---
    pub struct statfs {
        pub f_type: c_long,
        pub f_bsize: c_long,
        pub f_frsize: c_long,
        pub f_blocks: fsblkcnt_t,
        pub f_bfree: fsblkcnt_t,
        pub f_files: fsblkcnt_t,
        pub f_ffree: fsblkcnt_t,
        pub f_bavail: fsblkcnt_t,
        pub f_fsid: fsid_t,
        pub f_namelen: c_long,
        pub f_flags: c_long,
        f_spare: [c_long; 5],
    }
}

// ===== Constants =====
// Only constants NOT already in unix/mod.rs

// --- Socket address families ---
pub const AF_UNSPEC: c_int = 0;
pub const AF_UNIX: c_int = 1;
pub const AF_LOCAL: c_int = AF_UNIX;
pub const AF_INET: c_int = 2;
pub const AF_INET6: c_int = 24;

// --- Socket types (NOTE: swapped from Linux!) ---
pub const SOCK_DGRAM: c_int = 1;
pub const SOCK_STREAM: c_int = 2;
pub const SOCK_RAW: c_int = 4;
pub const SOCK_SEQPACKET: c_int = 5;
pub const SOCK_RDM: c_int = 6;

// --- Socket options ---
pub const SOL_SOCKET: c_int = 0xFFFF;
pub const SO_REUSEADDR: c_int = 0x4;
pub const SO_KEEPALIVE: c_int = 0x8;
pub const SO_BROADCAST: c_int = 0x20;
pub const SO_SNDBUF: c_int = 0x1001;
pub const SO_RCVBUF: c_int = 0x1002;
pub const SO_ERROR: c_int = 0x1007;
pub const SO_TYPE: c_int = 0x1008;
pub const SO_LINGER: c_int = 0x80;
pub const SO_OOBINLINE: c_int = 0x100;
pub const SO_REUSEPORT: c_int = 0x200;
pub const SO_RCVLOWAT: c_int = 0x1004;
pub const SO_SNDLOWAT: c_int = 0x1003;
pub const SO_RCVTIMEO: c_int = 0x1006;
pub const SO_SNDTIMEO: c_int = 0x1005;
pub const SO_ACCEPTCONN: c_int = 0x2;
pub const SO_DONTROUTE: c_int = 0x10;

// --- TCP options ---
pub const TCP_NODELAY: c_int = 1;

// --- IPv6 options ---
pub const IPV6_V6ONLY: c_int = 27;
pub const IPV6_JOIN_GROUP: c_int = 12;
pub const IPV6_LEAVE_GROUP: c_int = 13;
pub const IPV6_MULTICAST_HOPS: c_int = 10;
pub const IPV6_MULTICAST_IF: c_int = 9;
pub const IPV6_MULTICAST_LOOP: c_int = 11;
pub const IPV6_UNICAST_HOPS: c_int = 4;
pub const IPV6_RECVHOPLIMIT: c_int = 25;
pub const IPV6_RECVTCLASS: c_int = 31;
pub const IPV6_TCLASS: c_int = 30;
pub const IPV6_PKTINFO: c_int = 19;
pub const IPV6_ADD_MEMBERSHIP: c_int = IPV6_JOIN_GROUP;
pub const IPV6_DROP_MEMBERSHIP: c_int = IPV6_LEAVE_GROUP;

// --- IP options ---
pub const IP_TTL: c_int = 4;
pub const IP_MULTICAST_TTL: c_int = 10;
pub const IP_MULTICAST_LOOP: c_int = 11;
pub const IP_ADD_MEMBERSHIP: c_int = 12;
pub const IP_DROP_MEMBERSHIP: c_int = 13;
pub const IP_MULTICAST_IF: c_int = 9;
pub const IP_TOS: c_int = 3;
pub const IP_HDRINCL: c_int = 2;
pub const IP_RECVDSTADDR: c_int = 7;
pub const IP_RECVIF: c_int = 20;
pub const IP_RECVTOS: c_int = 30; // May not exist on IRIX, stub value
pub const IP_ADD_SOURCE_MEMBERSHIP: c_int = 39;
pub const IP_DROP_SOURCE_MEMBERSHIP: c_int = 40;

// --- TCP options ---
pub const TCP_MAXSEG: c_int = 2;
pub const TCP_KEEPIDLE: c_int = 4; // IRIX may not have this, stub value
pub const TCP_KEEPINTVL: c_int = 5;
pub const TCP_KEEPCNT: c_int = 6;

// --- Socket creation flags (IRIX doesn't have these — use fcntl fallback) ---
pub const SOCK_NONBLOCK: c_int = 0; // Stub — not available on IRIX
pub const SOCK_CLOEXEC: c_int = 0;  // Stub — not available on IRIX

// --- Message flags ---
pub const MSG_OOB: c_int = 0x1;
pub const MSG_PEEK: c_int = 0x2;
pub const MSG_DONTROUTE: c_int = 0x4;
pub const MSG_DONTWAIT: c_int = 0x80;
pub const MSG_WAITALL: c_int = 0x40;
pub const MSG_TRUNC: c_int = 0x10;
pub const MSG_CTRUNC: c_int = 0x20;
pub const MSG_EOR: c_int = 0x8;
pub const MSG_NOSIGNAL: c_int = 0; // Not available on IRIX

// --- Shutdown ---
pub const SHUT_RD: c_int = 0;
pub const SHUT_WR: c_int = 1;
pub const SHUT_RDWR: c_int = 2;

// --- File open flags ---
pub const O_RDONLY: c_int = 0x0;
pub const O_WRONLY: c_int = 0x1;
pub const O_RDWR: c_int = 0x2;
pub const O_APPEND: c_int = 0x8;
pub const O_CREAT: c_int = 0x100;
pub const O_EXCL: c_int = 0x400;
pub const O_TRUNC: c_int = 0x200;
pub const O_NONBLOCK: c_int = 0x80;
pub const O_NOCTTY: c_int = 0x800;
pub const O_SYNC: c_int = 0x10;
pub const O_DSYNC: c_int = 0x20;
pub const O_NDELAY: c_int = O_NONBLOCK;
pub const O_ACCMODE: c_int = 0x3;
pub const O_CLOEXEC: c_int = 0x80000;
pub const O_DIRECTORY: c_int = 0x10000;
pub const O_NOFOLLOW: c_int = 0x20000;
pub const O_ASYNC: c_int = 0x40;  // FASYNC on IRIX (SVR4)

// --- fcntl commands ---
pub const F_DUPFD: c_int = 0;
pub const F_GETFD: c_int = 1;
pub const F_SETFD: c_int = 2;
pub const F_GETFL: c_int = 3;
pub const F_SETFL: c_int = 4;
pub const F_GETLK: c_int = 14;
pub const F_SETLK: c_int = 6;
pub const F_SETLKW: c_int = 7;
pub const F_DUPFD_CLOEXEC: c_int = F_DUPFD;
pub const F_RDLCK: c_short = 1;
pub const F_WRLCK: c_short = 2;
pub const F_UNLCK: c_short = 3;

// --- Seek ---
pub const SEEK_SET: c_int = 0;
pub const SEEK_CUR: c_int = 1;
pub const SEEK_END: c_int = 2;

// --- File mode bits (S_ISUID/ISGID/ISVTX already in unix/mod.rs) ---
pub const S_IFMT: mode_t = 0xF000;
pub const S_IFIFO: mode_t = 0x1000;
pub const S_IFCHR: mode_t = 0x2000;
pub const S_IFDIR: mode_t = 0x4000;
pub const S_IFBLK: mode_t = 0x6000;
pub const S_IFREG: mode_t = 0x8000;
pub const S_IFLNK: mode_t = 0xA000;
pub const S_IFSOCK: mode_t = 0xC000;
pub const S_IRWXU: mode_t = 0x1C0;
pub const S_IRUSR: mode_t = 0x100;
pub const S_IWUSR: mode_t = 0x80;
pub const S_IXUSR: mode_t = 0x40;
pub const S_IRWXG: mode_t = 0x38;
pub const S_IRGRP: mode_t = 0x20;
pub const S_IWGRP: mode_t = 0x10;
pub const S_IXGRP: mode_t = 0x8;
pub const S_IRWXO: mode_t = 0x7;
pub const S_IROTH: mode_t = 0x4;
pub const S_IWOTH: mode_t = 0x2;
pub const S_IXOTH: mode_t = 0x1;

// --- Signal numbers (SIGIOT already in unix/mod.rs) ---
pub const SIGHUP: c_int = 1;
pub const SIGINT: c_int = 2;
pub const SIGQUIT: c_int = 3;
pub const SIGILL: c_int = 4;
pub const SIGTRAP: c_int = 5;
pub const SIGABRT: c_int = 6;
pub const SIGEMT: c_int = 7;
pub const SIGFPE: c_int = 8;
pub const SIGKILL: c_int = 9;
pub const SIGBUS: c_int = 10;
pub const SIGSEGV: c_int = 11;
pub const SIGSYS: c_int = 12;
pub const SIGPIPE: c_int = 13;
pub const SIGALRM: c_int = 14;
pub const SIGTERM: c_int = 15;
pub const SIGUSR1: c_int = 16;
pub const SIGUSR2: c_int = 17;
pub const SIGCHLD: c_int = 18;
pub const SIGCLD: c_int = 18;
pub const SIGPWR: c_int = 19;
pub const SIGWINCH: c_int = 20;
pub const SIGURG: c_int = 21;
pub const SIGIO: c_int = 22;
pub const SIGPOLL: c_int = 22;
pub const SIGSTOP: c_int = 23;
pub const SIGTSTP: c_int = 24;
pub const SIGCONT: c_int = 25;
pub const SIGTTIN: c_int = 26;
pub const SIGTTOU: c_int = 27;
pub const SIGVTALRM: c_int = 28;
pub const SIGPROF: c_int = 29;
pub const SIGXCPU: c_int = 30;
pub const SIGXFSZ: c_int = 31;

// --- Signal action flags ---
pub const SA_NOCLDSTOP: c_int = 0x20000;
pub const SA_RESTART: c_int = 0x4;
pub const SA_SIGINFO: c_int = 0x8;
pub const SA_NOCLDWAIT: c_int = 0x10000;
pub const SA_NODEFER: c_int = 0x10;
pub const SA_RESETHAND: c_int = 0x2;
pub const SA_ONSTACK: c_int = 0x1;
pub const SIG_BLOCK: c_int = 1;
pub const SIG_UNBLOCK: c_int = 2;
pub const SIG_SETMASK: c_int = 3;

// --- Errno values ---
pub const EPERM: c_int = 1;
pub const ENOENT: c_int = 2;
pub const ESRCH: c_int = 3;
pub const EINTR: c_int = 4;
pub const EIO: c_int = 5;
pub const ENXIO: c_int = 6;
pub const E2BIG: c_int = 7;
pub const ENOEXEC: c_int = 8;
pub const EBADF: c_int = 9;
pub const ECHILD: c_int = 10;
pub const EAGAIN: c_int = 11;
pub const EWOULDBLOCK: c_int = EAGAIN;
pub const ENOMEM: c_int = 12;
pub const EACCES: c_int = 13;
pub const EFAULT: c_int = 14;
pub const ENOTBLK: c_int = 15;
pub const EBUSY: c_int = 16;
pub const EEXIST: c_int = 17;
pub const EXDEV: c_int = 18;
pub const ENODEV: c_int = 19;
pub const ENOTDIR: c_int = 20;
pub const EISDIR: c_int = 21;
pub const EINVAL: c_int = 22;
pub const ENFILE: c_int = 23;
pub const EMFILE: c_int = 24;
pub const ENOTTY: c_int = 25;
pub const ETXTBSY: c_int = 26;
pub const EFBIG: c_int = 27;
pub const ENOSPC: c_int = 28;
pub const ESPIPE: c_int = 29;
pub const EROFS: c_int = 30;
pub const EMLINK: c_int = 31;
pub const EPIPE: c_int = 32;
pub const EDOM: c_int = 33;
pub const ERANGE: c_int = 34;
pub const ENOMSG: c_int = 35;
pub const EIDRM: c_int = 36;
pub const EDEADLK: c_int = 45;
pub const ENOLCK: c_int = 46;
pub const ENOSYS: c_int = 89;
pub const ENOTEMPTY: c_int = 93;
pub const ELOOP: c_int = 90;
pub const ENAMETOOLONG: c_int = 78;
pub const EOVERFLOW: c_int = 79;
pub const EILSEQ: c_int = 88;
pub const ENOTSOCK: c_int = 95;
pub const EDESTADDRREQ: c_int = 96;
pub const EMSGSIZE: c_int = 97;
pub const EPROTOTYPE: c_int = 98;
pub const ENOPROTOOPT: c_int = 99;
pub const EPROTONOSUPPORT: c_int = 120;
pub const ESOCKTNOSUPPORT: c_int = 121;
pub const EOPNOTSUPP: c_int = 122;
pub const EPFNOSUPPORT: c_int = 123;
pub const EAFNOSUPPORT: c_int = 124;
pub const EADDRINUSE: c_int = 125;
pub const EADDRNOTAVAIL: c_int = 126;
pub const ENETDOWN: c_int = 127;
pub const ENETUNREACH: c_int = 128;
pub const ENETRESET: c_int = 129;
pub const ECONNABORTED: c_int = 130;
pub const ECONNRESET: c_int = 131;
pub const ENOBUFS: c_int = 132;
pub const EISCONN: c_int = 133;
pub const ENOTCONN: c_int = 134;
pub const ESHUTDOWN: c_int = 143;
pub const ETOOMANYREFS: c_int = 144;
pub const ETIMEDOUT: c_int = 145;
pub const ECONNREFUSED: c_int = 146;
pub const EHOSTDOWN: c_int = 147;
pub const EHOSTUNREACH: c_int = 148;
pub const EALREADY: c_int = 149;
pub const EINPROGRESS: c_int = 150;
pub const ESTALE: c_int = 151;
pub const ECANCELED: c_int = 158;
pub const ENOTSUP: c_int = 1008;
pub const EDQUOT: c_int = 49;

// --- SVR4 errno values (shared with Solaris, IRIX is SVR4-derived) ---
pub const ECHRNG: c_int = 37;
pub const EL2NSYNC: c_int = 38;
pub const EL3HLT: c_int = 39;
pub const EL3RST: c_int = 40;
pub const ELNRNG: c_int = 41;
pub const EUNATCH: c_int = 42;
pub const ENOCSI: c_int = 43;
pub const EL2HLT: c_int = 44;
pub const EBADR: c_int = 51;
pub const ENOANO: c_int = 53;
pub const EBADRQC: c_int = 54;
pub const EBADSLT: c_int = 55;
pub const EDEADLOCK: c_int = 56;
pub const EBFONT: c_int = 57;
pub const EOWNERDEAD: c_int = 58;
pub const ENOTRECOVERABLE: c_int = 59;
pub const ENOSTR: c_int = 60;
pub const ENODATA: c_int = 61;
pub const ETIME: c_int = 62;
pub const ENOSR: c_int = 63;
pub const ENONET: c_int = 64;
pub const ENOPKG: c_int = 65;
pub const EREMOTE: c_int = 66;
pub const ENOLINK: c_int = 67;
pub const EADV: c_int = 68;
pub const ESRMNT: c_int = 69;
pub const ECOMM: c_int = 70;
pub const EPROTO: c_int = 71;
pub const EMULTIHOP: c_int = 74;
pub const EBADMSG: c_int = 77;
pub const ENOTUNIQ: c_int = 80;
pub const EBADFD: c_int = 81;
pub const EREMCHG: c_int = 82;
pub const ELIBACC: c_int = 83;
pub const ELIBBAD: c_int = 84;
pub const ELIBSCN: c_int = 85;
pub const ELIBMAX: c_int = 86;
pub const ELIBEXEC: c_int = 87;
pub const ERESTART: c_int = 91;
pub const ESTRPIPE: c_int = 92;
pub const EUSERS: c_int = 94;

// --- Errno values not in IRIX/SVR4 (stub values for Linux-origin crates) ---
// These will never actually be returned by IRIX syscalls but are needed
// for crate compilation. Use high values to avoid conflicts.
pub const EDOTDOT: c_int = 1100;
pub const EISNAM: c_int = 1101;
pub const ENAVAIL: c_int = 1102;
pub const ENOTNAM: c_int = 1103;
pub const EREMOTEIO: c_int = 1104;
pub const ENOMEDIUM: c_int = 1105;
pub const EMEDIUMTYPE: c_int = 1106;
pub const EKEYEXPIRED: c_int = 1107;
pub const EKEYREJECTED: c_int = 1108;
pub const EKEYREVOKED: c_int = 1109;
pub const ENOKEY: c_int = 1110;
pub const ERFKILL: c_int = 1111;
pub const EHWPOISON: c_int = 1112;
pub const EUCLEAN: c_int = 1113;
pub const EXFULL: c_int = 1114;

// --- mmap constants ---
pub const PROT_NONE: c_int = 0x0;
pub const PROT_READ: c_int = 0x1;
pub const PROT_WRITE: c_int = 0x2;
pub const PROT_EXEC: c_int = 0x4;
pub const MAP_SHARED: c_int = 0x1;
pub const MAP_PRIVATE: c_int = 0x2;
pub const MAP_FIXED: c_int = 0x10;
pub const MAP_ANON: c_int = 0x100;
pub const MAP_ANONYMOUS: c_int = MAP_ANON;
pub const MAP_FAILED: *mut c_void = !0 as *mut c_void;
pub const MS_ASYNC: c_int = 0x1;
pub const MS_INVALIDATE: c_int = 0x2;
pub const MS_SYNC: c_int = 0x4;
pub const MADV_NORMAL: c_int = 0;
pub const MADV_RANDOM: c_int = 1;
pub const MADV_SEQUENTIAL: c_int = 2;
pub const MADV_WILLNEED: c_int = 3;
pub const MADV_DONTNEED: c_int = 4;

// --- poll constants ---
pub const POLLIN: c_short = 0x1;
pub const POLLPRI: c_short = 0x2;
pub const POLLOUT: c_short = 0x4;
pub const POLLERR: c_short = 0x8;
pub const POLLHUP: c_short = 0x10;
pub const POLLNVAL: c_short = 0x20;
pub const POLLRDNORM: c_short = 0x40;
pub const POLLRDBAND: c_short = 0x80;
pub const POLLWRNORM: c_short = 0x4;
pub const POLLWRBAND: c_short = 0x100;

// --- Wait constants ---
pub const WNOHANG: c_int = 0x40;
pub const WUNTRACED: c_int = 0x4;
pub const WCONTINUED: c_int = 0x8;

// --- Resource limit constants ---
pub const RLIMIT_CPU: c_int = 0;
pub const RLIMIT_FSIZE: c_int = 1;
pub const RLIMIT_DATA: c_int = 2;
pub const RLIMIT_STACK: c_int = 3;
pub const RLIMIT_CORE: c_int = 4;
pub const RLIMIT_NOFILE: c_int = 5;
pub const RLIMIT_AS: c_int = 6;
pub const RLIMIT_RSS: c_int = 7;
pub const RLIM_INFINITY: rlim_t = 0x7FFFFFFFFFFFFFFF;

// --- Limit constants ---
pub const PATH_MAX: c_int = 1024;
pub const PIPE_BUF: usize = 10240;
pub const NAME_MAX: c_int = 255;
pub const IOV_MAX: c_int = 1024;

// --- Termios constants ---
pub const NCCS: usize = 23;
pub const VEOF: usize = 4;
pub const VEOL: usize = 5;
pub const VERASE: usize = 2;
pub const VKILL: usize = 3;
pub const VINTR: usize = 0;
pub const VQUIT: usize = 1;
pub const VSTART: usize = 8;
pub const VSTOP: usize = 9;
pub const VSUSP: usize = 10;
pub const VMIN: usize = 4;
pub const VTIME: usize = 5;
pub const BRKINT: tcflag_t = 0x2;
pub const ICRNL: tcflag_t = 0x100;
pub const INPCK: tcflag_t = 0x10;
pub const ISTRIP: tcflag_t = 0x20;
pub const IXON: tcflag_t = 0x400;
pub const IXOFF: tcflag_t = 0x1000;
pub const IXANY: tcflag_t = 0x800;
pub const IGNBRK: tcflag_t = 0x1;
pub const IGNPAR: tcflag_t = 0x4;
pub const PARMRK: tcflag_t = 0x8;
pub const INLCR: tcflag_t = 0x40;
pub const IGNCR: tcflag_t = 0x80;
pub const OPOST: tcflag_t = 0x1;
pub const CS5: tcflag_t = 0x0;
pub const CS6: tcflag_t = 0x10;
pub const CS7: tcflag_t = 0x20;
pub const CS8: tcflag_t = 0x30;
pub const CSIZE: tcflag_t = 0x30;
pub const CSTOPB: tcflag_t = 0x40;
pub const CREAD: tcflag_t = 0x80;
pub const PARENB: tcflag_t = 0x100;
pub const PARODD: tcflag_t = 0x200;
pub const HUPCL: tcflag_t = 0x400;
pub const CLOCAL: tcflag_t = 0x800;
pub const ECHO: tcflag_t = 0x8;
pub const ECHOE: tcflag_t = 0x10;
pub const ECHOK: tcflag_t = 0x20;
pub const ECHONL: tcflag_t = 0x40;
pub const ICANON: tcflag_t = 0x2;
pub const IEXTEN: tcflag_t = 0x100;
pub const ISIG: tcflag_t = 0x1;
pub const NOFLSH: tcflag_t = 0x80;
pub const TOSTOP: tcflag_t = 0x100;
pub const TCSANOW: c_int = 21518;
pub const TCSADRAIN: c_int = 21519;
pub const TCSAFLUSH: c_int = 21520;
pub const B0: speed_t = 0;
pub const B50: speed_t = 1;
pub const B75: speed_t = 2;
pub const B110: speed_t = 3;
pub const B134: speed_t = 4;
pub const B150: speed_t = 5;
pub const B200: speed_t = 6;
pub const B300: speed_t = 7;
pub const B600: speed_t = 8;
pub const B1200: speed_t = 9;
pub const B1800: speed_t = 10;
pub const B2400: speed_t = 11;
pub const B4800: speed_t = 12;
pub const B9600: speed_t = 13;
pub const B19200: speed_t = 14;
pub const B38400: speed_t = 15;
// Extended baud rates (SVR4/Solaris values where available, stubs for Linux-only speeds)
pub const B57600: speed_t = 16;
pub const B115200: speed_t = 18;
pub const B230400: speed_t = 20;
pub const B460800: speed_t = 22;
pub const B921600: speed_t = 23;
// These speeds don't exist on IRIX hardware but are needed for crate compilation
pub const B500000: speed_t = 24;
pub const B576000: speed_t = 25;
pub const B1000000: speed_t = 26;
pub const B1152000: speed_t = 27;
pub const B1500000: speed_t = 28;
pub const B2000000: speed_t = 29;
pub const B2500000: speed_t = 30;
pub const B3000000: speed_t = 31;
pub const B3500000: speed_t = 32;
pub const B4000000: speed_t = 33;

// --- Additional termios control characters ---
pub const VEOL2: usize = 6;
pub const VSWTC: usize = 7;     // VSWTCH on SVR4
pub const VSWTCH: usize = VSWTC;
pub const VREPRINT: usize = 12;
pub const VDISCARD: usize = 13;
pub const VWERASE: usize = 14;
pub const VLNEXT: usize = 15;

// --- Additional termios input flags ---
pub const IMAXBEL: tcflag_t = 0o020000;  // Same as Solaris
pub const IUTF8: tcflag_t = 0o040000;    // Stub for IRIX (no UTF-8 support)

// --- Additional termios output flags ---
pub const OLCUC: tcflag_t = 0o000002;
pub const ONLCR: tcflag_t = 0o000004;
pub const OCRNL: tcflag_t = 0o000010;
pub const ONOCR: tcflag_t = 0o000020;
pub const ONLRET: tcflag_t = 0o000040;
pub const OFILL: tcflag_t = 0o000100;
pub const OFDEL: tcflag_t = 0o000200;
pub const NLDLY: tcflag_t = 0o000400;
pub const NL0: tcflag_t = 0o000000;
pub const NL1: tcflag_t = 0o000400;
pub const CRDLY: tcflag_t = 0o003000;
pub const CR0: tcflag_t = 0o000000;
pub const CR1: tcflag_t = 0o001000;
pub const CR2: tcflag_t = 0o002000;
pub const CR3: tcflag_t = 0o003000;
pub const TABDLY: tcflag_t = 0o014000;
pub const TAB0: tcflag_t = 0o000000;
pub const TAB1: tcflag_t = 0o004000;
pub const TAB2: tcflag_t = 0o010000;
pub const TAB3: tcflag_t = 0o014000;
pub const XTABS: tcflag_t = 0o014000;
pub const BSDLY: tcflag_t = 0o020000;
pub const BS0: tcflag_t = 0o000000;
pub const BS1: tcflag_t = 0o020000;
pub const VTDLY: tcflag_t = 0o040000;
pub const VT0: tcflag_t = 0o000000;
pub const VT1: tcflag_t = 0o040000;
pub const FFDLY: tcflag_t = 0o100000;
pub const FF0: tcflag_t = 0o000000;
pub const FF1: tcflag_t = 0o100000;

// --- Additional termios control flags ---
pub const CRTSCTS: tcflag_t = 0x80000000;  // Hardware flow control
pub const CMSPAR: tcflag_t = 0x40000000;   // Stub — not available on IRIX

// --- Additional termios local flags ---
pub const ECHOCTL: tcflag_t = 0o001000;
pub const ECHOPRT: tcflag_t = 0o002000;
pub const ECHOKE: tcflag_t = 0o004000;
pub const FLUSHO: tcflag_t = 0o020000;
pub const PENDIN: tcflag_t = 0o040000;
pub const EXTPROC: tcflag_t = 0o200000;

// --- tcflow/tcflush constants ---
pub const TCIFLUSH: c_int = 0;
pub const TCOFLUSH: c_int = 1;
pub const TCIOFLUSH: c_int = 2;
pub const TCOOFF: c_int = 0;
pub const TCOON: c_int = 1;
pub const TCIOFF: c_int = 2;
pub const TCION: c_int = 3;

// --- Clock constants ---
pub const CLOCK_REALTIME: clockid_t = 1;
pub const CLOCK_SGI_CYCLE: clockid_t = 2;
// IRIX has no true monotonic clock; fall back to CLOCK_REALTIME
pub const CLOCK_MONOTONIC: clockid_t = CLOCK_REALTIME;

// --- sysconf constants ---
pub const _SC_CLK_TCK: c_int = 3;
pub const _SC_OPEN_MAX: c_int = 5;
pub const _SC_PAGESIZE: c_int = 11;
pub const _SC_PAGE_SIZE: c_int = _SC_PAGESIZE;
pub const _SC_NPROCESSORS_ONLN: c_int = 15;
pub const _SC_GETPW_R_SIZE_MAX: c_int = 71;
pub const _SC_GETGR_R_SIZE_MAX: c_int = 72;
pub const _SC_HOST_NAME_MAX: c_int = 72; // IRIX sysconf value

// --- IPC constants ---
pub const IPC_CREAT: c_int = 0x200;
pub const IPC_EXCL: c_int = 0x400;
pub const IPC_NOWAIT: c_int = 0x800;
pub const IPC_RMID: c_int = 10;
pub const IPC_SET: c_int = 11;
pub const IPC_STAT: c_int = 12;
pub const IPC_PRIVATE: key_t = 0;

// --- AT_* constants ---
pub const AT_FDCWD: c_int = -2;
pub const AT_REMOVEDIR: c_int = 0x1;
pub const AT_SYMLINK_NOFOLLOW: c_int = 0x2;
pub const AT_EACCESS: c_int = 0x4;            // Same as Solaris
pub const AT_SYMLINK_FOLLOW: c_int = 0x2000;  // Same as Solaris

// --- ioctl constants ---
// Rust libc uses c_ulong for ioctl request parameter on all Unix.
// IRIX native ioctl(2) takes int, but the Rust binding uses c_ulong.
// Note: IRIX doesn't support FIOCLEX on sockets — use fcntl(F_SETFD) instead.
pub const FIOCLEX: c_ulong = 0x6601;
pub const FIONBIO: c_ulong = 0x667e;
// IRIX ioctl encoding: direction | (size << 16) | (group << 8) | num
// struct winsize is 8 bytes, group 't' = 0x74
pub const TIOCGWINSZ: c_ulong = 0x40087468;  // _IOR('t', 104, struct winsize)
pub const TIOCSWINSZ: c_ulong = 0x80087467;  // _IOW('t', 103, struct winsize)
pub const FIONREAD: c_ulong = 0x467f;     // IRIX FIONREAD
pub const TIOCEXCL: c_ulong = 0x740d;     // tIOC | 13
pub const TIOCNXCL: c_ulong = 0x740e;     // tIOC | 14

// --- Socket listen backlog ---
pub const SOMAXCONN: c_int = 5;

// --- utimensat constants ---
pub const UTIME_OMIT: c_long = -1;
pub const UTIME_NOW: c_long = -2;

// --- SCM constants ---
pub const SCM_RIGHTS: c_int = 0x01;

// --- Misc ---
pub const FD_SETSIZE: usize = 8192;
pub const NFDBITS: usize = 32;

// --- addrinfo flags ---
pub const AI_PASSIVE: c_int = 0x1;
pub const AI_CANONNAME: c_int = 0x2;
pub const AI_NUMERICHOST: c_int = 0x4;
pub const AI_NUMERICSERV: c_int = 0x8;

// --- NI flags ---
pub const NI_MAXHOST: socklen_t = 1025;
pub const NI_MAXSERV: socklen_t = 32;
pub const NI_NUMERICHOST: c_int = 0x1;
pub const NI_NUMERICSERV: c_int = 0x2;
pub const NI_NOFQDN: c_int = 0x4;
pub const NI_NAMEREQD: c_int = 0x8;
pub const NI_DGRAM: c_int = 0x10;

// --- EAI error codes ---
pub const EAI_AGAIN: c_int = 2;
pub const EAI_BADFLAGS: c_int = 3;
pub const EAI_FAIL: c_int = 4;
pub const EAI_FAMILY: c_int = 5;
pub const EAI_MEMORY: c_int = 6;
pub const EAI_NONAME: c_int = 8;
pub const EAI_SERVICE: c_int = 9;
pub const EAI_SOCKTYPE: c_int = 10;
pub const EAI_SYSTEM: c_int = 11;
pub const EAI_OVERFLOW: c_int = 12;

// --- Process priority ---
pub const PRIO_PROCESS: c_int = 0;
pub const PRIO_PGRP: c_int = 1;
pub const PRIO_USER: c_int = 2;

// --- Rusage who ---
pub const RUSAGE_SELF: c_int = 0;
pub const RUSAGE_CHILDREN: c_int = -1;

// --- pthread constants ---
pub const PTHREAD_MUTEX_INITIALIZER: pthread_mutex_t = pthread_mutex_t { __data: [0; 8] };
pub const PTHREAD_COND_INITIALIZER: pthread_cond_t = pthread_cond_t { __data: [0; 8] };
pub const PTHREAD_RWLOCK_INITIALIZER: pthread_rwlock_t = pthread_rwlock_t { __data: [0; 16] };
pub const PTHREAD_MUTEX_NORMAL: c_int = 0;
pub const PTHREAD_MUTEX_RECURSIVE: c_int = 1;
pub const PTHREAD_MUTEX_ERRORCHECK: c_int = 2;
pub const PTHREAD_MUTEX_DEFAULT: c_int = PTHREAD_MUTEX_NORMAL;
pub const PTHREAD_CREATE_JOINABLE: c_int = 0;
pub const PTHREAD_CREATE_DETACHED: c_int = 1;
pub const PTHREAD_STACK_MIN: usize = 16384;
pub const PTHREAD_ONCE_INIT: pthread_once_t = 0;

// --- Standard file descriptors ---
pub const STDIN_FILENO: c_int = 0;
pub const STDOUT_FILENO: c_int = 1;
pub const STDERR_FILENO: c_int = 2;

// --- Access mode ---
pub const R_OK: c_int = 4;
pub const W_OK: c_int = 2;
pub const X_OK: c_int = 1;
pub const F_OK: c_int = 0;

// --- Exit status ---
pub const EXIT_SUCCESS: c_int = 0;
pub const EXIT_FAILURE: c_int = 1;

// --- dlopen constants ---
pub const RTLD_LAZY: c_int = 0x1;
pub const RTLD_NOW: c_int = 0x2;
pub const RTLD_GLOBAL: c_int = 0x4;
pub const RTLD_LOCAL: c_int = 0x0;
pub const RTLD_DEFAULT: *mut c_void = 0 as *mut c_void;

// --- Regex constants ---
pub const REG_EXTENDED: c_int = 1;
pub const REG_ICASE: c_int = 2;
pub const REG_NEWLINE: c_int = 4;
pub const REG_NOSUB: c_int = 8;
pub const REG_NOTBOL: c_int = 1;
pub const REG_NOTEOL: c_int = 2;
pub const REG_NOMATCH: c_int = 1;

// --- File locking (flock) ---
pub const LOCK_SH: c_int = 1;
pub const LOCK_EX: c_int = 2;
pub const LOCK_NB: c_int = 4;
pub const LOCK_UN: c_int = 8;

// --- statfs flags ---
pub const ST_RDONLY: c_ulong = 1;
pub const ST_NOSUID: c_ulong = 2;

// --- posix_fadvise constants ---
pub const POSIX_FADV_NORMAL: c_int = 0;
pub const POSIX_FADV_RANDOM: c_int = 1;
pub const POSIX_FADV_SEQUENTIAL: c_int = 2;
pub const POSIX_FADV_WILLNEED: c_int = 3;
pub const POSIX_FADV_DONTNEED: c_int = 4;
pub const POSIX_FADV_NOREUSE: c_int = 5;

// --- fallocate flags (Linux-origin, stubs for crate compilation) ---
pub const FALLOC_FL_KEEP_SIZE: c_int = 0x01;
pub const FALLOC_FL_PUNCH_HOLE: c_int = 0x02;
pub const FALLOC_FL_NO_HIDE_STALE: c_int = 0x04;
pub const FALLOC_FL_COLLAPSE_RANGE: c_int = 0x08;
pub const FALLOC_FL_ZERO_RANGE: c_int = 0x10;
pub const FALLOC_FL_INSERT_RANGE: c_int = 0x20;
pub const FALLOC_FL_UNSHARE_RANGE: c_int = 0x40;

// ===== Wait status macros =====

pub fn WIFEXITED(status: c_int) -> bool {
    (status & 0xff) == 0
}
pub fn WEXITSTATUS(status: c_int) -> c_int {
    (status >> 8) & 0xff
}
pub fn WIFSIGNALED(status: c_int) -> bool {
    (status & 0xff) != 0 && (status & 0xff) != 0x7f
}
pub fn WTERMSIG(status: c_int) -> c_int {
    status & 0x7f
}
pub fn WIFSTOPPED(status: c_int) -> bool {
    (status & 0xff) == 0x7f
}
pub fn WSTOPSIG(status: c_int) -> c_int {
    (status >> 8) & 0xff
}
pub fn WCOREDUMP(status: c_int) -> bool {
    (status & 0x80) != 0
}
pub fn WIFCONTINUED(status: c_int) -> bool {
    (status & 0xffff) == 0xffff
}

// ===== FD_SET macros =====

pub fn FD_CLR(fd: c_int, set: *mut fd_set) {
    let fd = fd as usize;
    unsafe { (*set).fds_bits[fd / NFDBITS] &= !(1 << (fd % NFDBITS)); }
}
pub fn FD_ISSET(fd: c_int, set: *const fd_set) -> bool {
    let fd = fd as usize;
    unsafe { ((*set).fds_bits[fd / NFDBITS] & (1 << (fd % NFDBITS))) != 0 }
}
pub fn FD_SET(fd: c_int, set: *mut fd_set) {
    let fd = fd as usize;
    unsafe { (*set).fds_bits[fd / NFDBITS] |= 1 << (fd % NFDBITS); }
}
pub fn FD_ZERO(set: *mut fd_set) {
    unsafe {
        let s = &mut (*set).fds_bits;
        for i in 0..s.len() { s[i] = 0; }
    }
}

// ===== IRIX-specific extern functions =====
// (Common POSIX functions are declared in unix/mod.rs)

extern "C" {
    // IRIX errno access
    pub fn __oserror() -> *mut c_int;
    // Alias for AIX compatibility (getrandom uses _Errno)
    #[link_name = "__oserror"]
    pub fn _Errno() -> *mut c_int;

    // IRIX-specific function declarations
    pub fn stat(path: *const c_char, buf: *mut stat) -> c_int;
    pub fn lstat(path: *const c_char, buf: *mut stat) -> c_int;
    pub fn fstat(fd: c_int, buf: *mut stat) -> c_int;
    pub fn poll(fds: *mut crate::pollfd, nfds: nfds_t, timeout: c_int) -> c_int;
    pub fn statvfs(path: *const c_char, buf: *mut statvfs) -> c_int;
    pub fn fstatvfs(fd: c_int, buf: *mut statvfs) -> c_int;
    pub fn sendmsg(fd: c_int, msg: *const msghdr, flags: c_int) -> isize;
    pub fn recvmsg(fd: c_int, msg: *mut msghdr, flags: c_int) -> isize;
    pub fn getaddrinfo(
        node: *const c_char, service: *const c_char,
        hints: *const addrinfo, res: *mut *mut addrinfo,
    ) -> c_int;
    pub fn freeaddrinfo(res: *mut addrinfo);
    pub fn gai_strerror(errcode: c_int) -> *const c_char;
    pub fn getnameinfo(
        sa: *const sockaddr, salen: socklen_t,
        host: *mut c_char, hostlen: socklen_t,
        serv: *mut c_char, servlen: socklen_t,
        flags: c_int,
    ) -> c_int;
    pub fn uname(buf: *mut utsname) -> c_int;
    pub fn sem_init(sem: *mut sem_t, pshared: c_int, value: c_uint) -> c_int;
    pub fn sem_destroy(sem: *mut sem_t) -> c_int;
    pub fn sem_wait(sem: *mut sem_t) -> c_int;
    pub fn sem_trywait(sem: *mut sem_t) -> c_int;
    pub fn sem_post(sem: *mut sem_t) -> c_int;

    // Common POSIX functions not in unix/mod.rs
    pub fn gettimeofday(tp: *mut crate::timeval, tzp: *mut c_void) -> c_int;
    pub fn settimeofday(tp: *const crate::timeval, tzp: *const c_void) -> c_int;
    pub fn getrusage(who: c_int, usage: *mut crate::rusage) -> c_int;
    pub fn getrlimit(resource: c_int, rlim: *mut crate::rlimit) -> c_int;
    pub fn setrlimit(resource: c_int, rlim: *const crate::rlimit) -> c_int;
    pub fn tcgetattr(fd: c_int, termios: *mut termios) -> c_int;
    pub fn tcsetattr(fd: c_int, actions: c_int, termios: *const termios) -> c_int;

    // pthread condattr clock selection
    pub fn pthread_condattr_setclock(
        attr: *mut pthread_condattr_t,
        clock_id: clockid_t,
    ) -> c_int;

    // Group management
    pub fn setgroups(ngroups: c_int, grouplist: *const crate::gid_t) -> c_int;

    // File timestamp functions
    pub fn futimens(fd: c_int, times: *const crate::timespec) -> c_int;
    pub fn utimensat(
        dirfd: c_int,
        path: *const c_char,
        times: *const crate::timespec,
        flag: c_int,
    ) -> c_int;

    // === Functions that are in OS-specific libc modules, not unix/mod.rs ===

    // Network
    pub fn bind(socket: c_int, address: *const sockaddr, address_len: socklen_t) -> c_int;
    pub fn connect(socket: c_int, address: *const sockaddr, len: socklen_t) -> c_int;
    pub fn listen(socket: c_int, backlog: c_int) -> c_int;
    pub fn accept(
        socket: c_int, address: *mut sockaddr, address_len: *mut socklen_t,
    ) -> c_int;
    pub fn socket(domain: c_int, ty: c_int, protocol: c_int) -> c_int;
    pub fn send(socket: c_int, buf: *const c_void, len: usize, flags: c_int) -> isize;
    pub fn recv(socket: c_int, buf: *mut c_void, len: usize, flags: c_int) -> isize;
    pub fn sendto(
        socket: c_int, buf: *const c_void, len: usize, flags: c_int,
        addr: *const sockaddr, addrlen: socklen_t,
    ) -> isize;
    pub fn recvfrom(
        socket: c_int, buf: *mut c_void, len: usize, flags: c_int,
        addr: *mut sockaddr, addrlen: *mut socklen_t,
    ) -> isize;
    pub fn setsockopt(
        socket: c_int, level: c_int, name: c_int,
        value: *const c_void, option_len: socklen_t,
    ) -> c_int;
    pub fn getsockopt(
        socket: c_int, level: c_int, name: c_int,
        value: *mut c_void, option_len: *mut socklen_t,
    ) -> c_int;
    pub fn getsockname(
        socket: c_int, address: *mut sockaddr, address_len: *mut socklen_t,
    ) -> c_int;
    pub fn getpeername(
        socket: c_int, address: *mut sockaddr, address_len: *mut socklen_t,
    ) -> c_int;
    pub fn shutdown(socket: c_int, how: c_int) -> c_int;
    pub fn socketpair(domain: c_int, ty: c_int, protocol: c_int, sv: *mut c_int) -> c_int;

    // I/O
    pub fn readv(fd: c_int, iov: *const crate::iovec, iovcnt: c_int) -> isize;
    pub fn writev(fd: c_int, iov: *const crate::iovec, iovcnt: c_int) -> isize;
    pub fn ioctl(fd: c_int, request: c_ulong, ...) -> c_int;

    // Process
    pub fn fork() -> crate::pid_t;
    pub fn execve(
        path: *const c_char, argv: *const *const c_char, envp: *const *const c_char,
    ) -> c_int;
    pub fn execvp(file: *const c_char, argv: *const *const c_char) -> c_int;
    pub fn _exit(status: c_int) -> !;
    pub fn waitpid(pid: crate::pid_t, status: *mut c_int, options: c_int) -> crate::pid_t;
    pub fn kill(pid: crate::pid_t, sig: c_int) -> c_int;
    pub fn getpid() -> crate::pid_t;
    pub fn getppid() -> crate::pid_t;
    pub fn getuid() -> crate::uid_t;
    pub fn geteuid() -> crate::uid_t;
    pub fn getgid() -> crate::gid_t;
    pub fn getegid() -> crate::gid_t;
    pub fn setuid(uid: crate::uid_t) -> c_int;
    pub fn setgid(gid: crate::gid_t) -> c_int;
    pub fn seteuid(uid: crate::uid_t) -> c_int;
    pub fn setegid(gid: crate::gid_t) -> c_int;
    pub fn setsid() -> crate::pid_t;

    // File I/O
    pub fn open(path: *const c_char, oflag: c_int, ...) -> c_int;
    pub fn close(fd: c_int) -> c_int;
    pub fn read(fd: c_int, buf: *mut c_void, count: usize) -> isize;
    pub fn write(fd: c_int, buf: *const c_void, count: usize) -> isize;
    pub fn lseek(fd: c_int, offset: off_t, whence: c_int) -> off_t;
    pub fn pread(fd: c_int, buf: *mut c_void, count: usize, offset: off_t) -> isize;
    pub fn pwrite(fd: c_int, buf: *const c_void, count: usize, offset: off_t) -> isize;
    pub fn dup(fd: c_int) -> c_int;
    pub fn dup2(src: c_int, dst: c_int) -> c_int;
    pub fn pipe(fds: *mut c_int) -> c_int;
    pub fn fcntl(fd: c_int, cmd: c_int, ...) -> c_int;
    pub fn ftruncate(fd: c_int, length: off_t) -> c_int;
    pub fn truncate(path: *const c_char, length: off_t) -> c_int;
    pub fn fsync(fd: c_int) -> c_int;
    pub fn fdatasync(fd: c_int) -> c_int;
    pub fn fchmod(fd: c_int, mode: mode_t) -> c_int;
    pub fn fchown(fd: c_int, owner: crate::uid_t, group: crate::gid_t) -> c_int;
    pub fn isatty(fd: c_int) -> c_int;

    // Filesystem
    pub fn mkdir(path: *const c_char, mode: mode_t) -> c_int;
    pub fn rmdir(path: *const c_char) -> c_int;
    pub fn rename(oldname: *const c_char, newname: *const c_char) -> c_int;
    pub fn unlink(path: *const c_char) -> c_int;
    pub fn link(src: *const c_char, dst: *const c_char) -> c_int;
    pub fn symlink(src: *const c_char, dst: *const c_char) -> c_int;
    pub fn readlink(path: *const c_char, buf: *mut c_char, bufsiz: usize) -> isize;
    pub fn realpath(path: *const c_char, resolved: *mut c_char) -> *mut c_char;
    pub fn chmod(path: *const c_char, mode: mode_t) -> c_int;
    pub fn chown(path: *const c_char, owner: crate::uid_t, group: crate::gid_t) -> c_int;
    pub fn lchown(path: *const c_char, owner: crate::uid_t, group: crate::gid_t) -> c_int;
    pub fn chdir(dir: *const c_char) -> c_int;
    pub fn fchdir(fd: c_int) -> c_int;
    pub fn getcwd(buf: *mut c_char, size: usize) -> *mut c_char;
    pub fn access(path: *const c_char, amode: c_int) -> c_int;
    pub fn umask(mask: mode_t) -> mode_t;

    // Directory
    pub fn opendir(name: *const c_char) -> *mut crate::DIR;
    pub fn readdir(dirp: *mut crate::DIR) -> *mut dirent;
    pub fn closedir(dirp: *mut crate::DIR) -> c_int;
    pub fn rewinddir(dirp: *mut crate::DIR);
    pub fn dirfd(dirp: *mut crate::DIR) -> c_int;

    // Memory
    pub fn mmap(
        addr: *mut c_void, len: usize, prot: c_int,
        flags: c_int, fd: c_int, offset: off_t,
    ) -> *mut c_void;
    pub fn munmap(addr: *mut c_void, len: usize) -> c_int;
    pub fn mprotect(addr: *mut c_void, len: usize, prot: c_int) -> c_int;
    pub fn msync(addr: *mut c_void, len: usize, flags: c_int) -> c_int;
    pub fn madvise(addr: *mut c_void, len: usize, advice: c_int) -> c_int;
    pub fn mlock(addr: *const c_void, len: usize) -> c_int;
    pub fn munlock(addr: *const c_void, len: usize) -> c_int;

    // Signal
    pub fn sigaction(signum: c_int, act: *const sigaction, oldact: *mut sigaction) -> c_int;
    pub fn sigemptyset(set: *mut sigset_t) -> c_int;
    pub fn sigfillset(set: *mut sigset_t) -> c_int;
    pub fn sigaddset(set: *mut sigset_t, signum: c_int) -> c_int;
    pub fn sigdelset(set: *mut sigset_t, signum: c_int) -> c_int;
    pub fn sigismember(set: *const sigset_t, signum: c_int) -> c_int;
    pub fn sigprocmask(how: c_int, set: *const sigset_t, oldset: *mut sigset_t) -> c_int;
    pub fn raise(sig: c_int) -> c_int;

    // Threads
    pub fn pthread_create(
        thread: *mut pthread_t, attr: *const pthread_attr_t,
        f: extern "C" fn(*mut c_void) -> *mut c_void, arg: *mut c_void,
    ) -> c_int;
    pub fn pthread_join(thread: pthread_t, retval: *mut *mut c_void) -> c_int;
    pub fn pthread_detach(thread: pthread_t) -> c_int;
    pub fn pthread_self() -> pthread_t;
    pub fn pthread_exit(value: *mut c_void) -> !;
    pub fn pthread_attr_init(attr: *mut pthread_attr_t) -> c_int;
    pub fn pthread_attr_destroy(attr: *mut pthread_attr_t) -> c_int;
    pub fn pthread_attr_setstacksize(attr: *mut pthread_attr_t, stacksize: usize) -> c_int;
    pub fn pthread_attr_getstacksize(attr: *const pthread_attr_t, stacksize: *mut usize) -> c_int;
    pub fn pthread_attr_setdetachstate(attr: *mut pthread_attr_t, state: c_int) -> c_int;
    pub fn pthread_mutex_init(
        mutex: *mut pthread_mutex_t, attr: *const pthread_mutexattr_t,
    ) -> c_int;
    pub fn pthread_mutex_destroy(mutex: *mut pthread_mutex_t) -> c_int;
    pub fn pthread_mutex_lock(mutex: *mut pthread_mutex_t) -> c_int;
    pub fn pthread_mutex_trylock(mutex: *mut pthread_mutex_t) -> c_int;
    pub fn pthread_mutex_unlock(mutex: *mut pthread_mutex_t) -> c_int;
    pub fn pthread_mutexattr_init(attr: *mut pthread_mutexattr_t) -> c_int;
    pub fn pthread_mutexattr_destroy(attr: *mut pthread_mutexattr_t) -> c_int;
    pub fn pthread_mutexattr_settype(attr: *mut pthread_mutexattr_t, kind: c_int) -> c_int;
    pub fn pthread_cond_init(
        cond: *mut pthread_cond_t, attr: *const pthread_condattr_t,
    ) -> c_int;
    pub fn pthread_cond_destroy(cond: *mut pthread_cond_t) -> c_int;
    pub fn pthread_cond_signal(cond: *mut pthread_cond_t) -> c_int;
    pub fn pthread_cond_broadcast(cond: *mut pthread_cond_t) -> c_int;
    pub fn pthread_cond_wait(cond: *mut pthread_cond_t, mutex: *mut pthread_mutex_t) -> c_int;
    pub fn pthread_cond_timedwait(
        cond: *mut pthread_cond_t, mutex: *mut pthread_mutex_t, abstime: *const crate::timespec,
    ) -> c_int;
    pub fn pthread_rwlock_init(
        lock: *mut pthread_rwlock_t, attr: *const pthread_rwlockattr_t,
    ) -> c_int;
    pub fn pthread_rwlock_destroy(lock: *mut pthread_rwlock_t) -> c_int;
    pub fn pthread_rwlock_rdlock(lock: *mut pthread_rwlock_t) -> c_int;
    pub fn pthread_rwlock_wrlock(lock: *mut pthread_rwlock_t) -> c_int;
    pub fn pthread_rwlock_unlock(lock: *mut pthread_rwlock_t) -> c_int;
    pub fn pthread_key_create(
        key: *mut pthread_key_t, dtor: Option<unsafe extern "C" fn(*mut c_void)>,
    ) -> c_int;
    pub fn pthread_key_delete(key: pthread_key_t) -> c_int;
    pub fn pthread_getspecific(key: pthread_key_t) -> *mut c_void;
    pub fn pthread_setspecific(key: pthread_key_t, value: *const c_void) -> c_int;
    pub fn pthread_sigmask(how: c_int, set: *const sigset_t, oldset: *mut sigset_t) -> c_int;
    pub fn pthread_atfork(
        prepare: Option<unsafe extern "C" fn()>,
        parent: Option<unsafe extern "C" fn()>,
        child: Option<unsafe extern "C" fn()>,
    ) -> c_int;

    // Name resolution
    pub fn gethostbyname(name: *const c_char) -> *mut crate::hostent;
    pub fn inet_addr(cp: *const c_char) -> crate::in_addr_t;
    pub fn inet_ntoa(addr: in_addr) -> *mut c_char;
    pub fn inet_pton(af: c_int, src: *const c_char, dst: *mut c_void) -> c_int;
    pub fn inet_ntop(
        af: c_int, src: *const c_void, dst: *mut c_char, size: socklen_t,
    ) -> *const c_char;

    // Time
    pub fn clock_gettime(clk_id: clockid_t, tp: *mut crate::timespec) -> c_int;
    pub fn nanosleep(rqtp: *const crate::timespec, rmtp: *mut crate::timespec) -> c_int;
    pub fn usleep(usecs: useconds_t) -> c_int;
    pub fn sleep(seconds: c_uint) -> c_uint;
    pub fn alarm(seconds: c_uint) -> c_uint;
    pub fn time(t: *mut time_t) -> time_t;
    pub fn mktime(tm: *mut tm) -> time_t;
    pub fn localtime_r(t: *const time_t, result: *mut tm) -> *mut tm;
    pub fn gmtime_r(t: *const time_t, result: *mut tm) -> *mut tm;
    pub fn difftime(time1: time_t, time0: time_t) -> f64;

    // User/Group
    pub fn getpwnam(name: *const c_char) -> *mut passwd;
    pub fn getpwuid(uid: crate::uid_t) -> *mut passwd;
    pub fn getpwuid_r(
        uid: crate::uid_t, pwd: *mut passwd, buf: *mut c_char, buflen: usize,
        result: *mut *mut passwd,
    ) -> c_int;
    pub fn getpwnam_r(
        name: *const c_char, pwd: *mut passwd, buf: *mut c_char, buflen: usize,
        result: *mut *mut passwd,
    ) -> c_int;
    pub fn getgrnam(name: *const c_char) -> *mut crate::group;
    pub fn getgrgid(gid: crate::gid_t) -> *mut crate::group;
    pub fn getgroups(ngroups: c_int, groups: *mut crate::gid_t) -> c_int;

    // System
    pub fn sysconf(name: c_int) -> c_long;
    pub fn gethostname(name: *mut c_char, len: usize) -> c_int;
    pub fn getenv(name: *const c_char) -> *mut c_char;
    pub fn setenv(name: *const c_char, val: *const c_char, overwrite: c_int) -> c_int;
    pub fn unsetenv(name: *const c_char) -> c_int;
    pub fn abort() -> !;

    // Dynamic linking
    pub fn dlopen(filename: *const c_char, flag: c_int) -> *mut c_void;
    pub fn dlclose(handle: *mut c_void) -> c_int;
    pub fn dlsym(handle: *mut c_void, symbol: *const c_char) -> *mut c_void;
    pub fn dlerror() -> *mut c_char;

    // Error
    pub fn strerror(errnum: c_int) -> *mut c_char;
    pub fn strerror_r(errnum: c_int, buf: *mut c_char, buflen: usize) -> c_int;

    // I/O multiplexing
    pub fn select(
        nfds: c_int, readfds: *mut fd_set, writefds: *mut fd_set,
        errorfds: *mut fd_set, timeout: *mut crate::timeval,
    ) -> c_int;

    // Terminal
    pub fn ttyname(fd: c_int) -> *mut c_char;

    // --- Additional functions needed by solarish code path ---

    // Vectored I/O (provided by compat library on IRIX)
    pub fn preadv(fd: c_int, iov: *const crate::iovec, iovcnt: c_int, offset: off_t) -> isize;
    pub fn pwritev(fd: c_int, iov: *const crate::iovec, iovcnt: c_int, offset: off_t) -> isize;

    // dup3 (provided by compat library on IRIX)
    pub fn dup3(src: c_int, dst: c_int, flags: c_int) -> c_int;

    // Filesystem
    pub fn statfs(path: *const c_char, buf: *mut statfs) -> c_int;
    pub fn fstatfs(fd: c_int, buf: *mut statfs) -> c_int;
    pub fn faccessat(fd: c_int, path: *const c_char, amode: c_int, flag: c_int) -> c_int;
    pub fn mknodat(dirfd: c_int, pathname: *const c_char, mode: mode_t, dev: dev_t) -> c_int;
    pub fn flock(fd: c_int, operation: c_int) -> c_int;

    // Advisory/allocation (provided by compat library on IRIX)
    pub fn posix_fadvise(fd: c_int, offset: off_t, len: off_t, advise: c_int) -> c_int;
    pub fn posix_fallocate(fd: c_int, offset: off_t, len: off_t) -> c_int;

    // Sync
    pub fn sync();

    // Device number manipulation
    pub fn makedev(major: c_uint, minor: c_uint) -> dev_t;
    pub fn major(dev: dev_t) -> c_uint;
    pub fn minor(dev: dev_t) -> c_uint;
}

// --- Additional missing constants ---
pub const MSG_NONBLOCK: c_int = MSG_DONTWAIT;
pub const SEEK_DATA: c_int = 3;
pub const SEEK_HOLE: c_int = 4;
pub const CBAUD: tcflag_t = 0xf;
pub const CBAUDEX: tcflag_t = 0x10;
pub const EXTA: tcflag_t = B19200;
pub const EXTB: tcflag_t = B38400;
pub const IUCLC: tcflag_t = 0x200;
pub const VDSUSP: usize = 11;
pub const EBADE: c_int = 51;
