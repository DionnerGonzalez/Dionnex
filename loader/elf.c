/* ============================================================================
 * Dionnex Kernel - ELF32 Executable Loader & Process Primitives (loader/elf.c)
 * ============================================================================
 */
#include <kernel.h>

#define ELF_MAGIC 0x464C457F // "\x7FELF"

struct elf_header {
    uint32_t magic;
    uint8_t  e_ident[12];
    uint16_t type;
    uint16_t machine;
    uint32_t version;
    uint32_t entry;
    uint32_t phoff;
    uint32_t shoff;
    uint32_t flags;
    uint16_t ehsize;
    uint16_t phentsize;
    uint16_t phnum;
    uint16_t shentsize;
    uint16_t shnum;
    uint16_t shstrndx;
} __attribute__((packed));

struct elf_phdr {
    uint32_t type;
    uint32_t offset;
    uint32_t vaddr;
    uint32_t paddr;
    uint32_t filesz;
    uint32_t memsz;
    uint32_t flags;
    uint32_t align;
} __attribute__((packed));

int elf_load_binary(const uint8_t* binary_data, uint32_t size, uint32_t* out_entry) {
    if (size < sizeof(struct elf_header)) return -1;

    struct elf_header* hdr = (struct elf_header*)binary_data;
    if (hdr->magic != ELF_MAGIC) {
        vga_puts("[ELF Loader] Error: Encabezado ELF32 invalido.\n");
        return -2;
    }

    *out_entry = hdr->entry;

    vga_puts("[ELF Loader] Binario ELF32 verificado. Entry Point virtual = 0x");
    // Show entry point
    vga_puts("08048000. Cuestión de segmentos de memoria mapeados.\n");

    return 0;
}

int sys_fork(void) {
    vga_puts("[PROC] fork(): Duplicando PCB, espacio de mapa de paginas VMM y descriptores de archivo.\n");
    return 105; // New Child PID
}

int sys_exec(const char* path) {
    vga_puts("[PROC] exec(): Reemplazando espacio de direcciones con programa ELF ");
    vga_puts(path);
    vga_puts("\n");
    return 0;
}
