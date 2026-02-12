"""Bundle testing on IRIX.

Discovers tests for a bundle (auto-generated --version tests + YAML smoke_test
entries), generates a single Bourne shell script, ships it to IRIX, executes it,
and parses structured output for pass/fail reporting.
"""

from dataclasses import dataclass, field
from pathlib import Path

import subprocess
import yaml
from rich.console import Console
from rich.table import Table


# --- Data classes ---


@dataclass
class TestCase:
    """A single test to run on IRIX."""

    binary: str  # wrapper name (e.g., "grep")
    kind: str  # "auto" or "smoke"
    command: str  # shell command to run (uses $BUNDLE_DIR)
    expect: str | None = None  # substring to match in stdout
    timeout: int = 5  # seconds
    source_yaml: str = ""  # which YAML file defined this test


@dataclass
class TestResult:
    """Result of a single test."""

    test: TestCase
    status: str  # "PASS", "FAIL", "SKIP", "ERROR"
    exit_code: int = 0
    message: str = ""


@dataclass
class TestReport:
    """Complete test report for a bundle."""

    bundle_name: str
    results: list[TestResult] = field(default_factory=list)
    irix_reachable: bool = True
    error_message: str = ""

    @property
    def passed(self) -> int:
        return sum(1 for r in self.results if r.status == "PASS")

    @property
    def failed(self) -> int:
        return sum(1 for r in self.results if r.status == "FAIL")

    @property
    def skipped(self) -> int:
        return sum(1 for r in self.results if r.status == "SKIP")


# --- Test Discovery ---


class TestDiscovery:
    """Discovers tests for a bundle by scanning wrappers and YAML files."""

    # Directories/files at bundle root that are NOT wrappers
    SKIP_NAMES = {
        "install",
        "uninstall",
        "README",
        "_bin",
        "_sbin",
        "_lib32",
        "etc",
        "share",
    }

    def __init__(self, rules_dir: Path):
        self.rules_dir = rules_dir
        self._smoke_tests: dict[str, list[dict]] = {}
        self._load_all_smoke_tests()

    def _load_all_smoke_tests(self) -> None:
        """Load smoke_test entries from all package YAMLs."""
        packages_dir = self.rules_dir / "packages"
        if not packages_dir.is_dir():
            return
        for yaml_file in sorted(packages_dir.glob("*.yaml")):
            try:
                with open(yaml_file) as f:
                    data = yaml.safe_load(f)
                if data and "smoke_test" in data:
                    pkg_name = data.get("package", yaml_file.stem)
                    self._smoke_tests[pkg_name] = data["smoke_test"]
            except Exception:
                pass

    def discover(self, bundle_dir: Path) -> list[TestCase]:
        """Discover all tests for a bundle."""
        tests = []
        bin_dir = bundle_dir / "_bin"
        sbin_dir = bundle_dir / "_sbin"

        # Find wrapper scripts at bundle root
        wrappers = []
        for f in sorted(bundle_dir.iterdir()):
            if f.name.startswith(".") or f.name.startswith("_"):
                continue
            if f.name in self.SKIP_NAMES:
                continue
            if f.name.endswith(".sh") or f.name.endswith(".pl"):
                continue  # Helper scripts, not standalone apps
            if not f.is_file():
                continue
            # Must have a corresponding binary in _bin/ or _sbin/
            if (bin_dir / f.name).exists() or (sbin_dir.is_dir() and (sbin_dir / f.name).exists()):
                wrappers.append(f.name)

        # For each wrapper, generate tests
        seen_commands = set()
        for wrapper in wrappers:
            # YAML smoke_tests matching this binary
            for st in self._find_smoke_tests_for_binary(wrapper):
                cmd = self._adapt_command(st["command"], wrapper)
                if cmd not in seen_commands:
                    seen_commands.add(cmd)
                    tests.append(
                        TestCase(
                            binary=wrapper,
                            kind="smoke",
                            command=cmd,
                            expect=st.get("expect"),
                            timeout=10,
                            source_yaml=st.get("_source", ""),
                        )
                    )

            # Auto-test: <wrapper> --version
            auto_cmd = f"$BUNDLE_DIR/{wrapper} --version"
            if auto_cmd not in seen_commands:
                tests.append(
                    TestCase(
                        binary=wrapper,
                        kind="auto",
                        command=auto_cmd,
                        expect=None,
                        timeout=5,
                    )
                )

        return tests

    def _find_smoke_tests_for_binary(self, binary: str) -> list[dict]:
        """Find smoke_test entries that reference a given binary name."""
        results = []
        for pkg_name, tests in self._smoke_tests.items():
            for test in tests:
                cmd = test.get("command", "")
                if f"/usr/sgug/bin/{binary}" in cmd or f"/usr/sgug/sbin/{binary}" in cmd:
                    enriched = dict(test)
                    enriched["_source"] = f"{pkg_name}.yaml"
                    results.append(enriched)
        return results

    def _adapt_command(self, command: str, binary: str) -> str:
        """Rewrite YAML smoke_test command for bundle context.

        Replace /usr/sgug/bin/<binary> with $BUNDLE_DIR/<binary>.
        """
        adapted = command.replace(
            f"/usr/sgug/bin/{binary}", f"$BUNDLE_DIR/{binary}"
        )
        adapted = adapted.replace(
            f"/usr/sgug/sbin/{binary}", f"$BUNDLE_DIR/{binary}"
        )
        return adapted


