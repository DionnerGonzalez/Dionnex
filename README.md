# Kernel monolítico modular Dionnex en proceso de creación

**Dionnex** es un kernel monolítico modular de 32 bits para arquitectura x86 (`i686`, Multiboot v1) escrito en **C**, **NASM** y **Rust (`no_std`)**, licenciado bajo **GNU General Public License v2 (GPLv2)**.

Autor original: **Ihosvanni Dionner Gonzalez**.

---

## Arquitectura General

Dionnex implementa un diseño **monolítico modular**: el núcleo central ejecuta en **Ring 0** con soporte para carga y descarga dinámica de módulos binarios relocatables `ELF32` (`ET_REL`, extensión `.ko`) en tiempo de ejecución, aislamiento real de procesos en **Ring 3** mediante tablas de páginas independientes por proceso y llamadas al sistema vía `int 0x80`.

```
+-------------------------------------------------------------------+
|                   ESPACIO DE USUARIO (Ring 3)                     |
|   init.elf / shell_user.elf  --->  int 0x80 (Syscall ABI)         |
+-------------------------------------------------------------------+
|               KERNEL MONOLÍTICO MODULAR (Ring 0)                  |
|                                                                   |
|  [Core C]       PMM/VMM/Heap/DMA | VFS (RAMFS, ProcFS, DevFS,     |
|                 Pipe IPC, FAT32) | Scheduler + Ring 3 ELF Loader  |
|                 SMP (INIT-SIPI)  | Module Loader (.ko ET_REL)     |
|                 Spinlocks/Mutex  | Kernel Shell & Selftest Suite  |
|                                                                   |
|  [Rust no_std]  libdionnex_rust.a (i586 ABI, sin SSE ni libc)     |
|                 CRC32 IEEE | Verificador ELF32 | Checksums ACPI   |
|                 Decodificador PCI | Ring Buffer C-compatible      |
|                                                                   |
|  [NASM]         Multiboot Entry | Trampolín SMP 16->32 bits       |
|                 GDT/IDT/TSS Flush | Context Switch | int 0x80     |
|                                                                   |
|  [Drivers]      VGA | Serial COM1 | PS/2 Teclado (0xE0) & Mouse   |
|                 PIC/APIC/IOAPIC | PIT & HPET | ACPI (RSDP/MCFG)   |
|                 PCI / PCIe ECAM | ATA PIO | SATA AHCI | NVMe      |
|                 USB (UHCI, OHCI, EHCI, xHCI) | VESA VBE | RTC     |
+-------------------------------------------------------------------+
```

---

## Subsistemas Implementados

### 1. Gestión de Memoria (`mm/`)
- **PMM (`mm/pmm.c`)**: Mapa de bits físico inicializado desde el mapa de memoria Multiboot (`mmap`), preservando regiones reservadas y la memoria baja (`< 1 MiB`), incluyendo el trampolín SMP en `0x8000`. Soporte para asignación de páginas individuales y bloques contiguos (`pmm_alloc_pages`, `pmm_alloc_low_page`).
- **VMM (`mm/vmm.c`)**: Paginación x86 de 32 bits con mapeo de kernel, `vmm_ioremap` dinámico para regiones MMIO (`LAPIC`, `IOAPIC`, `HPET`, `AHCI`, `NVMe`, `xHCI`, `ECAM`), clonado/aislamiento de directorios de páginas por proceso (`vmm_create_address_space`, `vmm_destroy_address_space`) y validación estricta de punteros de usuario (`vmm_validate_user_range`).
- **Kernel Heap (`mm/heap.c`)**: Asignador *first-fit* con coalescencia, `kmalloc`, `kfree`, `krealloc`, `kcalloc`, `kmalloc_aligned` y validación de integridad (`0xDEADBEEF`).
- **DMA Coherente (`mm/dma.c`)**: Asignación de buffers físicos alineados a página y mapeados linealmente para controladores AHCI, NVMe y USB (`dma_alloc_coherent`, `dma_free_coherent`).

