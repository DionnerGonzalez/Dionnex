/* ============================================================================
 * Dionnex Kernel - POSIX System Call Compatibility Layer (compatibility/posix.c)
 * ============================================================================
 */
#include <kernel/kernel.h>

int posix_open(const char* pathname, int flags) {
    (void)flags;
    const char* content = vfs_read_file(pathname);
    return content ? 3 : -1;
}

int posix_read(int fd, void* buf, size_t count) {
    (void)fd;
    if (!buf) return -1;
    return (int)count;
}

int posix_close(int fd) {
    (void)fd;
    return 0;
}