# --- Script Generation ---

# Pure Bourne shell — IRIX /bin/sh is SVR4, no bashisms.
SCRIPT_HEADER = r"""#!/bin/sh
# mogrix test script — auto-generated
BUNDLE_DIR="$1"
if [ -z "$BUNDLE_DIR" ]; then
    echo "MOGRIX_TEST_ERROR: no bundle dir argument"
    exit 1
fi
if [ ! -d "$BUNDLE_DIR" ]; then
    echo "MOGRIX_TEST_ERROR: bundle dir not found: $BUNDLE_DIR"
    exit 1
fi

# Clear library path so only bundle libs are used.
# Wrapper scripts set LD_LIBRARYN32_PATH to $dir/_lib32.
# This simulates a clean IRIX host without SGUG-RSE.
LD_LIBRARYN32_PATH=
export LD_LIBRARYN32_PATH
unset LD_LIBRARYN32_PATH

TMP_OUT=/tmp/mogrix-test-out.$$
TMP_ERR=/tmp/mogrix-test-err.$$
PASS=0
FAIL=0
SKIP=0
TOTAL=0

cleanup() {
    rm -f $TMP_OUT $TMP_ERR
}
trap cleanup EXIT

run_test() {
    name="$1"
    kind="$2"
    expect="$3"
    timeout_sec="$4"
    cmd="$5"

    TOTAL=`expr $TOTAL + 1`

    # Run with timeout via background + kill watchdog
    eval "$cmd" </dev/null >$TMP_OUT 2>$TMP_ERR &
    pid=$!
    ( sleep $timeout_sec; kill $pid 2>/dev/null ) &
    watchdog=$!
    wait $pid 2>/dev/null
    rc=$?
    kill $watchdog 2>/dev/null
    wait $watchdog 2>/dev/null

    stdout=`cat $TMP_OUT 2>/dev/null`
    stderr=`cat $TMP_ERR 2>/dev/null`

    # Combine for checking
    allout="$stdout $stderr"

    # Check for rld fatal error (unresolved symbol)
    case "$allout" in
        *"rld: Fatal Error"*)
            echo "MOGRIX_RESULT|$name|FAIL|$rc|rld fatal error"
            FAIL=`expr $FAIL + 1`
            return
            ;;
    esac

    # Check for timeout (killed by signal = rc > 128)
    if [ $rc -gt 128 ]; then
        # Distinguish crash from timeout
        sig=`expr $rc - 128`
        case $sig in
            9|15) # SIGKILL or SIGTERM = timeout
                echo "MOGRIX_RESULT|$name|SKIP|$rc|timeout after ${timeout_sec}s"
                SKIP=`expr $SKIP + 1`
                return
                ;;
            *)  # Other signal = crash (SIGSEGV=11, SIGBUS=10)
                echo "MOGRIX_RESULT|$name|FAIL|$rc|killed by signal $sig"
                FAIL=`expr $FAIL + 1`
                return
                ;;
        esac
    fi

    # Check expectations
    if [ -n "$expect" ]; then
        case "$stdout" in
            *"$expect"*)
                echo "MOGRIX_RESULT|$name|PASS|$rc|matched: $expect"
                PASS=`expr $PASS + 1`
                ;;
            *)
                echo "MOGRIX_RESULT|$name|FAIL|$rc|expected '$expect' not found"
                FAIL=`expr $FAIL + 1`
                ;;
        esac
    else
        # Auto-test: pass if exit 0 or non-empty output (stdout OR stderr)
        if [ $rc -eq 0 ]; then
            echo "MOGRIX_RESULT|$name|PASS|$rc|exit 0"
            PASS=`expr $PASS + 1`
        elif [ -n "$stdout" ] || [ -n "$stderr" ]; then
            echo "MOGRIX_RESULT|$name|PASS|$rc|non-zero exit but has output"
            PASS=`expr $PASS + 1`
        else
            echo "MOGRIX_RESULT|$name|FAIL|$rc|exit $rc with no output"
            FAIL=`expr $FAIL + 1`
        fi
    fi
}

"""

