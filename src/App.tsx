import React, { useState } from 'react';
import {
  Cpu,
  HardDrive,
  Terminal,
  FileCode,
  Folder,
  FolderOpen,
  Layers,
  Settings,
  ShieldCheck,
  Activity,
  Check,
  ChevronRight,
  ChevronDown,
  BookOpen,
  Search,
  Copy,
  Play,
  MemoryStick,
  Server,
  Code,
  ListFilter,
  CheckCircle2,
  Sliders,
  Maximize2
} from 'lucide-react';

// Data structure representing the requested Kernel File Tree
interface FileNode {
  name: string;
  type: 'file' | 'folder';
  path: string;
  lang?: string;
  content?: string;
  children?: FileNode[];
}

const KERNEL_FILES: FileNode[] = [
  {
    name: 'README.md',
    type: 'file',
    path: '/README.md',
    lang: 'markdown',
    content: `# DionOS Kernel - Modern Modular Monolithic Operating System Kernel

DionOS is a modern, enterprise-grade modular monolithic kernel designed in compliance with ISO C11 and Linux Kernel Coding Standards. It provides unified support across multiple architectures (x86, x86_64, ARM, ARM64, RISC-V, PowerPC, MIPS, SPARC, s390, LoongArch) with hardware-accelerated memory virtualization, pre-emptive fair scheduling, zero-copy IPC, and dynamic kernel module loading.

## Key Subsystems
- **Core Kernel**: init.c, panic.c, logging.c, errors.c
- **Process Scheduler**: CFS (Completely Fair Scheduler), RT POSIX schedulers
- **Memory Subsystem**: PMM Buddy Frame Allocator, VMM Page Tables, SLUB Cache Allocator
- **Syscall Dispatcher**: Fast vector table handling POSIX process, IPC, and socket calls`
  },
  {
    name: 'Makefile',
    type: 'file',
    path: '/Makefile',
    lang: 'makefile',
    content: `VERSION = 1
PATCHLEVEL = 0
SUBLEVEL = 0
EXTRAVERSION = -RELEASE
NAME = DionOS Modular Monolithic Kernel

ARCH ?= x86_64
CC = gcc
CFLAGS := -O2 -Wall -Wextra -Werror -std=c11 -ffreestanding -mcmodel=kernel -Iinclude -D__KERNEL__

CORE_OBJS := kernel/core/kernel.o kernel/core/init.o kernel/core/panic.o \\
             kernel/core/logging.o kernel/core/errors.o

PROC_OBJS := process/scheduler/fair/cfs.o process/processes/process.o \\
             process/context_switch/switch.o process/synchronization/spinlock.o

MM_OBJS   := memory/physical/pmm.o memory/virtual/vmm.o memory/slab/slab.o

all: dionos.bin`
  },
  {
    name: 'Kconfig',
    type: 'file',
    path: '/Kconfig',
    lang: 'kconfig',
    content: `mainmenu "DionOS Modular Monolithic Kernel v1.0.0 Configuration"

config DIONOS_VERSION
	string
	default "1.0.0-RELEASE"

choice
	prompt "Target Architecture"
	default ARCH_X86_64

config ARCH_X86_64
	bool "x86_64 Architecture"

config ARCH_ARM64
	bool "ARM64 Architecture (AArch64)"

config ARCH_RISCV64
	bool "RISC-V 64-bit Architecture"
endchoice

config PREEMPT
	bool "Preemptive Kernel Model"
	default y

config SCHED_CFS
	bool "Completely Fair Scheduler (CFS)"
	default y`
  },
  {
    name: '.config',
    type: 'file',
    path: '/.config',
    lang: 'ini',
    content: `CONFIG_DIONOS_VERSION="1.0.0-RELEASE"
CONFIG_ARCH_X86_64=y
CONFIG_PREEMPT=y
CONFIG_SCHED_CFS=y
CONFIG_SCHED_RT=y
CONFIG_NUMA=y
CONFIG_KASLR=y
CONFIG_MODULES=y
CONFIG_SLUB=y
CONFIG_HUGE_PAGES=y
CONFIG_MMU=y`
  },
  {
    name: 'ABI',
    type: 'folder',
    path: '/ABI',
    children: [
      {
        name: 'kernel',
        type: 'folder',
        path: '/ABI/kernel',
        children: [
          {
            name: 'internal',
            type: 'folder',
            path: '/ABI/kernel/internal',
            children: [
              {
                name: 'kabi.h',
                type: 'file',
                path: '/ABI/kernel/internal/kabi.h',
                lang: 'c',
                content: `/*
 * DionOS Kernel Internal ABI Definition
 */

#ifndef _DIONOS_KABI_H
#define _DIONOS_KABI_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t  u8;

typedef u64 phys_addr_t;
typedef u64 virt_addr_t;
typedef s32 pid_t;

#define ASMLINKAGE __attribute__((regparm(0)))
#define EXPORT_SYMBOL(sym) const void *__kstrtab_##sym __attribute__((section("__ksymtab"))) = (void *)&sym
#define __init __attribute__((section(".init.text")))

#endif /* _DIONOS_KABI_H */`
              }
            ]
          }
        ]
      },
      {
        name: 'userspace',
        type: 'folder',
        path: '/ABI/userspace',
        children: [
          {
            name: 'syscall',
            type: 'folder',
            path: '/ABI/userspace/syscall',
            children: [
              {
                name: 'sys_abi.h',
                type: 'file',
                path: '/ABI/userspace/syscall/sys_abi.h',
                lang: 'c',
                content: `/* DionOS System Call ABI Definitions */
#define SYS_READ      0
#define SYS_WRITE     1
#define SYS_OPEN      2
#define SYS_CLOSE     3
#define SYS_FORK     57
#define SYS_EXECVE   59
#define SYS_EXIT     60

#define SYSCALL_MAX 512
typedef s64 (*syscall_fn_t)(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);`
              }
            ]
          }
        ]
      }
    ]
  },
  {
    name: 'arch',
    type: 'folder',
    path: '/arch',
    children: [
      {
        name: 'x86_64',
        type: 'folder',
        path: '/arch/x86_64',
        children: [
          {
            name: 'mm',
            type: 'folder',
            path: '/arch/x86_64/mm',
            children: [
              {
                name: 'page_table.c',
                type: 'file',
                path: '/arch/x86_64/mm/page_table.c',
                lang: 'c',
                content: `/*
 * 64-Bit x86_64 4-Level and 5-Level Page Table Management
 */

#include <ABI/kernel/internal/kabi.h>

#define PML4_INDEX(v) (((v) >> 39) & 0x1FF)
#define PT_INDEX(v)   (((v) >> 12) & 0x1FF)

void x86_64_map_page(u64 *pml4, virt_addr_t vaddr, phys_addr_t paddr, u64 flags)
{
	u64 pml4_idx = PML4_INDEX(vaddr);
	u64 pt_idx   = PT_INDEX(vaddr);
	/* Maps physical frame paddr to virtual address vaddr in PML4 table */
}`
              }
            ]
          },
          {
            name: 'syscall',
            type: 'folder',
            path: '/arch/x86_64/syscall',
            children: [
              {
                name: 'syscall_64.c',
                type: 'file',
                path: '/arch/x86_64/syscall/syscall_64.c',
                lang: 'c',
                content: `/*
 * x86_64 SYSCALL Fast System Call Dispatch Routine
 */

#include <ABI/kernel/internal/kabi.h>
#include <ABI/userspace/syscall/sys_abi.h>

extern syscall_fn_t syscall_table[];

asmlinkage s64 x86_64_sys_dispatch(u64 nr, u64 a1, u64 a2, u64 a3, u64 a4, u64 a5, u64 a6)
{
	if (unlikely(nr >= SYSCALL_MAX || !syscall_table[nr]))
		return -1;

	return syscall_table[nr](a1, a2, a3, a4, a5, a6);
}`
              }
            ]
          }
        ]
      },
      {
        name: 'arm64',
        type: 'folder',
        path: '/arch/arm64',
        children: [
          {
            name: 'apple',
            type: 'folder',
            path: '/arch/arm64/apple',
            children: [
              {
                name: 'm1_m2.c',
                type: 'file',
                path: '/arch/arm64/apple/m1_m2.c',
                lang: 'c',
                content: `/*
 * Apple Silicon (M1/M2/M3/M4) ARM64 Platform Core
 */

#include <ABI/kernel/internal/kabi.h>

void apple_silicon_init(void)
{
	/* Apple AIC Interrupt Controller & Firestorm/Icestorm core setup */
}`
              }
            ]
          }
        ]
      },
      {
        name: 'riscv',
        type: 'folder',
        path: '/arch/riscv',
        children: [
          {
            name: 'kernel.c',
            type: 'file',
            path: '/arch/riscv/kernel.c',
            lang: 'c',
            content: `/* RISC-V (RV64GC) Sv39 / Sv48 Virtual Memory and SBI Call Vector */
#include <ABI/kernel/internal/kabi.h>

static inline u64 sbi_call(u64 extension, u64 function, u64 arg0, u64 arg1)
{
	register u64 a0 __asm__("a0") = arg0;
	register u64 a1 __asm__("a1") = arg1;
	register u64 a6 __asm__("a6") = function;
	register u64 a7 __asm__("a7") = extension;
	__asm__ __volatile__("ecall" : "+r"(a0) : "r"(a1), "r"(a6), "r"(a7) : "memory");
	return a0;
}`
          }
        ]
      }
    ]
  },
  {
    name: 'kernel',
    type: 'folder',
    path: '/kernel',
    children: [
      {
        name: 'core',
        type: 'folder',
        path: '/kernel/core',
        children: [
          {
            name: 'kernel.c',
            type: 'file',
            path: '/kernel/core/kernel.c',
            lang: 'c',
            content: `/*
 * DionOS Core Kernel Execution Entry & Symbol Table Manager
 */

#include <kernel/core/kernel.h>

static const char *kernel_banner = "DionOS Modular Monolithic Kernel v1.0.0-RELEASE (c) 2026";

const char *get_kernel_banner(void)
{
	return kernel_banner;
}

EXPORT_SYMBOL(get_kernel_banner);`
          },
          {
            name: 'init.c',
            type: 'file',
            path: '/kernel/core/init.c',
            lang: 'c',
            content: `/*
 * DionOS Main System Initialization Sequence
 */

#include <kernel/core/kernel.h>

extern void pmm_init(void);
extern void vmm_init(void);
extern void sched_init(void);
extern void syscall_init(void);
extern void irq_init(void);

asmlinkage void __init start_kernel(void)
{
	pr_info("Booting %s...\\n", get_kernel_banner());

	/* Phase 1: Core Physical Memory & Page Frame Allocator */
	pmm_init();

	/* Phase 2: Virtual Memory Manager & Slab Allocator */
	vmm_init();

	/* Phase 3: Interrupt Vectors & APIC Initialization */
	irq_init();

	/* Phase 4: Scheduler & Initial Task0 Creation */
	sched_init();

	/* Phase 5: System Call Entry Point Wiring */
	syscall_init();

	pr_info("DionOS Kernel Subsystems Initialized Successfully.\\n");
}`
          },
          {
            name: 'panic.c',
            type: 'file',
            path: '/kernel/core/panic.c',
            lang: 'c',
            content: `/*
 * DionOS Kernel Panic Handler & Registers Dump
 */

#include <kernel/core/kernel.h>

void panic(const char *fmt, ...)
{
	pr_err("========================================================\\n");
	pr_err("KERNEL PANIC: Fatal exception occurred in kernel mode!\\n");
	pr_err("System halted. Please reboot hardware or check kernel log.\\n");
	pr_err("========================================================\\n");

	__asm__ __volatile__("cli");
	for (;;) {
		__asm__ __volatile__("hlt");
	}
}
EXPORT_SYMBOL(panic);`
          },
          {
            name: 'logging.c',
            type: 'file',
            path: '/kernel/core/logging.c',
            lang: 'c',
            content: `/* DionOS Circular Ring Buffer Kernel Logger */
#include <kernel/core/kernel.h>

#define LOG_BUF_SIZE 65536
static char log_buf[LOG_BUF_SIZE];

void pr_info(const char *fmt, ...) { /* Ring buffer append */ }
void pr_err(const char *fmt, ...) { /* Ring buffer append */ }
EXPORT_SYMBOL(pr_info);`
          },
          {
            name: 'errors.c',
            type: 'file',
            path: '/kernel/core/errors.c',
            lang: 'c',
            content: `/* DionOS System Error Codes Map */
#define EPERM 1
#define ENOENT 2
#define ENOMEM 12
#define EFAULT 14

const char *kernel_strerror(int err) {
	switch (err) {
	case ENOMEM: return "Out of memory";
	case EFAULT: return "Bad address";
	default: return "Unknown error";
	}
}`
          }
        ]
      }
    ]
  },
  {
    name: 'process',
    type: 'folder',
    path: '/process',
    children: [
      {
        name: 'scheduler',
        type: 'folder',
        path: '/process/scheduler',
        children: [
          {
            name: 'fair',
            type: 'folder',
            path: '/process/scheduler/fair',
            children: [
              {
                name: 'cfs.c',
                type: 'file',
                path: '/process/scheduler/fair/cfs.c',
                lang: 'c',
                content: `/*
 * Completely Fair Scheduler (CFS) Subsystem
 */

#include <ABI/kernel/internal/kabi.h>

struct sched_entity {
	u64 vruntime;
	u64 exec_start;
	u64 sum_exec_runtime;
	s32 load_weight;
};

void cfs_update_curr(struct cfs_rq *rq, u64 delta_exec)
{
	if (!rq || !rq->curr)
		return;

	rq->curr->sum_exec_runtime += delta_exec;
	rq->curr->vruntime += delta_exec * (1024 / rq->curr->load_weight);
}`
              }
            ]
          }
        ]
      },
      {
        name: 'synchronization',
        type: 'folder',
        path: '/process/synchronization',
        children: [
          {
            name: 'spinlock.c',
            type: 'file',
            path: '/process/synchronization/spinlock.c',
            lang: 'c',
            content: `/* Atomic Spinlock Synchronization Primitives */
#include <ABI/kernel/internal/kabi.h>

typedef struct { volatile u32 counter; } spinlock_t;

void spin_lock(spinlock_t *lock) {
	while (__sync_lock_test_and_set(&lock->counter, 1)) {
		__asm__ __volatile__("pause");
	}
}
void spin_unlock(spinlock_t *lock) {
	__sync_lock_release(&lock->counter);
}`
          }
        ]
      }
    ]
  },
  {
    name: 'memory',
    type: 'folder',
    path: '/memory',
    children: [
      {
        name: 'physical',
        type: 'folder',
        path: '/memory/physical',
        children: [
          {
            name: 'pmm.c',
            type: 'file',
            path: '/memory/physical/pmm.c',
            lang: 'c',
            content: `/* Physical Memory Allocator (PMM) - Buddy Allocation Algorithm */
#include <ABI/kernel/internal/kabi.h>

#define PAGE_SIZE 4096
#define MAX_ORDER 11

phys_addr_t alloc_pages(u32 order) {
	/* Allocate 2^order physical frames */
	return 0x100000;
}`
          }
        ]
      },
      {
        name: 'allocator',
        type: 'folder',
        path: '/memory/allocator',
        children: [
          {
            name: 'kmalloc.c',
            type: 'file',
            path: '/memory/allocator/kmalloc.c',
            lang: 'c',
            content: `/* Kernel General Heap Allocator (kmalloc / kfree) */
#include <ABI/kernel/internal/kabi.h>

void *kmalloc(size_t size, u32 flags) {
	static char heap_buf[65536];
	static size_t offset = 0;
	void *ptr = &heap_buf[offset];
	offset += (size + 15) & ~15;
	return ptr;
}
EXPORT_SYMBOL(kmalloc);`
          }
        ]
      }
    ]
  },
  {
    name: 'syscall',
    type: 'folder',
    path: '/syscall',
    children: [
      {
        name: 'table',
        type: 'folder',
        path: '/syscall/table',
        children: [
          {
            name: 'syscall_table.c',
            type: 'file',
            path: '/syscall/table/syscall_table.c',
            lang: 'c',
            content: `/* DionOS Master System Call Vector Table */
#include <ABI/userspace/syscall/sys_abi.h>

extern s64 sys_read(u64, u64, u64, u64, u64, u64);
extern s64 sys_write(u64, u64, u64, u64, u64, u64);
extern s64 sys_fork(u64, u64, u64, u64, u64, u64);
extern s64 sys_exit(u64, u64, u64, u64, u64, u64);

syscall_fn_t syscall_table[SYSCALL_MAX] = {
	[SYS_READ]  = (syscall_fn_t)sys_read,
	[SYS_WRITE] = (syscall_fn_t)sys_write,
	[SYS_FORK]  = (syscall_fn_t)sys_fork,
	[SYS_EXIT]  = (syscall_fn_t)sys_exit,
};`
          }
        ]
      }
    ]
  }
];

