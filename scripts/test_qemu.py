#!/usr/bin/env python3
"""
Dionnex Monolithic Kernel - Automated QEMU Headless Integration Test Suite
Tests Multiboot boot, SMP AP wakeup, PCI/PCIe, HPET, AHCI+FAT32, NVMe,
USB UHCI/xHCI, Ring 3 ELF init, .ko insmod/rmmod, Rust ABI, /dev, and ISO boot.
"""

import os
import select
import struct
import subprocess
import sys
import tempfile
import time


def build_fat32_image(path: str, filename_83: bytes, content: bytes) -> None:
    """Create a valid minimal FAT32 disk image with one root directory file."""
    total_sectors = 66592
    reserved_sectors = 32
    num_fats = 2
    fat_size_32 = 512
    spc = 1
    root_cluster = 2
    data_start = reserved_sectors + num_fats * fat_size_32

    with open(path, "wb") as f:
        f.truncate(total_sectors * 512)

        # Boot Sector (BPB)
        bpb = bytearray(512)
        bpb[0:3] = b"\xEB\x58\x90"
        bpb[3:11] = b"DIONNEX "
        struct.pack_into("<HBHBHHBHHHII", bpb, 11,
                         512, spc, reserved_sectors, num_fats,
                         0, 0, 0xF8, 0, 63, 255, 0, total_sectors)
        struct.pack_into("<IHHIHH", bpb, 36,
                         fat_size_32, 0, 0, root_cluster, 1, 6)
        bpb[64] = 0x80
        bpb[66] = 0x29
        struct.pack_into("<I", bpb, 67, 0x20260001)
        bpb[71:82] = b"DIONNEX_FS "
        bpb[82:90] = b"FAT32   "
        bpb[510] = 0x55
        bpb[511] = 0xAA
        f.seek(0)
        f.write(bpb)

        # FAT tables
        fat_sec = bytearray(512)
        struct.pack_into("<IIII", fat_sec, 0,
                         0x0FFFFFF8, 0x0FFFFFFF, 0x0FFFFFFF, 0x0FFFFFFF)
        for fat_idx in range(num_fats):
            f.seek((reserved_sectors + fat_idx * fat_size_32) * 512)
            f.write(fat_sec)

        # Root directory (cluster 2)
        dir_sec = bytearray(512)
        assert len(filename_83) == 11
        dir_sec[0:11] = filename_83
        dir_sec[11] = 0x20  # Archive
        struct.pack_into("<HI", dir_sec, 26, 3, len(content))
        f.seek(data_start * 512)
        f.write(dir_sec)

        # File data (cluster 3)
        file_sec = bytearray(512)
        file_sec[:len(content)] = content
        f.seek((data_start + 1) * 512)
        f.write(file_sec)


def build_blank_image(path: str, mb: int = 8) -> None:
    with open(path, "wb") as f:
        f.truncate(mb * 1024 * 1024)


def run_qemu_session(qemu_args: list[str], commands: list[str], timeout_s: float = 25.0) -> str:
    proc = subprocess.Popen(
        qemu_args,
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        bufsize=0,
    )

    output_chunks: list[str] = []
    cmd_idx = 0
    buf = ""
    start = time.monotonic()
    fd = proc.stdout.fileno()

    try:
        while time.monotonic() - start < timeout_s:
            rlist, _, _ = select.select([fd], [], [], 0.05)
            if rlist:
                raw = os.read(fd, 4096)
                if not raw:
                    break
                chunk = raw.decode("utf-8", errors="replace")
                output_chunks.append(chunk)
                buf += chunk
                while cmd_idx < len(commands) and "dionnex> " in buf:
                    idx = buf.index("dionnex> ") + len("dionnex> ")
                    buf = buf[idx:]
                    cmd = commands[cmd_idx]
                    cmd_idx += 1
                    proc.stdin.write((cmd + "\n").encode("utf-8"))
                    proc.stdin.flush()
            elif proc.poll() is not None:
                break
    finally:
        if proc.poll() is None:
            proc.kill()
        proc.wait(timeout=5)

    return "".join(output_chunks)


