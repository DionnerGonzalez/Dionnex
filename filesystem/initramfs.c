/* ============================================================================
 * Dionnex Kernel - Initial RAM Disk Mount (filesystem/initramfs.c)
 * ============================================================================
 */
#include <kernel/kernel.h>

void initramfs_init(void) {
    const char* readme = 
        "Welcome to Dionnex Microkernel Architecture!\n"
        "This system features physical bitmap PMM, 2-level Paging VMM,\n"
        "Completely Fair Scheduler (CFS), and PS/2 Driver subsystems.\n";

    const char* sysinfo = 
        "OS Name   : Dionnex Kernel\n"
        "Arch      : x86 / x86_64\n"
        "Status    : Real Code Active\n";

    const char* hello_elf = "\x7F" "ELF" "\x01\x01\x01\x00" "Dummy ELF Binary Executable";

    vfs_add_file("README.txt", readme, strlen(readme));
    vfs_add_file("sysinfo.cfg", sysinfo, strlen(sysinfo));
    vfs_add_file("hello.elf", hello_elf, strlen(hello_elf));
}