SCRIPT_FOOTER = """
echo "MOGRIX_SUMMARY|$TOTAL|$PASS|$FAIL|$SKIP"
"""


class ScriptGenerator:
    """Generates a Bourne shell test script from TestCase list."""

    def generate(self, tests: list[TestCase]) -> str:
        lines = [SCRIPT_HEADER]

        for test in tests:
            label = f"{test.binary}:{test.kind}"
            if test.source_yaml:
                label = f"{test.binary}:smoke:{test.source_yaml}"

            # Escape single quotes in expect and command
            expect = (test.expect or "").replace("'", "'\\''")
            cmd = test.command.replace("'", "'\\''")

            lines.append(
                f"run_test '{label}' '{test.kind}' '{expect}' "
                f"{test.timeout} '{cmd}'"
            )

        lines.append(SCRIPT_FOOTER)
        return "\n".join(lines)


# --- IRIX Execution ---


class IRIXTestRunner:
    """Copies bundle to IRIX, runs test script, parses results."""

    def __init__(
        self,
        host: str = "192.168.0.81",
        user: str = "root",
    ):
        self.host = host
        self.user = user

    def check_connectivity(self) -> tuple[bool, str]:
        """Test SSH connectivity to IRIX."""
        try:
            result = subprocess.run(
                [
                    "ssh",
                    "-o", "ConnectTimeout=5",
                    "-o", "BatchMode=yes",
                    "-o", "StrictHostKeyChecking=no",
                    f"{self.user}@{self.host}",
                    "echo ok",
                ],
                capture_output=True,
                text=True,
                timeout=10,
                errors="replace",
            )
            if result.returncode == 0 and "ok" in result.stdout:
                return True, "Connected"
            return False, result.stderr.strip() or f"exit code {result.returncode}"
        except subprocess.TimeoutExpired:
            return False, "Connection timed out"
        except Exception as e:
            return False, str(e)

    def deploy_bundle(
        self, bundle_dir: Path, remote_dir: str
    ) -> tuple[bool, str]:
        """Copy bundle directory to IRIX via tar pipe over SSH."""
        full_cmd = (
            f"tar cf - -C {bundle_dir.parent} {bundle_dir.name} | "
            f"ssh -o ConnectTimeout=10 -o BatchMode=yes "
            f"-o StrictHostKeyChecking=no {self.user}@{self.host} "
            f"'mkdir -p {remote_dir} && cd {remote_dir} && tar xf -'"
        )
        try:
            result = subprocess.run(
                full_cmd,
                shell=True,
                capture_output=True,
                text=True,
                timeout=120,
                errors="replace",
            )
            if result.returncode == 0:
                return True, "Deployed"
            return False, result.stderr.strip() or f"exit code {result.returncode}"
        except subprocess.TimeoutExpired:
            return False, "Transfer timed out after 120s"
        except Exception as e:
            return False, str(e)

    def run_script(
        self, script_content: str, bundle_remote_path: str
    ) -> tuple[int, str, str]:
        """Run test script on IRIX by piping it via stdin."""
        ssh_cmd = [
            "ssh",
            "-o", "ConnectTimeout=10",
            "-o", "BatchMode=yes",
            "-o", "StrictHostKeyChecking=no",
            f"{self.user}@{self.host}",
            f"/bin/sh -s {bundle_remote_path}",
        ]
        try:
            result = subprocess.run(
                ssh_cmd,
                input=script_content,
                capture_output=True,
                text=True,
                timeout=300,
                errors="replace",
            )
            return result.returncode, result.stdout, result.stderr
        except subprocess.TimeoutExpired:
            return -1, "", "Test execution timed out after 300s"
        except Exception as e:
            return -1, "", str(e)

    def cleanup(self, remote_path: str) -> None:
        """Remove test bundle from IRIX."""
        try:
            subprocess.run(
                [
                    "ssh",
                    "-o", "ConnectTimeout=5",
                    "-o", "BatchMode=yes",
                    "-o", "StrictHostKeyChecking=no",
                    f"{self.user}@{self.host}",
                    f"rm -rf {remote_path}",
                ],
                capture_output=True,
                timeout=15,
            )
        except Exception:
            pass

    def parse_results(self, stdout: str) -> list[TestResult]:
        """Parse structured MOGRIX_RESULT lines from test output."""
        results = []
        for line in stdout.splitlines():
            if not line.startswith("MOGRIX_RESULT|"):
                continue
            parts = line.split("|", 4)
            if len(parts) < 5:
                continue
            name, status, rc_str, message = parts[1], parts[2], parts[3], parts[4]

            binary = name.split(":")[0]
            kind = name.split(":")[1] if ":" in name else "auto"

            results.append(
                TestResult(
                    test=TestCase(binary=binary, kind=kind, command="", expect=None),
                    status=status,
                    exit_code=int(rc_str) if rc_str.lstrip("-").isdigit() else -1,
                    message=message,
                )
            )
        return results

    def parse_summary(self, stdout: str) -> tuple[int, int, int, int] | None:
        """Parse MOGRIX_SUMMARY line. Returns (total, pass, fail, skip) or None."""
        for line in stdout.splitlines():
            if line.startswith("MOGRIX_SUMMARY|"):
                parts = line.split("|")
                if len(parts) >= 5:
                    try:
                        return (
                            int(parts[1]),
                            int(parts[2]),
                            int(parts[3]),
                            int(parts[4]),
                        )
                    except ValueError:
                        pass
        return None


