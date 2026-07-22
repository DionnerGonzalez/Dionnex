/* ============================================================================
 * Dionnex Kernel - Hardware Virtualization & Hypervisor Core (virtualization/virt.h)
 * ============================================================================
 */
#ifndef _DIONNEX_VIRTUALIZATION_VIRT_H
#define _DIONNEX_VIRTUALIZATION_VIRT_H

#include <kernel/kernel.h>

struct vm_cpu_state {
    uint32_t rax, rbx, rcx, rdx;
    uint32_t rsi, rdi, rbp, rsp;
    uint32_t rip, rflags;
    uint32_t cr0, cr3, cr4;
};

struct vcpu {
    uint32_t vcpu_id;
    int is_running;
    struct vm_cpu_state state;
};

struct guest_vm {
    uint32_t vm_id;
    uint32_t memory_mb;
    uint32_t num_vcpus;
    struct vcpu vcpus[4];
};

void virtualization_subsystem_init(void);
struct guest_vm* kvm_create_vm(uint32_t memory_mb, uint32_t num_vcpus);
int kvm_run_vcpu(struct guest_vm* vm, uint32_t vcpu_id);

#endif /* _DIONNEX_VIRTUALIZATION_VIRT_H */