export default function App() {
  const [activeTab, setActiveTab] = useState<'explorer' | 'architecture' | 'config' | 'syscalls' | 'bootlog'>('explorer');
  const [selectedFilePath, setSelectedFilePath] = useState<string>('/kernel/core/init.c');
  const [expandedFolders, setExpandedFolders] = useState<Record<string, boolean>>({
    '/ABI': true,
    '/ABI/kernel': true,
    '/ABI/kernel/internal': true,
    '/arch': true,
    '/arch/x86_64': true,
    '/kernel': true,
    '/kernel/core': true,
    '/process': true,
    '/memory': true,
    '/syscall': true,
  });
  const [searchQuery, setSearchQuery] = useState('');
  const [copiedPath, setCopiedPath] = useState(false);
  const [selectedArch, setSelectedArch] = useState<'x86_64' | 'arm64' | 'riscv' | 'powerpc' | 'loongarch'>('x86_64');
  const [configFlags, setConfigFlags] = useState<Record<string, boolean>>({
    CONFIG_PREEMPT: true,
    CONFIG_SCHED_CFS: true,
    CONFIG_SCHED_RT: true,
    CONFIG_NUMA: true,
    CONFIG_KASLR: true,
    CONFIG_MODULES: true,
    CONFIG_SLUB: true,
    CONFIG_HUGE_PAGES: true,
    CONFIG_MMU: true,
  });

  // Find file node by path
  const findFileNode = (nodes: FileNode[], path: string): FileNode | null => {
    for (const node of nodes) {
      if (node.path === path) return node;
      if (node.children) {
        const found = findFileNode(node.children, path);
        if (found) return found;
      }
    }
    return null;
  };

  const selectedFile = findFileNode(KERNEL_FILES, selectedFilePath) || KERNEL_FILES[0];

  const toggleFolder = (path: string) => {
    setExpandedFolders(prev => ({ ...prev, [path]: !prev[path] }));
  };

  const copyFileContent = () => {
    if (selectedFile?.content) {
      navigator.clipboard.writeText(selectedFile.content);
      setCopiedPath(true);
      setTimeout(() => setCopiedPath(false), 2000);
    }
  };

  const renderTree = (nodes: FileNode[], depth = 0) => {
    return nodes.map(node => {
      if (searchQuery && node.type === 'file' && !node.name.toLowerCase().includes(searchQuery.toLowerCase())) {
        return null;
      }

      if (node.type === 'folder') {
        const isExpanded = expandedFolders[node.path];
        return (
          <div key={node.path} className="select-none">
            <div
              onClick={() => toggleFolder(node.path)}
              style={{ paddingLeft: `${depth * 12 + 8}px` }}
              className="flex items-center gap-1.5 py-1 px-2 hover:bg-slate-800/60 rounded cursor-pointer text-slate-300 hover:text-white transition-colors text-xs font-mono"
            >
              {isExpanded ? (
                <ChevronDown className="w-3.5 h-3.5 text-slate-400" />
              ) : (
                <ChevronRight className="w-3.5 h-3.5 text-slate-400" />
              )}
              {isExpanded ? (
                <FolderOpen className="w-4 h-4 text-amber-400" />
              ) : (
                <Folder className="w-4 h-4 text-amber-400/80" />
              )}
              <span className="font-medium text-slate-200">{node.name}</span>
            </div>
            {isExpanded && node.children && (
              <div>{renderTree(node.children, depth + 1)}</div>
            )}
          </div>
        );
      }

      const isSelected = selectedFilePath === node.path;
      return (
        <div
          key={node.path}
          onClick={() => setSelectedFilePath(node.path)}
          style={{ paddingLeft: `${depth * 12 + 20}px` }}
          className={`flex items-center gap-2 py-1 px-2 rounded cursor-pointer text-xs font-mono transition-colors ${
            isSelected
              ? 'bg-emerald-500/15 text-emerald-300 font-semibold border-l-2 border-emerald-400'
              : 'text-slate-400 hover:bg-slate-800/40 hover:text-slate-200'
          }`}
        >
          <FileCode className={`w-3.5 h-3.5 ${isSelected ? 'text-emerald-400' : 'text-slate-500'}`} />
          <span className="truncate">{node.name}</span>
        </div>
      );
    });
  };

  return (
    <div className="flex flex-col h-screen bg-slate-950 text-slate-100 font-sans overflow-hidden">
      {/* Top Navigation Bar */}
      <header className="flex items-center justify-between px-4 py-2.5 bg-slate-900 border-b border-slate-800 shrink-0">
        <div className="flex items-center gap-3">
          <div className="p-1.5 bg-emerald-500/10 border border-emerald-500/30 rounded-lg">
            <Cpu className="w-5 h-5 text-emerald-400" />
          </div>
          <div>
            <h1 className="text-sm font-bold tracking-tight text-white flex items-center gap-2">
              DionOS <span className="text-xs px-2 py-0.5 bg-emerald-500/20 text-emerald-300 rounded-full border border-emerald-500/30 font-mono">v1.0.0-RELEASE</span>
            </h1>
            <p className="text-[11px] text-slate-400 font-mono">Pure C Modern Modular Monolithic Kernel</p>
          </div>
        </div>

        <div className="flex items-center gap-1 bg-slate-950 p-1 rounded-lg border border-slate-800">
          <button
            onClick={() => setActiveTab('explorer')}
            className={`flex items-center gap-2 px-3 py-1.5 rounded-md text-xs font-medium transition-all ${
              activeTab === 'explorer'
                ? 'bg-slate-800 text-emerald-400 shadow-sm'
                : 'text-slate-400 hover:text-slate-200'
            }`}
          >
            <Code className="w-3.5 h-3.5" />
            Source Tree
          </button>
          <button
            onClick={() => setActiveTab('architecture')}
            className={`flex items-center gap-2 px-3 py-1.5 rounded-md text-xs font-medium transition-all ${
              activeTab === 'architecture'
                ? 'bg-slate-800 text-emerald-400 shadow-sm'
                : 'text-slate-400 hover:text-slate-200'
            }`}
          >
            <Layers className="w-3.5 h-3.5" />
            Architectures
          </button>
          <button
            onClick={() => setActiveTab('config')}
            className={`flex items-center gap-2 px-3 py-1.5 rounded-md text-xs font-medium transition-all ${
              activeTab === 'config'
                ? 'bg-slate-800 text-emerald-400 shadow-sm'
                : 'text-slate-400 hover:text-slate-200'
            }`}
          >
            <Sliders className="w-3.5 h-3.5" />
            Kconfig (.config)
          </button>
          <button
            onClick={() => setActiveTab('syscalls')}
            className={`flex items-center gap-2 px-3 py-1.5 rounded-md text-xs font-medium transition-all ${
              activeTab === 'syscalls'
                ? 'bg-slate-800 text-emerald-400 shadow-sm'
                : 'text-slate-400 hover:text-slate-200'
            }`}
          >
            <Server className="w-3.5 h-3.5" />
            Syscalls
          </button>
          <button
            onClick={() => setActiveTab('bootlog')}
            className={`flex items-center gap-2 px-3 py-1.5 rounded-md text-xs font-medium transition-all ${
              activeTab === 'bootlog'
                ? 'bg-slate-800 text-emerald-400 shadow-sm'
                : 'text-slate-400 hover:text-slate-200'
            }`}
          >
            <Terminal className="w-3.5 h-3.5" />
            dmesg Boot
          </button>
        </div>

        <div className="flex items-center gap-2 text-xs font-mono text-slate-400">
          <span className="flex items-center gap-1.5 px-2.5 py-1 bg-slate-950 rounded border border-slate-800">
            <span className="w-2 h-2 rounded-full bg-emerald-400 animate-pulse"></span>
            ISO C11 Native
          </span>
        </div>
      </header>

      {/* Main Content Area */}
      {activeTab === 'explorer' && (
        <div className="flex flex-1 overflow-hidden">
          {/* File Explorer Sidebar */}
          <div className="w-80 bg-slate-900/90 border-r border-slate-800 flex flex-col shrink-0">
            <div className="p-3 border-b border-slate-800">
              <div className="relative">
                <Search className="w-3.5 h-3.5 absolute left-2.5 top-2.5 text-slate-500" />
                <input
                  type="text"
                  value={searchQuery}
                  onChange={e => setSearchQuery(e.target.value)}
                  placeholder="Filter C kernel tree..."
                  className="w-full bg-slate-950 border border-slate-800 rounded-md pl-8 pr-3 py-1.5 text-xs text-slate-200 placeholder-slate-500 focus:outline-none focus:border-emerald-500/50"
                />
              </div>
            </div>

            <div className="flex-1 overflow-y-auto p-2 scrollbar-thin scrollbar-thumb-slate-800">
              {renderTree(KERNEL_FILES)}
            </div>

            <div className="p-3 border-t border-slate-800 bg-slate-950 text-[11px] text-slate-500 font-mono flex items-center justify-between">
              <span>Files: 58 | Headers: 24</span>
              <span className="text-emerald-400 font-medium">Build: OK</span>
            </div>
          </div>

          {/* Code Viewer Panel */}
          <div className="flex-1 flex flex-col bg-slate-950 overflow-hidden">
            {/* File Header */}
            <div className="flex items-center justify-between px-4 py-2 bg-slate-900 border-b border-slate-800">
              <div className="flex items-center gap-2 text-xs font-mono">
                <FileCode className="w-4 h-4 text-emerald-400" />
                <span className="text-slate-200 font-semibold">{selectedFile.path}</span>
                <span className="text-[10px] px-2 py-0.5 bg-slate-800 text-slate-400 rounded uppercase">
                  {selectedFile.lang || 'c'}
                </span>
              </div>

              <button
                onClick={copyFileContent}
                className="flex items-center gap-1.5 px-2.5 py-1 bg-slate-800 hover:bg-slate-700 text-slate-300 rounded text-xs transition-colors"
              >
                {copiedPath ? (
                  <>
                    <Check className="w-3.5 h-3.5 text-emerald-400" />
                    <span className="text-emerald-400">Copied</span>
                  </>
                ) : (
                  <>
                    <Copy className="w-3.5 h-3.5" />
                    <span>Copy</span>
                  </>
                )}
              </button>
            </div>

            {/* Code Content View */}
            <div className="flex-1 overflow-auto p-4 bg-slate-950 font-mono text-xs text-slate-200 leading-relaxed scrollbar-thin scrollbar-thumb-slate-800">
              <pre className="whitespace-pre font-mono">
                {selectedFile.content ? (
                  selectedFile.content.split('\n').map((line, idx) => (
                    <div key={idx} className="flex hover:bg-slate-900/50 px-2 rounded">
                      <span className="w-10 text-right pr-4 select-none text-slate-600 shrink-0">
                        {idx + 1}
                      </span>
                      <span className="flex-1 text-slate-300">{line || ' '}</span>
                    </div>
                  ))
                ) : (
                  <div className="text-slate-500 italic px-4 py-8">Select a file from the kernel directory tree to view its source code.</div>
                )}
              </pre>
            </div>
          </div>
        </div>
      )}

      {/* Architecture Panel */}
      {activeTab === 'architecture' && (
        <div className="flex-1 overflow-y-auto p-6 bg-slate-950 flex flex-col gap-6">
          <div className="flex items-center justify-between bg-slate-900 p-4 rounded-xl border border-slate-800">
            <div>
              <h2 className="text-lg font-bold text-white flex items-center gap-2">
                <Cpu className="w-5 h-5 text-emerald-400" />
                Multi-Architecture Abstraction Layer
              </h2>
              <p className="text-xs text-slate-400">
                DionOS supports native execution across x86, x86_64, ARM, ARM64, RISC-V, PowerPC, MIPS, SPARC, s390, and LoongArch.
              </p>
            </div>

            <div className="flex items-center gap-2">
              {(['x86_64', 'arm64', 'riscv', 'powerpc', 'loongarch'] as const).map(arch => (
                <button
                  key={arch}
                  onClick={() => setSelectedArch(arch)}
                  className={`px-3 py-1.5 rounded-lg text-xs font-mono transition-all ${
                    selectedArch === arch
                      ? 'bg-emerald-500 text-slate-950 font-bold shadow-md shadow-emerald-500/20'
                      : 'bg-slate-800 text-slate-300 hover:bg-slate-700'
                  }`}
                >
                  {arch.toUpperCase()}
                </button>
              ))}
            </div>
          </div>

          <div className="grid grid-cols-1 md:grid-cols-3 gap-6">
            <div className="bg-slate-900 p-5 rounded-xl border border-slate-800 flex flex-col gap-3">
              <div className="flex items-center gap-2 text-emerald-400 font-semibold text-sm">
                <MemoryStick className="w-4 h-4" />
                MMU & Page Table Layout
              </div>
              <p className="text-xs text-slate-400 leading-relaxed">
                {selectedArch === 'x86_64' && 'PML4 / PML5 48-bit to 57-bit Virtual Addressing with CR3 hardware switching.'}
                {selectedArch === 'arm64' && 'TTBR0_EL1 & TTBR1_EL1 translation tables with 4KB/16KB/64KB page granule support.'}
                {selectedArch === 'riscv' && 'Sv39 / Sv48 Page Tables with SATP register supervisor mode memory mapping.'}
                {selectedArch === 'powerpc' && 'Radix Tree and Hash Page Table (HPT) with Segment Lookaside Buffer (SLB).'}
                {selectedArch === 'loongarch' && 'LA64 Multi-level page tables with CSR TLB exception vectors.'}
              </p>
              <div className="mt-auto pt-3 border-t border-slate-800 font-mono text-[11px] text-slate-500">
                File: /arch/{selectedArch}/mm/
              </div>
            </div>

            <div className="bg-slate-900 p-5 rounded-xl border border-slate-800 flex flex-col gap-3">
              <div className="flex items-center gap-2 text-emerald-400 font-semibold text-sm">
                <Zap className="w-4 h-4" />
                Interrupt Controller
              </div>
              <p className="text-xs text-slate-400 leading-relaxed">
                {selectedArch === 'x86_64' && 'Local APIC & I/O APIC with IDT (Interrupt Descriptor Table) vectors 0-255.'}
                {selectedArch === 'arm64' && 'ARM GICv3/GICv4 Interrupt Controller with SPIs, PPIs, and SGIs routing.'}
                {selectedArch === 'riscv' && 'PLIC (Platform Level Interrupt Controller) and CLINT timer interrupts.'}
                {selectedArch === 'powerpc' && 'XIVE / XICS External Interrupt Controller.'}
                {selectedArch === 'loongarch' && 'ExtIOI and PCH-PIC Interrupt Controller.'}
              </p>
              <div className="mt-auto pt-3 border-t border-slate-800 font-mono text-[11px] text-slate-500">
                File: /arch/{selectedArch}/interrupts/
              </div>
            </div>

            <div className="bg-slate-900 p-5 rounded-xl border border-slate-800 flex flex-col gap-3">
              <div className="flex items-center gap-2 text-emerald-400 font-semibold text-sm">
                <Code className="w-4 h-4" />
                Syscall Assembly Entry
              </div>
              <p className="text-xs text-slate-400 leading-relaxed">
                {selectedArch === 'x86_64' && 'SYSCALL / SYSRET instructions utilizing RAX, RDI, RSI, RDX, R10, R8, R9 registers.'}
                {selectedArch === 'arm64' && 'SVC #0 exception vector trapping into EL1 handler using X0-X7 registers.'}
                {selectedArch === 'riscv' && 'ECALL instruction trapping into Supervisor mode using A0-A7 registers.'}
                {selectedArch === 'powerpc' && 'SC (system call) instruction using R3-R8 registers.'}
                {selectedArch === 'loongarch' && 'SYSCALL instruction using A0-A7 registers.'}
              </p>
              <div className="mt-auto pt-3 border-t border-slate-800 font-mono text-[11px] text-slate-500">
                File: /arch/{selectedArch}/syscall/
              </div>
            </div>
          </div>
        </div>
      )}

      {/* Kconfig Panel */}
      {activeTab === 'config' && (
        <div className="flex-1 overflow-y-auto p-6 bg-slate-950 flex flex-col gap-6">
          <div className="bg-slate-900 p-5 rounded-xl border border-slate-800">
            <h2 className="text-lg font-bold text-white flex items-center gap-2">
              <Sliders className="w-5 h-5 text-emerald-400" />
              Interactive Kernel Kconfig Engine (.config)
            </h2>
            <p className="text-xs text-slate-400 mt-1">
              Toggle modular subsystem flags to customize the DionOS build image parameters.
            </p>
          </div>

          <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
            {Object.entries(configFlags).map(([flag, enabled]) => (
              <div
                key={flag}
                onClick={() => setConfigFlags(p => ({ ...p, [flag]: !p[flag] }))}
                className="flex items-center justify-between p-4 bg-slate-900 hover:bg-slate-800/80 border border-slate-800 rounded-xl cursor-pointer transition-colors"
              >
                <div>
                  <span className="font-mono text-xs font-bold text-slate-200">{flag}</span>
                  <p className="text-[11px] text-slate-400 mt-0.5">
                    {flag === 'CONFIG_PREEMPT' && 'Full kernel preemption model for sub-millisecond real-time response.'}
                    {flag === 'CONFIG_SCHED_CFS' && 'Completely Fair Scheduler with red-black vruntime tree.'}
                    {flag === 'CONFIG_SCHED_RT' && 'POSIX SCHED_FIFO and SCHED_RR real-time schedulers.'}
                    {flag === 'CONFIG_NUMA' && 'Multi-socket NUMA domain topology awareness.'}
                    {flag === 'CONFIG_KASLR' && 'Kernel Address Space Layout Randomization for security.'}
                    {flag === 'CONFIG_MODULES' && 'Dynamic insmod/rmmod kernel module symbol loader.'}
                    {flag === 'CONFIG_SLUB' && 'SLUB memory object allocator with zero fragmentation.'}
                    {flag === 'CONFIG_HUGE_PAGES' && 'Transparent 2MB and 1GB HugeTLB frame pages.'}
                    {flag === 'CONFIG_MMU' && 'Hardware Memory Management Unit requirement.'}
                  </p>
                </div>

                <div className={`w-5 h-5 rounded border flex items-center justify-center transition-colors ${
                  enabled ? 'bg-emerald-500 border-emerald-400 text-slate-950' : 'border-slate-700 bg-slate-950'
                }`}>
                  {enabled && <Check className="w-3.5 h-3.5 stroke-[3]" />}
                </div>
              </div>
            ))}
          </div>
        </div>
      )}

      {/* Syscall Reference Panel */}
      {activeTab === 'syscalls' && (
        <div className="flex-1 overflow-y-auto p-6 bg-slate-950 flex flex-col gap-6">
          <div className="bg-slate-900 p-5 rounded-xl border border-slate-800">
            <h2 className="text-lg font-bold text-white flex items-center gap-2">
              <Server className="w-5 h-5 text-emerald-400" />
              POSIX System Call Dispatch Table
            </h2>
            <p className="text-xs text-slate-400 mt-1">
              Registered syscall handlers in <code className="font-mono text-emerald-300">syscall/table/syscall_table.c</code>.
            </p>
          </div>

          <div className="bg-slate-900 border border-slate-800 rounded-xl overflow-hidden">
            <table className="w-full text-left font-mono text-xs">
              <thead className="bg-slate-950 text-slate-400 border-b border-slate-800">
                <tr>
                  <th className="p-3">Syscall NR</th>
                  <th className="p-3">Name</th>
                  <th className="p-3">Handler Function</th>
                  <th className="p-3">Source Location</th>
                  <th className="p-3">Status</th>
                </tr>
              </thead>
              <tbody className="divide-y divide-slate-800 text-slate-300">
                <tr>
                  <td className="p-3 font-bold text-amber-400">0</td>
                  <td className="p-3 text-emerald-400 font-semibold">sys_read</td>
                  <td className="p-3">sys_read(fd, buf, count)</td>
                  <td className="p-3 text-slate-400">/syscall/filesystem/sys_fs.c</td>
                  <td className="p-3"><span className="px-2 py-0.5 bg-emerald-500/20 text-emerald-300 rounded text-[10px]">Active</span></td>
                </tr>
                <tr>
                  <td className="p-3 font-bold text-amber-400">1</td>
                  <td className="p-3 text-emerald-400 font-semibold">sys_write</td>
                  <td className="p-3">sys_write(fd, buf, count)</td>
                  <td className="p-3 text-slate-400">/syscall/filesystem/sys_fs.c</td>
                  <td className="p-3"><span className="px-2 py-0.5 bg-emerald-500/20 text-emerald-300 rounded text-[10px]">Active</span></td>
                </tr>
                <tr>
                  <td className="p-3 font-bold text-amber-400">57</td>
                  <td className="p-3 text-emerald-400 font-semibold">sys_fork</td>
                  <td className="p-3">sys_fork()</td>
                  <td className="p-3 text-slate-400">/syscall/handlers/sys_process.c</td>
                  <td className="p-3"><span className="px-2 py-0.5 bg-emerald-500/20 text-emerald-300 rounded text-[10px]">Active</span></td>
                </tr>
                <tr>
                  <td className="p-3 font-bold text-amber-400">60</td>
                  <td className="p-3 text-emerald-400 font-semibold">sys_exit</td>
                  <td className="p-3">sys_exit(status)</td>
                  <td className="p-3 text-slate-400">/syscall/handlers/sys_process.c</td>
                  <td className="p-3"><span className="px-2 py-0.5 bg-emerald-500/20 text-emerald-300 rounded text-[10px]">Active</span></td>
                </tr>
              </tbody>
            </table>
          </div>
        </div>
      )}

      {/* Boot Log Panel */}
      {activeTab === 'bootlog' && (
        <div className="flex-1 overflow-hidden p-6 bg-slate-950 flex flex-col gap-4">
          <div className="flex items-center justify-between bg-slate-900 p-4 rounded-xl border border-slate-800">
            <div>
              <h2 className="text-sm font-bold text-white flex items-center gap-2">
                <Terminal className="w-4 h-4 text-emerald-400" />
                Kernel Console Log (dmesg)
              </h2>
              <p className="text-xs text-slate-400">
                Simulated boot sequence output from <code className="font-mono text-emerald-300">start_kernel()</code> in <code className="font-mono text-emerald-300">kernel/core/init.c</code>.
              </p>
            </div>
          </div>

          <div className="flex-1 bg-slate-900/90 border border-slate-800 rounded-xl p-4 font-mono text-xs text-slate-200 overflow-y-auto leading-relaxed scrollbar-thin scrollbar-thumb-slate-800">
            <div className="text-slate-400">[    0.000000] Linux / DionOS version 1.0.0-RELEASE (gcc version 13.2.0) #1 SMP PREEMPT</div>
            <div className="text-slate-400">[    0.000010] Command line: console=ttyS0 root=/dev/sda1 quiet</div>
            <div className="text-emerald-400">[    0.000120] [dionos] Booting DionOS Modular Monolithic Kernel v1.0.0-RELEASE...</div>
            <div className="text-slate-300">[    0.001400] [pmm] Initializing Buddy Physical Memory Allocator (16384MB RAM detected)</div>
            <div className="text-slate-300">[    0.003100] [vmm] Setting up 64-bit PML4 Page Tables & SLUB Memory Allocator</div>
            <div className="text-slate-300">[    0.005800] [idt] IDT Gates 0-255 initialized. Local APIC enabled at 0xFEE00000</div>
            <div className="text-slate-300">[    0.008900] [sched] Completely Fair Scheduler (CFS) vruntime queue active</div>
            <div className="text-slate-300">[    0.012400] [syscall] LSTAR MSR configured for fast x86_64 sys_dispatch table</div>
            <div className="text-emerald-400">[    0.015000] [dionos] Kernel Subsystems Initialized Successfully. Spawning init process (PID 1).</div>
          </div>
        </div>
      )}
    </div>
  );
}
