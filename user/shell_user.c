/* ============================================================================
 * Dionnex Ring 3 Userspace Shell Process
 * Author: Ihosvanni Dionner Gonzalez
 * ============================================================================
 */
typedef unsigned int uint32_t;
typedef int int32_t;

#define SYS_EXIT      1
#define SYS_READ      3
#define SYS_WRITE     4
#define SYS_GETPID    20
#define SYS_NANOSLEEP 162

static inline int32_t syscall1(int num, int32_t a) {
    int32_t ret;
    asm volatile("int $0x80" : "=a"(ret) : "a"(num), "b"(a));
    return ret;
}

static inline int32_t syscall3(int num, int32_t a, int32_t b, int32_t c) {
    int32_t ret;
    asm volatile("int $0x80" : "=a"(ret) : "a"(num), "b"(a), "c"(b), "d"(c));
    return ret;
}

static void print_str(const char *s) {
    uint32_t len = 0;
    while (s[len]) len++;
    syscall3(SYS_WRITE, 1, (int32_t)s, len);
}

static void print_num(int num) {
    char buf[16];
    int i = 0;
    if (num == 0) {
        print_str("0");
        return;
    }
    if (num < 0) {
        print_str("-");
        num = -num;
    }
    while (num > 0) {
        buf[i++] = '0' + (num % 10);
        num /= 10;
    }
    for (int j = i - 1; j >= 0; j--) {
        char c = buf[j];
        syscall3(SYS_WRITE, 1, (int32_t)&c, 1);
    }
}

static int str_equals(const char *s1, const char *s2) {
    while (*s1 && *s2) {
        if (*s1 != *s2) return 0;
        s1++; s2++;
    }
    return (*s1 == *s2);
}

static int str_starts_with(const char *str, const char *prefix) {
    while (*prefix) {
        if (*str != *prefix) return 0;
        str++; prefix++;
    }
    return 1;
}

void _start(void) {
    print_str("\n--- Dionnex User Shell v0.1 (Ring 3) ---\n");

    char cmd[128];
    int pos = 0;

    while (1) {
        print_str("user> ");
        pos = 0;

        while (pos < 127) {
            char c;
            int n = syscall3(SYS_READ, 0, (int32_t)&c, 1);
            if (n <= 0) continue;

            if (c == '\r' || c == '\n') {
                cmd[pos] = '\0';
                print_str("\n");
                break;
            } else if (c == '\b') {
                if (pos > 0) {
                    pos--;
                    print_str("\b \b");
                }
            } else {
                cmd[pos++] = c;
                syscall3(SYS_WRITE, 1, (int32_t)&c, 1); // echo
            }
        }

        if (pos == 0) continue;

        if (str_equals(cmd, "exit")) {
            print_str("Exiting userspace shell...\n");
            syscall1(SYS_EXIT, 0);
        } else if (str_equals(cmd, "getpid")) {
            print_str("User Process PID: ");
            print_num(syscall1(SYS_GETPID, 0));
            print_str("\n");
        } else if (str_starts_with(cmd, "write ")) {
            print_str(cmd + 6);
            print_str("\n");
        } else if (str_starts_with(cmd, "sleep ")) {
            print_str("Sleeping...\n");
            syscall1(SYS_NANOSLEEP, 500);
            print_str("Done.\n");
        } else {
            print_str("Unknown user command: ");
            print_str(cmd);
            print_str("\n");
        }
    }

    syscall1(SYS_EXIT, 0);
}
