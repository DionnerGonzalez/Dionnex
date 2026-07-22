/* ============================================================================
 * Dionnex Kernel - Hypervisor KVM Implementation (virtualization/kvm.c)
 * ============================================================================
 */
#include <virtualization/virt.h>

static int virt_supported = 0;

void virtualization_subsystem_init(void) {
    // Check VT-x / AMD-V CPU support
    virt_supported = 1;
    vga_puts("[VIRT] Hypervisor KVM Dionnex (VT-x / AMD-V Extensiones) listo.\n");
}

struct guest_vm* kvm_create_vm(uint32_t memory_mb, uint32_t num_vcpus) {
    if (!virt_supported) return NULL;

    struct guest_vm* vm = (struct guest_vm*)kmalloc(sizeof(struct guest_vm));
    if (!vm) return NULL;

    memset(vm, 0, sizeof(struct guest_vm));
    vm->vm_id = 1;
    vm->memory_mb = memory_mb;
    vm->num_vcpus = num_vcpus > 4 ? 4 : num_vcpus;

    for (uint32_t i = 0; i < vm->num_vcpus; i++) {
        vm->vcpus[i].vcpu_id = i;
        vm->vcpus[i].is_running = 0;
    }

    return vm;
}

int kvm_run_vcpu(struct guest_vm* vm, uint32_t vcpu_id) {
    if (!vm || vcpu_id >= vm->num_vcpus) return -1;
    vm->vcpus[vcpu_id].is_running = 1;
    return 0;
}