### 2. Procesos, Multitarea, SMP y Ring 3 (`kernel/`, `sched/`, `user/`)
- **SMP Real (`boot/ap_trampoline.asm`, `kernel/smp.c`)**: Detección mediante ACPI `MADT` / tabla `MP`, copia del trampolín real-mode a protegido en `0x8000` y arranque de Application Processors (APs) mediante secuencia `INIT - SIPI - SIPI`.
- **Sincronización (`kernel/sync.c`)**: Spinlocks atómicos (`xchgl`) con `spin_lock_irqsave`, mutexes y semáforos.
- **Ring 3 y Syscalls (`kernel/process.c`, `kernel/tss.c`, `kernel/syscall.c`)**: Cambio de contexto vía `TSS` (`SS0:ESP0`), entrada a Ring 3 mediante `iret`, cargador `ELF32` (`PT_LOAD`) con validación de límites de usuario en `int 0x80`. Incluye binarios de usuario embebidos (`user/init.elf`, `user/shell_user.elf`).

### 3. Módulos Dinámicos del Kernel (`kernel/module_loader.c`, `modules/`)
- Carga y descarga en caliente (`insmod` / `rmmod` / `modinfo` / `lsmod`) de módulos binarios `ELF32` `ET_REL` (`.ko`).
- Resolución de símbolos exportados (`kernel/module_symbols.c`), aplicación de relocaciones `R_386_32` y `R_386_PC32`, extracción de metadatos `.modinfo` y ejecución de `module_init` / `module_cleanup`.
- Módulos incluidos y probados: `hello.ko`, `counter.ko`, `sysinfo_mod.ko`.

### 4. Rust `no_std` Integrado (`rust/`, `include/dionnex_rust.h`)
- Crate `dionnex_rust` compilado como biblioteca estática freestanding (`i586-unknown-linux-gnu`, `panic = "abort"`, sin instrucciones SSE/MMX ni `libc`) y enlazado directamente en `dionnex.bin`.
- Expone ABI C verificada para CRC-32 IEEE, validación de checksums ACPI, validación de cabeceras `ELF32` con comprobación de desbordamientos, decodificación de clases PCI y buffers circulares seguros.

### 5. Sistemas de Archivos (`fs/`)
- **VFS (`fs/vfs.c`)**: Puntos de montaje jerárquicos (`/ram`, `/proc`, `/dev`, `/fat`).
- **RAMFS (`fs/ramfs.c`)**, **ProcFS (`fs/procfs.c`)** y **Pipe IPC (`fs/pipe.c`)**.
- **DevFS (`fs/devfs.c`)**: Dispositivos de caracteres `/dev/null`, `/dev/zero`, `/dev/tty`, `/dev/klog`.
- **FAT32 (`fs/fat32.c`)**: Parseo de BPB/MBR, recorrido de cadena de clústeres, listado raíz (`fatls`) y lectura de archivos (`fatcat`) sobre AHCI, NVMe o ATA PIO.

### 6. Drivers de Hardware (`drivers/`)
- **Buses y Temporizadores**: `PCI`, `PCIe` (`MCFG` ECAM + capacidad `0x10`), `ACPI`, `APIC`/`IOAPIC`, `HPET`, `PIT`, `RTC`.
- **Almacenamiento**: `ATA PIO`, `SATA AHCI` (FIS H2D, listas/tablas de comandos DMA, `IDENTIFY`, `READ DMA EXT`), `NVMe` (colas Admin y I/O SQ/CQ, `IDENTIFY Controller/NS1`, `NVM READ`).
- **USB**: Detección e inicialización base de controladores `UHCI`, `OHCI`, `EHCI` y `xHCI`.
- **Entrada y Consola**: VGA modo texto, VESA VBE, puerto serial COM1, teclado PS/2 extendido (`0xE0`, Ctrl/Alt) y ratón PS/2 (`IRQ12`).

---

## Procedencia de Código y Licencias

