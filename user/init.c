/* ============================================================================
 * Dionnex Ring 3 Userspace Init Process (PID 1)
 * Author: Ihosvanni Dionner Gonzalez
 * ============================================================================
 */
typedef unsigned int uint32_t;
typedef int int32_t;

#define SYS_EXIT         1
#define SYS_WRITE        4
#define SYS_SBRK         12
#define SYS_GETPID       20
#define SYS_UNAME        122

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

static void write_str(const char *s) {
    uint32_t len = 0;
    while (s[len]) len++;
    syscall3(SYS_WRITE, 1, (int32_t)s, (int32_t)len);
}

void _start(void) {
    int32_t pid = syscall1(SYS_GETPID, 0);
    (void)pid;

    write_str("Dionnex init: Hello from Ring 3 userspace! PID=1\n");

    char uname_buf[195];
    if (syscall1(SYS_UNAME, (int32_t)uname_buf) == 0) {
        write_str("Dionnex init: uname verified in Ring 3 (");
        write_str(uname_buf);
        write_str(")\n");
    }

    int32_t heap_ptr = syscall1(SYS_SBRK, 4096);
    if (heap_ptr != -1) {
        volatile char *p = (volatile char*)heap_ptr;
        p[0] = 'O';
        p[1] = 'K';
        write_str("Dionnex init: User sbrk heap allocation OK\n");
    }

    /* Security check: kernel pointer syscall must be rejected (-1) */
    int32_t sec_res = syscall3(SYS_WRITE, 1, 0x00100000, 16);
    if (sec_res == -1) {
        write_str("Dionnex init: Syscall kernel-pointer isolation check PASSED\n");
    }

    syscall1(SYS_EXIT, 0);
    while (1) {}
}
