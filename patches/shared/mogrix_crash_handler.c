/*
 * mogrix_crash_handler.c - Generic crash diagnostic handler for IRIX
 *
 * Installs signal handlers for SIGSEGV, SIGBUS, SIGABRT, SIGFPE, SIGPIPE,
 * SIGILL, SIGTRAP that dump registers, stack, and crash context to BOTH
 * stderr and a log file. Designed for cross-compiled MIPS n32 binaries
 * running on IRIX 6.5.
 *
 * Self-initializing: __attribute__((constructor)) calls mogrix_crash_init()
 * before main(). Only activates when MOGRIX_CRASH_DEBUG=1 is set.
 *
 * Output goes to:
 *   1. stderr (may be swallowed by parent process like GSubprocess)
 *   2. $MOGRIX_CRASH_DIR/mogrix_crash_<pid>.log (reliable — always works)
 *      Default MOGRIX_CRASH_DIR: current working directory
 *
 * Also installs atexit() handler to detect clean exits (exit() without
 * any signal). If the process dies and there's no log file at all, the
 * library wasn't loaded (check _RLDN32_LIST).
 *
 * Uses _rld_new_interface(_RLD_DLADDR) to resolve crash addresses to
 * library names and symbols — no manual address mapping needed.
 *
 * Usage:
 *   Method 1 (preload via libmogrix_compat.so — recommended):
 *     export MOGRIX_CRASH_DEBUG=1
 *     export MOGRIX_CRASH_DIR=/usr/people/edodd  # optional
 *     <run your bundle normally>
 *
 *   Method 2 (compile into a specific package):
 *     add_source: [mogrix_crash_handler.c, mogrix_crash_handler.h]
 *     # Then add mogrix_crash_handler.c to the build
 */

#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ucontext.h>
#include <fcntl.h>

/* IRIX rld interface for dladdr.
 * _rld_new_interface is provided by IRIX rld at runtime (not in any .so
 * at cross-compile time). We define what we need manually to avoid
 * link-time dependency on the symbol. */
#define _RLD_DLADDR 14

typedef struct {
    const char *dli_fname;
    void       *dli_fbase;
    const char *dli_sname;
    void       *dli_saddr;
    int         dli_version;
    int         dli_reserved1;
    long        dli_reserved[4];
} mogrix_Dl_info;

/* _rld_new_interface is resolved by rld itself. Declare weak so the
 * cross-linker doesn't reject it with --no-undefined. At runtime on
 * IRIX, rld always provides this symbol. */
__attribute__((weak)) void *_rld_new_interface(unsigned long op, ...);

/* Function pointer cached at init time */
typedef void *(*rld_fn_t)(unsigned long, ...);
static rld_fn_t rld_dladdr_fn = 0;

/* Log file fd — set in signal handler or atexit handler before output */
static int log_fd = -1;

/* Crash log directory (from MOGRIX_CRASH_DIR, default ".") */
static char crash_dir[256] = ".";

/* Whether we're initialized */
static int initialized = 0;


/* ---------- async-signal-safe output helpers ---------- */
/* All output goes to BOTH stderr AND log_fd (if open) */

static void wbuf(const char *buf, size_t len) {
    write(STDERR_FILENO, buf, len);
    if (log_fd >= 0) write(log_fd, buf, len);
}

static void ws(const char *s) {
    if (s) wbuf(s, strlen(s));
}

static void wc(char c) {
    wbuf(&c, 1);
}

static void whex32(unsigned int val) {
    static const char hex[] = "0123456789abcdef";
    char buf[10];
    buf[0] = '0'; buf[1] = 'x';
    buf[2] = hex[(val >> 28) & 0xf];
    buf[3] = hex[(val >> 24) & 0xf];
    buf[4] = hex[(val >> 20) & 0xf];
    buf[5] = hex[(val >> 16) & 0xf];
    buf[6] = hex[(val >> 12) & 0xf];
    buf[7] = hex[(val >>  8) & 0xf];
    buf[8] = hex[(val >>  4) & 0xf];
    buf[9] = hex[(val      ) & 0xf];
    wbuf(buf, 10);
}

