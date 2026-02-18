/*
 * mogrix_crash_handler.c - Generic crash diagnostic handler for IRIX
 *
 * Installs signal handlers for SIGSEGV, SIGBUS, SIGABRT, SIGFPE that
 * dump registers, stack, and crash context to stderr. Designed for
 * cross-compiled MIPS n32 binaries running on IRIX 6.5.
 *
 * Self-initializing: __attribute__((constructor)) calls mogrix_crash_init()
 * before main(). Only activates when MOGRIX_CRASH_DEBUG=1 is set.
 *
 * Uses _rld_new_interface(_RLD_DLADDR) to resolve crash addresses to
 * library names and symbols — no manual address mapping needed.
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

/* ---------- async-signal-safe output helpers ---------- */

static void ws(const char *s) {
    if (s) write(STDERR_FILENO, s, strlen(s));
}

static void wc(char c) {
    write(STDERR_FILENO, &c, 1);
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
    write(STDERR_FILENO, buf, 10);
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
        ws(" (unknown — dladdr failed)");
    }
    wc('\n');
}

/* ---------- signal handler ---------- */

static void crash_handler(int sig, siginfo_t *si, void *ctx) {
    ucontext_t *uc = (ucontext_t *)ctx;
    unsigned int pc, ra, sp, gp;
    unsigned int *stack;
    int i;

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
        default:      ws("signal "); wdec(sig); break;
    }
    ws("\n");

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
    }

    ws("Fault addr: ");
    whex32((unsigned int)(unsigned long)si->si_addr);
    ws("\n\n");

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
    if (si->si_addr == (void *)0) {
        ws("  NULL pointer dereference\n");
    } else if ((unsigned int)(unsigned long)si->si_addr < 0x1000) {
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
    ws("\n");

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

    /* Re-raise with default handler to get core dump */
    signal(sig, SIG_DFL);
    raise(sig);
}

/* ---------- initialization ---------- */

void mogrix_crash_init(void) {
    const char *env = getenv("MOGRIX_CRASH_DEBUG");
    struct sigaction sa;

    if (!env || env[0] == '0' || env[0] == '\0') return;

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

    ws("[mogrix] Crash handler installed (MOGRIX_CRASH_DEBUG=1)\n");
}

/* Auto-initialize before main() */
__attribute__((constructor))
static void mogrix_crash_auto_init(void) {
    mogrix_crash_init();
}