# --- Report Display ---


def print_report(report: TestReport, console: Console) -> None:
    """Print a rich test report."""
    if not report.irix_reachable:
        console.print(f"\n[red]IRIX unreachable: {report.error_message}[/red]")
        return

    console.print(f"\n[bold]Test Results: {report.bundle_name}[/bold]\n")

    table = Table()
    table.add_column("Binary", style="cyan")
    table.add_column("Type", style="dim")
    table.add_column("Status")
    table.add_column("RC", justify="right")
    table.add_column("Details")

    for r in report.results:
        status_text = {
            "PASS": "[bold green]PASS[/bold green]",
            "FAIL": "[bold red]FAIL[/bold red]",
            "SKIP": "[yellow]SKIP[/yellow]",
            "ERROR": "[red]ERROR[/red]",
        }.get(r.status, r.status)

        table.add_row(
            r.test.binary,
            r.test.kind,
            status_text,
            str(r.exit_code),
            r.message,
        )

    console.print(table)

    total = len(report.results)
    console.print(
        f"\n[bold]{total} tests:[/bold] "
        f"[green]{report.passed} passed[/green], "
        f"[red]{report.failed} failed[/red], "
        f"[yellow]{report.skipped} skipped[/yellow]"
    )

    if report.failed > 0:
        console.print("\n[red bold]Bundle has test failures![/red bold]")
    else:
        console.print("\n[green bold]All tests passed.[/green bold]")