static void wdec(int val) {
    char buf[12];
    int i = 0;
    unsigned int uval;
    if (val < 0) { wc('-'); uval = (unsigned int)(-val); }
    else uval = (unsigned int)val;
    if (uval == 0) { wc('0'); return; }
    while (uval > 0) { buf[i++] = '0' + (uval % 10); uval /= 10; }
    while (--i >= 0) wc(buf[i]);
}

static void wreg(const char *name, unsigned int val) {
    ws(name);
    whex32(val);
    wc('\n');
}

/* ---------- path construction (async-signal-safe) ---------- */

static void build_crash_path(char *buf, size_t bufsz, const char *suffix) {
    const char *dir = crash_dir;
    int pid = getpid();
    char pidbuf[12];
    int pidlen = 0;
    int pval = pid;
    int i, j;

    /* Convert PID to string */
    if (pval == 0) { pidbuf[pidlen++] = '0'; }
    else { while (pval > 0) { pidbuf[pidlen++] = '0' + (pval % 10); pval /= 10; } }
    /* Reverse */
    for (i = 0; i < pidlen / 2; i++) {
        char t = pidbuf[i];
        pidbuf[i] = pidbuf[pidlen - 1 - i];
        pidbuf[pidlen - 1 - i] = t;
    }

    /* Construct: <dir>/mogrix_<suffix>_<pid>.log */
    i = 0;
    while (*dir && i < (int)bufsz - 1) buf[i++] = *dir++;
    if (i > 0 && buf[i-1] != '/') buf[i++] = '/';

    { const char *p = "mogrix_"; while (*p && i < (int)bufsz - 1) buf[i++] = *p++; }
    { const char *p = suffix;    while (*p && i < (int)bufsz - 1) buf[i++] = *p++; }
    buf[i++] = '_';
    for (j = 0; j < pidlen && i < (int)bufsz - 1; j++) buf[i++] = pidbuf[j];
    { const char *p = ".log";    while (*p && i < (int)bufsz - 1) buf[i++] = *p++; }
    buf[i] = '\0';
}

/* ---------- address resolution via IRIX rld ---------- */

static void resolve_addr(unsigned int addr, const char *label) {
    mogrix_Dl_info info;
    void *result;

    ws("  ");
    ws(label);
    ws(": ");
    whex32(addr);

    memset(&info, 0, sizeof(info));
    result = rld_dladdr_fn ? rld_dladdr_fn(_RLD_DLADDR, addr, &info) : 0;

    if (result && info.dli_fname) {
        ws(" in ");
        ws(info.dli_fname);
        if (info.dli_fbase) {
            ws(" [base ");
            whex32((unsigned int)(unsigned long)info.dli_fbase);
            ws(", offset +");
            whex32(addr - (unsigned int)(unsigned long)info.dli_fbase);
            ws("]");
        }
        if (info.dli_sname) {
            ws("\n         nearest symbol: ");
            ws(info.dli_sname);
            ws(" at ");
            whex32((unsigned int)(unsigned long)info.dli_saddr);
            ws(" (+");
            wdec(addr - (unsigned int)(unsigned long)info.dli_saddr);
            ws(")");
        }
    } else {
        ws(" (unknown -- dladdr failed)");
    }
    wc('\n');
}

/* ---------- signal handler ---------- */

/* Re-entrancy guard: prevents infinite recursion if handler itself crashes */
static volatile int in_handler = 0;