def assert_contains(haystack: str, needle: str, label: str) -> None:
    if needle not in haystack:
        print(f"[FAIL] {label}: expected substring '{needle}' not found!")
        print("--- Captured QEMU output (tail) ---")
        print(haystack[-2500:])
        sys.exit(1)
    print(f"[PASS] {label}")


def main() -> int:
    with tempfile.TemporaryDirectory() as tmp:
        ahci_img = os.path.join(tmp, "ahci_fat32.img")
        nvme_img = os.path.join(tmp, "nvme_disk.img")
        fat_msg = b"Dionnex FAT32 over AHCI DMA works!"
        build_fat32_image(ahci_img, b"DIONNEX TXT", fat_msg)
        build_blank_image(nvme_img, mb=8)

        print("=== Phase A: Full Hardware + SMP + FAT32 + Drivers QEMU Test ===")
        full_args = [
            "qemu-system-i386",
            "-kernel", "dionnex.bin",
            "-m", "256M",
            "-smp", "2",
            "-nographic",
            "-no-reboot",
            "-device", "ahci,id=ahci",
            "-drive", f"id=d0,file={ahci_img},if=none,format=raw",
            "-device", "ide-hd,drive=d0,bus=ahci.0",
            "-drive", f"id=n0,file={nvme_img},if=none,format=raw",
            "-device", "nvme,serial=DIONNEX01,drive=n0",
            "-device", "piix3-usb-uhci",
            "-device", "qemu-xhci",
        ]
        commands = [
            "smp",
            "hpet",
            "pcie",
            "ahci",
            "nvme",
            "usb",
            "fatls",
            "fatcat DIONNEX.TXT",
            "ls /dev",
            "selftest",
            "rusttest",
            "insmod counter.ko",
            "modinfo counter",
            "rmmod counter",
            "shutdown",
        ]
        out = run_qemu_session(full_args, commands, timeout_s=30.0)

        assert_contains(out, "Rust: dionnex-rust 0.1.0", "Rust no_std early init")
        assert_contains(out, "2/2 CPU cores brought online", "SMP INIT-SIPI-SIPI AP wakeup")
        assert_contains(out, "HPET: Active", "HPET ACPI timer")
        assert_contains(out, "AHCI: Port 0 SATA drive", "AHCI SATA DMA IDENTIFY")
        assert_contains(out, "DIONNEX01", "NVMe PCIe controller & IDENTIFY")
        assert_contains(out, "USB: UHCI", "USB UHCI probe")
        assert_contains(out, "USB: xHCI", "USB xHCI probe")
        assert_contains(out, "FAT32: Mounted volume", "FAT32 mount over AHCI")
        assert_contains(out, "DIONNEX.TXT", "FAT32 directory listing (fatls)")
        assert_contains(out, fat_msg.decode(), "FAT32 file read (fatcat)")
        assert_contains(out, "null", "Devfs /dev/null")
        assert_contains(out, "klog", "Devfs /dev/klog")
        assert_contains(out, "Process 'init' (pid 1) completed with exit code 0", "Ring 3 init ELF execution")
        assert_contains(out, "=== SELFTEST SUMMARY: 0 failure(s) ===", "Kernel in-memory selftest suite")
        assert_contains(out, "CRC32='123456789' -> 0xcbf43926", "Rust CRC32 FFI test")
        assert_contains(out, "insmod: 'counter' loaded", "Relocatable .ko insmod")
        assert_contains(out, "rmmod: 'counter' unloaded", "Relocatable .ko rmmod")

        if os.path.exists("dionnex.iso"):
            print("=== Phase B: Bootable GRUB ISO Verification ===")
            iso_args = [
                "qemu-system-i386",
                "-cdrom", "dionnex.iso",
                "-m", "256M",
                "-nographic",
                "-no-reboot",
            ]
            iso_out = run_qemu_session(iso_args, ["uptime", "shutdown"], timeout_s=25.0)
            assert_contains(iso_out, "All subsystems initialized", "GRUB ISO Multiboot boot")
            assert_contains(iso_out, "System Uptime:", "Interactive shell on ISO boot")

    print("=== ALL QEMU INTEGRATION TESTS PASSED ===")
    return 0


if __name__ == "__main__":
    sys.exit(main())
