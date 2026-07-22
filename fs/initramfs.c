/* ============================================================================
 * Dionnex Kernel - InitRAMFS RAM Disk Pre-loaded Filesystem (fs/initramfs.c)
 * ============================================================================
 */
#include <kernel.h>

extern int vfs_add_file(const char* name, const char* content, uint32_t size);

static const char* welcome_msg = "Bienvenido a Dionnex Kernel OS Real x86!\nSistema operativo x86 completo en modo protegido.";
static const char* version_msg = "Dionnex OS Version 1.0.0-RELEASE (GCC 11.2.0, NASM 2.15, Linker i386)";
static const char* meminfo_msg = "MemTotal: 131072 kB\nMemFree: 122880 kB\nBuffers: 4096 kB\nCached: 4096 kB";

void initramfs_init(void) {
    vfs_add_file("welcome.txt", welcome_msg, 84);
    vfs_add_file("proc_version", version_msg, 68);
    vfs_add_file("proc_meminfo", meminfo_msg, 72);

    vga_puts("[INITRAMFS] RAMDisk cargado con archivos iniciales en /initramfs.\n");
}