static void crash_handler(int sig, siginfo_t *si, void *ctx) {
    ucontext_t *uc = (ucontext_t *)ctx;
    unsigned int pc, ra, sp, gp;
    unsigned int *stack;
    int i;
    char path[512];

    /* Re-entrancy guard — if handler crashes, just die immediately */
    if (in_handler) {
        const char *msg = "[mogrix] RE-ENTRANT CRASH in handler, dying\n";
        write(STDERR_FILENO, msg, 44);
        _exit(128 + sig);
    }
    in_handler = 1;

    /* Open log file for this crash */
    build_crash_path(path, sizeof(path), "crash");
    log_fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0666);

    ws("\n");
    ws("======================================================\n");
    ws("         MOGRIX CRASH DIAGNOSTIC HANDLER\n");
    ws("======================================================\n\n");

    /* Signal info */
    ws("Signal: ");
    switch (sig) {
        case SIGSEGV: ws("SIGSEGV (segmentation fault)"); break;
        case SIGBUS:  ws("SIGBUS (bus error)"); break;
        case SIGABRT: ws("SIGABRT (abort)"); break;
        case SIGFPE:  ws("SIGFPE (floating point exception)"); break;
        case SIGPIPE: ws("SIGPIPE (broken pipe)"); break;
        case SIGILL:  ws("SIGILL (illegal instruction)"); break;
        case SIGTRAP: ws("SIGTRAP (trace trap)"); break;
        default:      ws("signal "); wdec(sig); break;
    }
    ws("\nPID: ");
    wdec(getpid());
    ws("\n");

    /* NULL-check si before accessing fields */
    if (!si) {
        ws("siginfo: NULL\n");
    } else {
        if (sig == SIGSEGV) {
            ws("SEGV code: ");
            switch (si->si_code) {
                case SEGV_MAPERR: ws("SEGV_MAPERR (address not mapped)"); break;
                case SEGV_ACCERR: ws("SEGV_ACCERR (invalid permissions)"); break;
                default: ws("code="); wdec(si->si_code); break;
            }
            ws("\n");
        } else if (sig == SIGBUS) {
            ws("BUS code: ");
            switch (si->si_code) {
                case BUS_ADRALN: ws("BUS_ADRALN (alignment error)"); break;
                case BUS_ADRERR: ws("BUS_ADRERR (nonexistent address)"); break;
                case BUS_OBJERR: ws("BUS_OBJERR (object-specific)"); break;
                default: ws("code="); wdec(si->si_code); break;
            }
            ws("\n");
        } else if (sig == SIGPIPE) {
            ws("SIGPIPE: write to a pipe/socket with no reader.\n");
            ws("  Common cause: MSG_NOSIGNAL is 0 on IRIX (no effect).\n");
            ws("  Fix: SIG_IGN SIGPIPE at process start, handle EPIPE returns.\n");
        }

        ws("Fault addr: ");
        whex32((unsigned int)(unsigned long)si->si_addr);
        ws("\n");
    }
    ws("Log file: ");
    ws(path);
    ws("\n\n");

    /* NULL-check ctx before accessing ucontext registers */
    if (!ctx) {
        ws("ucontext: NULL (no register info available)\n\n");
        pc = ra = sp = gp = 0;
        goto skip_registers;
    }

    /* Key registers — cast from machreg_t (64-bit on n32) to 32-bit */
    pc = (unsigned int)uc->uc_mcontext.__gregs[CTX_EPC];
    ra = (unsigned int)uc->uc_mcontext.__gregs[CTX_RA];
    sp = (unsigned int)uc->uc_mcontext.__gregs[CTX_SP];
    gp = (unsigned int)uc->uc_mcontext.__gregs[CTX_GP];

    /* Resolve PC and RA to library/symbol names */
    ws("-- Crash Location (resolved via rld) -----------------\n");
    resolve_addr(pc, "PC ");
    resolve_addr(ra, "RA ");
    ws("\n");

    ws("-- Key Registers -------------------------------------\n");
    wreg("PC  (EPC) : ", pc);
    wreg("RA        : ", ra);
    wreg("SP        : ", sp);
    wreg("GP        : ", gp);
    ws("\n");

    /* Full general register dump */
    ws("-- All General Registers (n32) -----------------------\n");
    {
        static const char *rnames[] = {
            "zero", "at  ", "v0  ", "v1  ",
            "a0  ", "a1  ", "a2  ", "a3  ",
            "a4  ", "a5  ", "a6  ", "a7  ",  /* n32: a4-a7 (not t0-t3) */
            "t0  ", "t1  ", "t2  ", "t3  ",
            "s0  ", "s1  ", "s2  ", "s3  ",
            "s4  ", "s5  ", "s6  ", "s7  ",
            "t8  ", "t9  ", "k0  ", "k1  ",
            "gp  ", "sp  ", "s8  ", "ra  "
        };
        for (i = 0; i < 32; i++) {
            ws("  $");
            ws(rnames[i]);
            ws(" = ");
            whex32((unsigned int)uc->uc_mcontext.__gregs[i]);
            if (i % 4 == 3) wc('\n');
            else ws("  ");
        }
    }
    ws("\n");

    /* CAUSE and SR registers */
    ws("-- Special Registers ---------------------------------\n");
    wreg("CAUSE     : ", (unsigned int)uc->uc_mcontext.__gregs[CTX_CAUSE]);
    wreg("SR        : ", (unsigned int)uc->uc_mcontext.__gregs[CTX_SR]);
    wreg("MDLO      : ", (unsigned int)uc->uc_mcontext.__gregs[CTX_MDLO]);
    wreg("MDHI      : ", (unsigned int)uc->uc_mcontext.__gregs[CTX_MDHI]);
    ws("\n");

    /* Hint: what was at the faulting address? */
    ws("-- Crash Analysis Hints ------------------------------\n");
    if (si && si->si_addr == (void *)0) {
        ws("  NULL pointer dereference\n");
    } else if (si && (unsigned int)(unsigned long)si->si_addr < 0x1000) {
        ws("  Low address -- likely NULL pointer + small offset (struct field)\n");
        ws("  Offset from NULL: ");
        wdec((int)(unsigned long)si->si_addr);
        ws(" bytes\n");
    } else if (pc == 0) {
        ws("  PC is NULL -- jumped through NULL function pointer\n");
    }
    if (ra == 0) {
        ws("  RA is NULL -- corrupted return address or top-level crash\n");
    }
    if (sig == SIGPIPE) {
        ws("  SIGPIPE in subprocess -- parent probably closed the IPC socket\n");
        ws("  Or: this process wrote to a pipe/socket before the reader connected\n");
    }
    ws("\n");

