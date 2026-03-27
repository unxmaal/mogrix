/*
 * Layer 0 (non-interactive): Validate struct layouts and signals
 * without requiring a terminal. Can run over SSH.
 *
 * Cross-compile: irix-cc -o layer0_noterm layer0_noterm.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <poll.h>
#include <stddef.h>
#include <errno.h>

static volatile int got_sigwinch = 0;
static void sigwinch_handler(int sig) { (void)sig; got_sigwinch = 1; }

static int pass_count = 0, fail_count = 0;
#define PASS(fmt, ...) do { pass_count++; printf("PASS: " fmt "\n", ##__VA_ARGS__); } while(0)
#define FAIL(fmt, ...) do { fail_count++; printf("FAIL: " fmt "\n", ##__VA_ARGS__); } while(0)
#define CHECK(cond, name) do { if (cond) PASS(name); else FAIL(name); } while(0)

int main(void) {
    printf("=== Layer 0 (non-interactive): Struct Layout Validation ===\n\n");

    /* --- struct termios layout --- */
    printf("--- struct termios ---\n");
    printf("sizeof(struct termios) = %d\n", (int)sizeof(struct termios));
    printf("sizeof(tcflag_t) = %d\n", (int)sizeof(tcflag_t));
    printf("sizeof(cc_t) = %d\n", (int)sizeof(cc_t));
    CHECK(sizeof(struct termios) == 48, "termios size == 48");
    CHECK(sizeof(tcflag_t) == 4, "tcflag_t size == 4");
    CHECK(sizeof(cc_t) == 1, "cc_t size == 1");

    /* Field offsets — must match Rust libc bindings */
    printf("offsetof(c_iflag) = %d\n", (int)offsetof(struct termios, c_iflag));
    printf("offsetof(c_oflag) = %d\n", (int)offsetof(struct termios, c_oflag));
    printf("offsetof(c_cflag) = %d\n", (int)offsetof(struct termios, c_cflag));
    printf("offsetof(c_lflag) = %d\n", (int)offsetof(struct termios, c_lflag));
    printf("offsetof(c_cc)    = %d\n", (int)offsetof(struct termios, c_cc));
    CHECK(offsetof(struct termios, c_iflag) == 0, "c_iflag at offset 0");
    CHECK(offsetof(struct termios, c_oflag) == 4, "c_oflag at offset 4");
    CHECK(offsetof(struct termios, c_cflag) == 8, "c_cflag at offset 8");
    CHECK(offsetof(struct termios, c_lflag) == 12, "c_lflag at offset 12");

    /* NCCS (number of c_cc entries) */
    printf("NCCS = %d\n", NCCS);
    CHECK(NCCS == 23, "NCCS == 23");

    /* --- struct winsize layout --- */
    printf("\n--- struct winsize ---\n");
    printf("sizeof(struct winsize) = %d\n", (int)sizeof(struct winsize));
    CHECK(sizeof(struct winsize) == 8, "winsize size == 8");

    /* --- termios flag constants --- */
    printf("\n--- termios constants ---\n");
    printf("ICANON = 0x%x\n", ICANON);
    printf("ECHO   = 0x%x\n", ECHO);
    printf("ISIG   = 0x%x\n", ISIG);
    printf("IEXTEN = 0x%x\n", IEXTEN);
    printf("OPOST  = 0x%x\n", OPOST);
    printf("IXON   = 0x%x\n", IXON);
    printf("ICRNL  = 0x%x\n", ICRNL);
    printf("VMIN   = %d\n", VMIN);
    printf("VTIME  = %d\n", VTIME);
    /* These must match what Rust crossterm uses */
    CHECK(ICANON != 0, "ICANON defined");
    CHECK(ECHO != 0, "ECHO defined");

    /* --- ioctl constants --- */
    printf("\n--- ioctl constants ---\n");
    printf("TIOCGWINSZ = 0x%lx\n", (unsigned long)TIOCGWINSZ);
    printf("FIONBIO    = 0x%lx\n", (unsigned long)FIONBIO);
    printf("FIONREAD   = 0x%lx\n", (unsigned long)FIONREAD);
    CHECK(TIOCGWINSZ != 0, "TIOCGWINSZ defined");
    CHECK(FIONBIO != 0, "FIONBIO defined");

    /* --- TCSANOW/TCSADRAIN/TCSAFLUSH --- */
    printf("\n--- tcsetattr actions ---\n");
    printf("TCSANOW   = %d\n", TCSANOW);
    printf("TCSADRAIN = %d\n", TCSADRAIN);
    printf("TCSAFLUSH = %d\n", TCSAFLUSH);
    CHECK(TCSANOW >= 0, "TCSANOW defined");

    /* --- Signals --- */
    printf("\n--- Signals ---\n");
    printf("SIGWINCH = %d\n", SIGWINCH);
    CHECK(SIGWINCH > 0, "SIGWINCH defined");
    {
        struct sigaction sa;
        memset(&sa, 0, sizeof(sa));
        sa.sa_handler = sigwinch_handler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_RESTART;
        CHECK(sigaction(SIGWINCH, &sa, NULL) == 0, "sigaction(SIGWINCH)");

        got_sigwinch = 0;
        kill(getpid(), SIGWINCH);
        usleep(10000);
        CHECK(got_sigwinch == 1, "SIGWINCH delivered and caught");
    }

    /* --- poll() on a pipe (non-tty equivalent) --- */
    printf("\n--- poll() ---\n");
    {
        int pipefd[2];
        if (pipe(pipefd) == 0) {
            write(pipefd[1], "x", 1);
            struct pollfd pfd;
            pfd.fd = pipefd[0];
            pfd.events = POLLIN;
            pfd.revents = 0;
            int ret = poll(&pfd, 1, 100);
            CHECK(ret == 1, "poll() detects readable pipe");
            CHECK((pfd.revents & POLLIN) != 0, "POLLIN set in revents");
            close(pipefd[0]);
            close(pipefd[1]);
        } else {
            FAIL("pipe() failed: %s", strerror(errno));
        }
    }

    printf("\n=== Results: %d passed, %d failed ===\n", pass_count, fail_count);
    return fail_count > 0 ? 1 : 0;
}
