# Dionnex Kernel Source Provenance & License Compliance

All original code in Dionnex is licensed under **GNU General Public License v2 (GPLv2)**.
Where hardware register definitions, driver state machines, or low-level initialization
sequences have been adapted from established open-source operating systems, Dionnex strictly
restricts reuse to GPLv2-compatible licenses (`GPL-2.0-only`, `GPL-2.0-or-later`,
`BSD-2-Clause`, `MIT`, `Apache-2.0`) and preserves original copyright and attribution notices.

## Adapted Open-Source Components

| Dionnex File(s) | Upstream Project & Path | Upstream License | Adaptation Summary |
|---|---|---|---|
| `drivers/apic.c`, `drivers/apic.h` | Linux kernel: `arch/x86/kernel/apic/` | `GPL-2.0-only` | Local APIC / IOAPIC MMIO accessors, SVR/TPR init, INIT-SIPI-SIPI IPI sequences |
| `kernel/module_loader.c`, `kernel/module_loader.h`, `kernel/module_symbols.c` | Linux kernel: `kernel/module/` | `GPL-2.0-or-later` | ELF32 `ET_REL` section parsing, symbol resolution, `R_386_32`/`R_386_PC32` relocation, `.modinfo` metadata |
| `drivers/ahci.c`, `drivers/ahci.h` | Linux `drivers/ata/ahci.c` & FreeBSD `sys/dev/ahci/` | `GPL-2.0-or-later` / `BSD-2-Clause` | AHCI 1.0 HBA ABAR reset (`GHC.AE`), `PI` port probe, H2D FIS, Command List/Table DMA, `IDENTIFY DEVICE` & `READ DMA EXT` |
| `drivers/nvme.c`, `drivers/nvme.h` | Linux `drivers/nvme/host/pci.c` & FreeBSD `sys/dev/nvme/` | `GPL-2.0-only` / `BSD-2-Clause` | NVMe `CAP`/`CC`/`CSTS` reset, Admin SQ/CQ ring setup, `IDENTIFY` Controller/Namespace, I/O queue creation & sector `READ` |
| `drivers/usb/uhci.c`, `ohci.c`, `ehci.c`, `xhci.c`, `usb.h` | Linux `drivers/usb/host/` & FreeBSD `sys/dev/usb/controller/` | `GPL-2.0-only` / `BSD-2-Clause` | UHCI/OHCI/EHCI/xHCI controller discovery, host reset, DCBAA/frame list DMA setup, root-hub port status inspection |
| `rust/` (`dionnex_rust`) | Original Dionnex crate (`core` stdlib via rustup) | `GPL-2.0-only` (`core`: `MIT OR Apache-2.0`) | Freestanding `no_std` i586 staticlib: CRC32, ACPI checksums, ELF32 bounds verifier, PCI class decoder, ring buffer |

## Verification Rules
- Every adapted file retains upstream attribution in its top comment header.
- Full BSD-2-Clause license text is preserved in `third_party/FREEBSD_LICENSE.txt`.
- No proprietary, GPLv3-only, or incompatible third-party code is included.
