/* ============================================================================
 * Dionnex Monolithic Kernel - Host-Side C + Rust FFI Unit Tests
 * License: GNU GPL v2 (GPL-2.0-only)
 * Author: Ihosvanni Dionner Gonzalez
 * ============================================================================
 */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Rust no_std FFI symbols from libdionnex_rust.a */
int32_t dionnex_rust_init(void);
int32_t dionnex_rust_is_initialized(void);
const char *dionnex_rust_version(void);
uint32_t dionnex_rust_crc32(const uint8_t *ptr, uint32_t len);
int32_t dionnex_rust_acpi_checksum_ok(const uint8_t *ptr, uint32_t len);
int32_t dionnex_rust_validate_elf32(const uint8_t *ptr, uint32_t len);
const char *dionnex_rust_pci_class_name(uint8_t class_code, uint8_t subclass, uint8_t prog_if);
int32_t dionnex_rust_selftest(void);

typedef struct dionnex_rust_ringbuf {
    uint8_t data[256];
    uint32_t head;
    uint32_t tail;
    uint32_t count;
} dionnex_rust_ringbuf_t;

void dionnex_rust_ring_init(dionnex_rust_ringbuf_t *rb);
int32_t dionnex_rust_ring_push(dionnex_rust_ringbuf_t *rb, uint8_t byte);
int32_t dionnex_rust_ring_pop(dionnex_rust_ringbuf_t *rb, uint8_t *out);

void dionnex_rust_panic_hook(const uint8_t *msg, uint32_t len) {
    (void)msg;
    (void)len;
    abort();
}

void rust_eh_personality(void) {
}

static uint8_t *read_file_bytes(const char *path, uint32_t *out_len) {
    FILE *f = fopen(path, "rb");
    assert(f != NULL);
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    assert(sz > 0);
    fseek(f, 0, SEEK_SET);
    uint8_t *buf = (uint8_t *)malloc((size_t)sz);
    assert(buf != NULL);
    size_t rd = fread(buf, 1, (size_t)sz, f);
    assert(rd == (size_t)sz);
    fclose(f);
    *out_len = (uint32_t)sz;
    return buf;
}

static int user_addr_overflow_or_kernel(uint32_t addr, uint32_t size) {
    if (size == 0) return 0;
    if (addr < 0x08048000U) return 0;
    if (addr > 0xC0000000U) return 0;
    if (addr + size < addr) return 0;
    if (addr + size > 0xC0000000U) return 0;
    return 1;
}

int main(void) {
    assert(dionnex_rust_init() == 0);
    assert(dionnex_rust_is_initialized() == 1);
    assert(strstr(dionnex_rust_version(), "dionnex-rust") != NULL);
    assert(dionnex_rust_selftest() == 0);

    /* CRC32 IEEE standard vector */
    assert(dionnex_rust_crc32((const uint8_t *)"123456789", 9) == 0xCBF43926U);

    /* Real built ELF32 user binary & .ko module validation via Rust */
    uint32_t init_len = 0, ko_len = 0;
    uint8_t *init_elf = read_file_bytes("user/init.elf", &init_len);
    uint8_t *hello_ko = read_file_bytes("modules/hello.ko", &ko_len);
    assert(dionnex_rust_validate_elf32(init_elf, init_len) == 1);
    assert(dionnex_rust_validate_elf32(hello_ko, ko_len) == 1);
    init_elf[0] = 0x00;
    assert(dionnex_rust_validate_elf32(init_elf, init_len) == 0);
    free(init_elf);
    free(hello_ko);

    /* PCI class decoding */
    assert(strcmp(dionnex_rust_pci_class_name(0x01, 0x08, 0x02), "NVMe Storage Controller") == 0);
    assert(strcmp(dionnex_rust_pci_class_name(0x0C, 0x03, 0x30), "USB xHCI Controller") == 0);

    /* Ring buffer round-trip */
    dionnex_rust_ringbuf_t rb;
    dionnex_rust_ring_init(&rb);
    assert(dionnex_rust_ring_push(&rb, 0x42) == 0);
    uint8_t val = 0;
    assert(dionnex_rust_ring_pop(&rb, &val) == 0);
    assert(val == 0x42);
    assert(dionnex_rust_ring_pop(&rb, &val) == -1);

    /* User-space bounds validator */
    assert(user_addr_overflow_or_kernel(0x08048000U, 4096U) == 1);
    assert(user_addr_overflow_or_kernel(0x00100000U, 64U) == 0);
    assert(user_addr_overflow_or_kernel(0xBFFFF000U, 0x2000U) == 0);
    assert(user_addr_overflow_or_kernel(0xFFFFFFFFU, 16U) == 0);

    printf("ALL HOST C + RUST FFI UNIT TESTS PASSED\n");
    return 0;
}
