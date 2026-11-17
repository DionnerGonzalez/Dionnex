/* ============================================================================
 * Dionnex Monolithic Kernel - C <-> Rust ABI Header
 * License: GNU GPL v2 (GPL-2.0-only)
 * Author: Ihosvanni Dionner Gonzalez
 * ============================================================================
 */
#ifndef _DIONNEX_RUST_H_
#define _DIONNEX_RUST_H_

#include <types.h>

#define DIONNEX_RUST_RING_CAP 256U

typedef struct dionnex_rust_ringbuf {
    uint8_t data[DIONNEX_RUST_RING_CAP];
    uint32_t head;
    uint32_t tail;
    uint32_t count;
} dionnex_rust_ringbuf_t;

int32_t dionnex_rust_init(void);
int32_t dionnex_rust_is_initialized(void);
const char *dionnex_rust_version(void);
uint32_t dionnex_rust_crc32(const uint8_t *ptr, uint32_t len);
int32_t dionnex_rust_acpi_checksum_ok(const uint8_t *ptr, uint32_t len);
int32_t dionnex_rust_validate_elf32(const uint8_t *ptr, uint32_t len);
const char *dionnex_rust_pci_class_name(uint8_t class_code, uint8_t subclass, uint8_t prog_if);
void dionnex_rust_ring_init(dionnex_rust_ringbuf_t *rb);
int32_t dionnex_rust_ring_push(dionnex_rust_ringbuf_t *rb, uint8_t byte);
int32_t dionnex_rust_ring_pop(dionnex_rust_ringbuf_t *rb, uint8_t *out);
int32_t dionnex_rust_selftest(void);

#endif /* _DIONNEX_RUST_H_ */
