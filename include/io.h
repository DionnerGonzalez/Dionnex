/* ============================================================================
 * Dionnex Monolithic Kernel - Hardware Port I/O Operations
 * ============================================================================
 */
#ifndef _DIONNEX_IO_H_
#define _DIONNEX_IO_H_

#include <types.h>

static inline void outb(uint16_t port, uint8_t val) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outw(uint16_t port, uint16_t val) {
    asm volatile ("outw %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint16_t inw(uint16_t port) {
    uint16_t ret;
    asm volatile ("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outl(uint16_t port, uint32_t val) {
    asm volatile ("outl %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint32_t inl(uint16_t port) {
    uint32_t ret;
    asm volatile ("inl %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void io_wait(void) {
    outb(0x80, 0);
}

#define MSR_IA32_APICBASE 0x1B
#define MSR_IA32_EFER     0xC0000080

static inline uint64_t rdmsr(uint32_t msr) {
    uint32_t low, high;
    asm volatile ("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));
    return ((uint64_t)high << 32) | low;
}

static inline void wrmsr(uint32_t msr, uint64_t val) {
    uint32_t low = (uint32_t)(val & 0xFFFFFFFFU);
    uint32_t high = (uint32_t)(val >> 32);
    asm volatile ("wrmsr" : : "c"(msr), "a"(low), "d"(high));
}

static inline uint32_t read_cr0(void) {
    uint32_t val;
    asm volatile ("mov %%cr0, %0" : "=r"(val));
    return val;
}

static inline void write_cr0(uint32_t val) {
    asm volatile ("mov %0, %%cr0" : : "r"(val) : "memory");
}

static inline uint32_t read_cr2(void) {
    uint32_t val;
    asm volatile ("mov %%cr2, %0" : "=r"(val));
    return val;
}

static inline uint32_t read_cr3(void) {
    uint32_t val;
    asm volatile ("mov %%cr3, %0" : "=r"(val));
    return val;
}

static inline void write_cr3(uint32_t val) {
    asm volatile ("mov %0, %%cr3" : : "r"(val) : "memory");
}

static inline uint32_t read_eflags(void) {
    uint32_t flags;
    asm volatile ("pushfl; popl %0" : "=r"(flags));
    return flags;
}

static inline void cpu_cli(void) {
    asm volatile ("cli" ::: "memory");
}

static inline void cpu_sti(void) {
    asm volatile ("sti" ::: "memory");
}

static inline void cpu_pause(void) {
    asm volatile ("pause" ::: "memory");
}

static inline uint64_t rdtsc(void) {
    uint32_t low, high;
    asm volatile ("rdtsc" : "=a"(low), "=d"(high));
    return ((uint64_t)high << 32) | low;
}

#endif /* _DIONNEX_IO_H_ */
