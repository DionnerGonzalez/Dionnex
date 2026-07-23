/* ============================================================================
 * Dionnex Ring 3 Userspace Init Process (PID 1)
 * Author: Ihosvanni Dionner Gonzalez
 * ============================================================================
 */
typedef unsigned int uint32_t;
typedef int int32_t;

#define SYS_EXIT   1
#define SYS_READ   3
#define SYS_WRITE  4
#define SYS_GETPID 20

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

void _start(void) {
    int32_t pid = syscall1(SYS_GETPID, 0);
    (void)pid;

    char msg1[] = "Dionnex init: Hello from userspace! PID=1\n";
    syscall3(SYS_WRITE, 1, (int32_t)msg1, sizeof(msg1) - 1);

    char msg2[] = "Dionnex init: System ready in Ring 3\n";
    syscall3(SYS_WRITE, 1, (int32_t)msg2, sizeof(msg2) - 1);

    while (1) {
        char c;
        int32_t n = syscall3(SYS_READ, 0, (int32_t)&c, 1);
        if (n > 0) {
            syscall3(SYS_WRITE, 1, (int32_t)&c, 1);
        }
    }

    syscall1(SYS_EXIT, 0);
}
