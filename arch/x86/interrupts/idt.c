/*
 * Interrupt Descriptor Table (IDT) Setup for x86/x86_64
 */

#include <ABI/kernel/internal/kabi.h>

struct idt_entry {
	u16 offset_low;
	u16 selector;
	u8  ist;
	u8  type_attributes;
	u16 offset_mid;
	u32 offset_high;
	u32 zero;
} __attribute__((packed));

struct idt_ptr {
	u16 limit;
	u64 base;
} __attribute__((packed));

static struct idt_entry idt[256];
static struct idt_ptr idtr;

void idt_set_gate(u8 num, u64 base, u16 sel, u8 flags)
{
	idt[num].offset_low  = (u16)(base & 0xFFFF);
	idt[num].selector    = sel;
	idt[num].ist         = 0;
	idt[num].type_attributes = flags;
	idt[num].offset_mid  = (u16)((base >> 16) & 0xFFFF);
	idt[num].offset_high = (u32)((base >> 32) & 0xFFFFFFFF);
	idt[num].zero        = 0;
}

void idt_init(void)
{
	idtr.limit = sizeof(struct idt_entry) * 256 - 1;
	idtr.base  = (u64)&idt;
	__asm__ __volatile__("lidt %0" : : "m"(idtr));
}