Todo el código original de Dionnex se distribuye bajo **GNU GPL v2 (`GPL-2.0-only`)**. Las adaptaciones puntuales de definiciones de registros y secuencias de hardware proceden exclusivamente de **Linux (`GPL-2.0-only` / `GPL-2.0-or-later`)** y **FreeBSD (`BSD-2-Clause`)**, conservando atribución explícita en cada archivo y documentadas en:
- `docs/PROVENANCE.md`
- `third_party/README.md`
- `third_party/FREEBSD_LICENSE.txt`

---

## Compilación y Pruebas

### Dependencias (Debian / Ubuntu)
```bash
sudo apt-get install -y gcc-multilib nasm binutils grub-pc-bin grub-common xorriso mtools qemu-system-x86 python3
rustup target add i586-unknown-linux-gnu x86_64-unknown-linux-gnu
```

### Comandos principales
```bash
make            # Compila Rust no_std, userspace Ring 3, módulos .ko, C y NASM -> dionnex.bin
make iso        # Genera imagen ISO arrancable con GRUB -> dionnex.iso
make run        # Ejecuta Dionnex en QEMU con salida serial en stdio
make test-host  # Compila y ejecuta pruebas unitarias C + Rust FFI en host
make test       # Ejecuta build completo, pruebas Rust, pruebas host C y suite QEMU
```

---

## Resultados Verificados (QEMU Headless & Host Tests)

Las siguientes comprobaciones se han ejecutado y superado sobre `dionnex.bin` y `dionnex.iso`:
- **Pruebas unitarias Rust (`cargo test --target x86_64-unknown-linux-gnu`)**: 4/4 pruebas aprobadas.
- **Pruebas unitarias Host C + Rust FFI (`make test-host`)**: verificación de `init.elf`, `hello.ko`, CRC-32 (`0xcbf43926`), decodificación PCI, ring buffer y validación de rangos Ring 3.
- **Suite QEMU (`scripts/test_qemu.py` con `-smp 2`, `-device ahci`, `-device nvme`, `-device piix3-usb-uhci`, `-device qemu-xhci`)**:
  - Arranque Multiboot + inicialización temprana `dionnex-rust 0.1.0`
  - Activación SMP `2/2` núcleos online mediante `INIT-SIPI-SIPI`
  - Detección y lectura en vivo de `HPET`, `PCIe`, `AHCI` (`IDENTIFY` + `READ DMA EXT`), `NVMe` (`DIONNEX01`), `USB UHCI` y `USB xHCI`
  - Montaje automático de volumen **FAT32** sobre AHCI DMA y lectura correcta de archivo (`fatls` / `fatcat`)
  - Dispositivos `/dev/null`, `/dev/zero`, `/dev/tty`, `/dev/klog`
  - Ejecución de `init.elf` en **Ring 3** (`PID 1`, código de salida `0`)
  - Suite interna `selftest` (PMM/VMM/Heap/DMA, spinlocks/semáforos, pipes, devfs, ciclo `.ko` y Rust FFI): **0 fallos**
  - Carga/inspección/descarga en caliente de `counter.ko` (`insmod` / `modinfo` / `rmmod`)
  - Arranque de `dionnex.iso` mediante GRUB (`-cdrom dionnex.iso`) y apagado ACPI limpio (`shutdown`)

---

## Estado Actual y Hoja de Ruta

### Estado Actual
Kernel monolítico modular funcional y verificable en CI (`/.github/workflows/ci.yml`), con arranque Multiboot/ISO, memoria paginada con aislamiento Ring 3, cargador de módulos `.ko`, núcleo compartido en Rust `no_std`, SMP real, almacenamiento AHCI/NVMe con FAT32 en solo lectura, detección USB y suite automatizada de pruebas.

### Hoja de Ruta
1. Soporte completo de escritura FAT32 e I/O asíncrono con interrupciones MSI/IOAPIC para AHCI y NVMe.
2. Enumeración completa de dispositivos USB (HID / Mass Storage) sobre xHCI/UHCI.
3. Pila de red (driver `e1000` / `virtio-net`, ARP/IPv4/ICMP/UDP/TCP) y sockets POSIX.
4. Planificador SMP multi-cola real con balanceo de tareas por núcleo AP.
5. Ampliación progresiva de subsistemas críticos de memoria y parsing en Rust `no_std`.