skip_registers:
    if (sp == 0) {
        ws("-- Stack Backtrace: SP is NULL, no stack trace --\n\n");
        ws("======================================================\n\n");
        if (log_fd >= 0) { close(log_fd); log_fd = -1; }
        _exit(128 + sig);
    }

    /* Resolve potential return addresses on stack */
    ws("-- Stack Backtrace (resolved) ------------------------\n");
    stack = (unsigned int *)sp;
    {
        int found = 0;
        mogrix_Dl_info info;
        void *result;

        for (i = 0; i < 256 && found < 20; i++) {
            unsigned int val = stack[i];
            /* Heuristic: typical IRIX n32 text segment ranges */
            if ((val >= 0x00400000 && val < 0x02000000) ||   /* main exe */
                (val >= 0x0fa00000 && val < 0x60000000)) {   /* shared libs */
                memset(&info, 0, sizeof(info));
                result = rld_dladdr_fn ? rld_dladdr_fn(_RLD_DLADDR, val, &info) : 0;
                ws("  SP+");
                wdec(i * 4);
                ws(": ");
                whex32(val);
                if (result && info.dli_sname) {
                    ws("  ");
                    ws(info.dli_sname);
                    if (info.dli_saddr) {
                        ws("+");
                        wdec(val - (unsigned int)(unsigned long)info.dli_saddr);
                    }
                    if (info.dli_fname) {
                        ws(" [");
                        /* Print just the filename, not full path */
                        const char *p = info.dli_fname;
                        const char *last_slash = p;
                        while (*p) { if (*p == '/') last_slash = p + 1; p++; }
                        ws(last_slash);
                        ws("]");
                    }
                } else if (result && info.dli_fname) {
                    ws("  (no symbol) [");
                    const char *p = info.dli_fname;
                    const char *last_slash = p;
                    while (*p) { if (*p == '/') last_slash = p + 1; p++; }
                    ws(last_slash);
                    ws("]");
                }
                wc('\n');
                found++;
            }
        }
        if (found == 0) ws("  (none found in scan range)\n");
    }
    ws("\n");

    /* Raw stack dump — 32 words */
    ws("-- Stack Dump (32 words from SP) ---------------------\n");
    for (i = 0; i < 32; i++) {
        if (i % 4 == 0) {
            ws("  ");
            whex32((unsigned int)(unsigned long)&stack[i]);
            ws(": ");
        }
        whex32(stack[i]);
        if (i % 4 == 3) wc('\n');
        else wc(' ');
    }
    ws("\n");

    ws("======================================================\n\n");

    /* Close log file before exiting */
    if (log_fd >= 0) { close(log_fd); log_fd = -1; }

    /* Exit immediately — don't raise() which can re-enter the handler on IRIX.
     * IRIX signal()/sigaction() interaction quirk: signal(sig, SIG_DFL) may
     * not override a sigaction-installed handler, causing raise() to re-enter
     * this handler with NULL si/ctx → crash at si->si_addr (offset 12). */
    _exit(128 + sig);
}

