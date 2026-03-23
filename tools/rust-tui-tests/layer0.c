/*
 * Layer 0: IRIX Terminal Primitives Test
 *
 * Tests the C-level terminal I/O that Rust TUI apps depend on:
 *   1. struct termios size and layout
 *   2. TIOCGWINSZ (terminal size query)
 *   3. Raw mode (disable ICANON/ECHO)
 *   4. poll() on stdin (event multiplexing)
 *   5. read() in non-blocking raw mode
 *   6. ANSI escape code output (colors, cursor, box-drawing)
 *   7. SIGWINCH delivery
 *
 * Cross-compile:
 *   irix-cc -o layer0 layer0.c
 *
 * Deploy + run on each terminal type:
 *   irix_copy_to local_path=layer0 remote_path=/usr/people/edodd/layer0 host_path=true owner=edodd
 *   test_binary binary=/usr/people/edodd/layer0 host_mode=true
 *
 * Run on: IRIX console, aterm, urxvt, SSH
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <poll.h>
#include <errno.h>

static volatile int got_sigwinch = 0;

static void sigwinch_handler(int sig) {
    (void)sig;
    got_sigwinch = 1;
}

static int pass_count = 0;
static int fail_count = 0;

#define PASS(fmt, ...) do { pass_count++; printf("PASS: " fmt "\n", ##__VA_ARGS__); } while(0)
#define FAIL(fmt, ...) do { fail_count++; printf("FAIL: " fmt "\n", ##__VA_ARGS__); } while(0)

int main(void) {
    struct termios orig, raw;
    struct winsize ws;
    int rc;

    printf("=== Layer 0: IRIX Terminal Primitives ===\n");
    printf("TERM=%s\n", getenv("TERM") ? getenv("TERM") : "(unset)");
    printf("isatty(0)=%d isatty(1)=%d\n\n", isatty(0), isatty(1));

    /* --- Test 1: struct sizes --- */
    printf("--- Struct Sizes ---\n");
    printf("sizeof(struct termios) = %d\n", (int)sizeof(struct termios));
    printf("sizeof(struct winsize) = %d\n", (int)sizeof(struct winsize));
    printf("sizeof(tcflag_t) = %d\n", (int)sizeof(tcflag_t));
    printf("sizeof(cc_t) = %d\n", (int)sizeof(cc_t));
    if (sizeof(struct termios) == 48)
        PASS("struct termios is 48 bytes (matches Rust libc)");
    else
        FAIL("struct termios is %d bytes, expected 48", (int)sizeof(struct termios));

    if (sizeof(struct winsize) == 8)
        PASS("struct winsize is 8 bytes");
    else
        FAIL("struct winsize is %d bytes, expected 8", (int)sizeof(struct winsize));

    /* --- Test 2: TIOCGWINSZ --- */
    printf("\n--- Terminal Size ---\n");
    memset(&ws, 0, sizeof(ws));
    rc = ioctl(0, TIOCGWINSZ, &ws);
    if (rc == 0 && ws.ws_col > 0 && ws.ws_row > 0) {
        PASS("TIOCGWINSZ: %d cols x %d rows (%dx%d pixels)",
             ws.ws_col, ws.ws_row, ws.ws_xpixel, ws.ws_ypixel);
    } else {
        FAIL("TIOCGWINSZ: rc=%d errno=%d (%s) cols=%d rows=%d",
             rc, errno, strerror(errno), ws.ws_col, ws.ws_row);
    }

    /* --- Test 3: tcgetattr --- */
    printf("\n--- termios ---\n");
    memset(&orig, 0, sizeof(orig));
    rc = tcgetattr(0, &orig);
    if (rc == 0) {
        PASS("tcgetattr succeeded");
        printf("  c_iflag=0x%08lx c_oflag=0x%08lx\n",
               (unsigned long)orig.c_iflag, (unsigned long)orig.c_oflag);
        printf("  c_cflag=0x%08lx c_lflag=0x%08lx\n",
               (unsigned long)orig.c_cflag, (unsigned long)orig.c_lflag);
        printf("  ICANON=%s ECHO=%s ISIG=%s\n",
               (orig.c_lflag & ICANON) ? "on" : "OFF",
               (orig.c_lflag & ECHO) ? "on" : "OFF",
               (orig.c_lflag & ISIG) ? "on" : "OFF");
    } else {
        FAIL("tcgetattr: errno=%d (%s)", errno, strerror(errno));
    }

    /* --- Test 4: Raw mode --- */
    printf("\n--- Raw Mode ---\n");
    raw = orig;
    raw.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK | ISIG | IEXTEN);
    raw.c_iflag &= ~(IXON | IXOFF | ICRNL | INLCR | IGNCR);
    raw.c_oflag &= ~(OPOST);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;
    rc = tcsetattr(0, TCSANOW, &raw);
    if (rc == 0) {
        PASS("tcsetattr raw mode");
    } else {
        FAIL("tcsetattr raw: errno=%d (%s)", errno, strerror(errno));
        /* Can't continue without raw mode */
        return 1;
    }

    /* Verify raw mode took effect */
    {
        struct termios check;
        tcgetattr(0, &check);
        if (!(check.c_lflag & ICANON) && !(check.c_lflag & ECHO))
            PASS("raw mode verified (ICANON=off ECHO=off)");
        else
            FAIL("raw mode NOT applied: ICANON=%s ECHO=%s",
                 (check.c_lflag & ICANON) ? "on" : "off",
                 (check.c_lflag & ECHO) ? "on" : "off");
    }

    /* --- Test 5: ANSI output --- */
    /* Note: in raw mode, we need \r\n not just \n */
    printf("\r\n--- ANSI Output ---\r\n");
    printf("\033[31mRed\033[0m \033[32mGreen\033[0m \033[34mBlue\033[0m \033[1mBold\033[0m \033[4mUnderline\033[0m\r\n");
    /* Box-drawing with Unicode (UTF-8) */
    printf("Box (ASCII):  +--+\r\n");
    printf("              |  |\r\n");
    printf("              +--+\r\n");
    /* Box-drawing with Unicode if terminal supports it */
    printf("Box (UTF-8):  \xe2\x94\x8c\xe2\x94\x80\xe2\x94\x80\xe2\x94\x90\r\n");
    printf("              \xe2\x94\x82  \xe2\x94\x82\r\n");
    printf("              \xe2\x94\x94\xe2\x94\x80\xe2\x94\x80\xe2\x94\x98\r\n");
    printf("(If UTF-8 box looks garbled, terminal lacks UTF-8 support)\r\n");
    PASS("ANSI output written (visual check required)");

    /* --- Test 6: poll() on stdin --- */
    printf("\r\n--- poll() on stdin ---\r\n");
    printf("Press any key within 3 seconds...\r\n");
    fflush(stdout);
    {
        struct pollfd pfd;
        pfd.fd = 0;
        pfd.events = POLLIN;
        pfd.revents = 0;
        rc = poll(&pfd, 1, 3000);
        if (rc > 0) {
            char buf[64];
            int n = read(0, buf, sizeof(buf));
            PASS("poll returned %d, read %d bytes", rc, n);
            printf("  bytes:");
            {
                int i;
                for (i = 0; i < n && i < 16; i++)
                    printf(" 0x%02x", (unsigned char)buf[i]);
            }
            printf("\r\n");
        } else if (rc == 0) {
            PASS("poll timeout (no keypress, but poll() works on stdin)");
        } else {
            FAIL("poll error: rc=%d errno=%d (%s)", rc, errno, strerror(errno));
        }
    }

    /* --- Test 7: SIGWINCH --- */
    printf("\r\n--- SIGWINCH ---\r\n");
    {
        struct sigaction sa;
        memset(&sa, 0, sizeof(sa));
        sa.sa_handler = sigwinch_handler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_RESTART;
        rc = sigaction(SIGWINCH, &sa, NULL);
        if (rc == 0)
            PASS("sigaction(SIGWINCH) installed");
        else
            FAIL("sigaction(SIGWINCH): errno=%d", errno);

        /* Self-signal to test delivery */
        got_sigwinch = 0;
        kill(getpid(), SIGWINCH);
        usleep(10000); /* 10ms for signal delivery */
        if (got_sigwinch)
            PASS("SIGWINCH delivered and caught");
        else
            FAIL("SIGWINCH not caught");
    }

    /* --- Restore terminal --- */
    tcsetattr(0, TCSANOW, &orig);

    printf("\r\n=== Results: %d passed, %d failed ===\n", pass_count, fail_count);
    return fail_count > 0 ? 1 : 0;
}