/* ---------- atexit handler ---------- */

static void exit_handler(void) {
    char path[512];
    if (!initialized) return;

    build_crash_path(path, sizeof(path), "exit");
    log_fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0666);

    ws("[mogrix] Process exiting normally (atexit handler)\n");
    ws("PID: ");
    wdec(getpid());
    ws("\nLog dir: ");
    ws(crash_dir);
    ws("\n");

    if (log_fd >= 0) { close(log_fd); log_fd = -1; }
}

/* ---------- initialization ---------- */

void mogrix_crash_init(void) {
    const char *env = getenv("MOGRIX_CRASH_DEBUG");
    const char *dir_env;
    struct sigaction sa;
    char path[512];

    if (!env || env[0] == '0' || env[0] == '\0') return;

    /* Set crash log directory */
    dir_env = getenv("MOGRIX_CRASH_DIR");
    if (dir_env && dir_env[0] != '\0') {
        size_t len = strlen(dir_env);
        if (len >= sizeof(crash_dir)) len = sizeof(crash_dir) - 1;
        memcpy(crash_dir, dir_env, len);
        crash_dir[len] = '\0';
    }

    /* Cache rld function pointer for dladdr resolution.
     * _rld_new_interface is weak — always present on IRIX at runtime. */
    rld_dladdr_fn = _rld_new_interface;

    memset(&sa, 0, sizeof(sa));
    sa.sa_sigaction = crash_handler;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);

    sigaction(SIGSEGV, &sa, NULL);
    sigaction(SIGBUS, &sa, NULL);
    sigaction(SIGABRT, &sa, NULL);
    sigaction(SIGFPE, &sa, NULL);
    /* NOTE: Do NOT catch SIGPIPE — WebKit sets SIGPIPE to SIG_IGN during
     * initialization. Catching it here would kill WebProcess/NetworkProcess
     * on normal broken-pipe conditions (e.g. IPC socket close). */
    sigaction(SIGILL, &sa, NULL);
    sigaction(SIGTRAP, &sa, NULL);

    /* Register atexit handler for clean exit detection */
    atexit(exit_handler);

    initialized = 1;

    /* Write init confirmation to both stderr and a log file */
    build_crash_path(path, sizeof(path), "init");
    log_fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0666);

    ws("[mogrix] Crash handler installed (MOGRIX_CRASH_DEBUG=1)\n");
    ws("  PID: ");
    wdec(getpid());
    ws("\n  Signals: SEGV BUS ABRT FPE ILL TRAP (not PIPE)\n");
    ws("  Log dir: ");
    ws(crash_dir);
    ws("\n  Crash logs: ");
    ws(crash_dir);
    ws("/mogrix_crash_<pid>.log\n");

    if (log_fd >= 0) { close(log_fd); log_fd = -1; }
}

/*
 * NOTE: _exit() interposition does NOT work on IRIX.
 * When libc's exit() internally calls _exit(), IRIX rld does not set up
 * $t9 for the interposed function, causing GP to be wrong → SIGBUS.
 * This is a fundamental limitation of IRIX rld symbol interposition for
 * internal libc calls.
 *
 * To detect _exit() calls from subprocesses, use par tracing instead:
 *   par -si MiniBrowser about:blank  (traces child processes too)
 *   Look for "exit" syscall in child process output.
 */

/* Auto-initialize before main() */
__attribute__((constructor))
static void mogrix_crash_auto_init(void) {
    mogrix_crash_init();
}
