 /*
 * FG-OS x86_64 Architecture Header
 * Phase 4: Kernel Architecture Design
 * 
 * Developed by: Faiz Nasir
 * Company: FGCompany Official
 * 
 * Architecture-specific definitions for x86_64 platform.
 */

#ifndef ARCH_X86_64_H
#define ARCH_X86_64_H

#include <types.h>

// CPU Feature Flags
#define CPU_FEATURE_FPU         (1 << 0)   // Floating Point Unit
#define CPU_FEATURE_PAE         (1 << 1)   // Physical Address Extension
#define CPU_FEATURE_SSE         (1 << 2)   // Streaming SIMD Extensions
#define CPU_FEATURE_SSE2        (1 << 3)   // SSE2 Extensions
#define CPU_FEATURE_RDTSC       (1 << 4)   // Read Time-Stamp Counter
#define CPU_FEATURE_APIC        (1 << 5)   // Advanced Programmable Interrupt Controller

// Memory Management Constants (PAGE_SIZE, PAGE_SHIFT, PAGE_MASK defined in types.h)
// #define PAGE_SIZE               4096
// #define PAGE_SHIFT              12 
// #define PAGE_MASK               (PAGE_SIZE - 1)

// Virtual Memory Layout (x86_64)
#define KERNEL_BASE             0xFFFFFFFF80000000UL
#define USER_BASE               0x0000000000400000UL
#define KERNEL_HEAP_BASE        0xFFFFFFFF90000000UL
#define KERNEL_STACK_BASE       0xFFFFFFFFA0000000UL

// Page Table Entry Flags
#define PTE_PRESENT             (1 << 0)
#define PTE_WRITABLE            (1 << 1)
#define PTE_USER                (1 << 2)
#define PTE_WRITE_THROUGH       (1 << 3)
#define PTE_CACHE_DISABLE       (1 << 4)
#define PTE_ACCESSED            (1 << 5)
#define PTE_DIRTY               (1 << 6)
#define PTE_HUGE                (1 << 7)
#define PTE_GLOBAL              (1 << 8)
#define PTE_NO_EXECUTE          (1UL << 63)

// Interrupt and Exception Vectors
#define EXCEPTION_DIVIDE_ERROR      0
#define EXCEPTION_DEBUG             1
#define EXCEPTION_NMI               2
#define EXCEPTION_BREAKPOINT        3
#define EXCEPTION_OVERFLOW          4
#define EXCEPTION_BOUND_RANGE       5
#define EXCEPTION_INVALID_OPCODE    6
#define EXCEPTION_DEVICE_NOT_AVAIL  7
#define EXCEPTION_DOUBLE_FAULT      8
#define EXCEPTION_INVALID_TSS       10
#define EXCEPTION_SEGMENT_NOT_PRES  11
#define EXCEPTION_STACK_FAULT       12
#define EXCEPTION_GENERAL_PROTECT   13
#define EXCEPTION_PAGE_FAULT        14
#define EXCEPTION_X87_FPU           16
#define EXCEPTION_ALIGNMENT_CHECK   17
#define EXCEPTION_MACHINE_CHECK     18
#define EXCEPTION_SIMD_FP           19

// System Call Interface
#define SYSCALL_VECTOR              0x80
#define SYSCALL_MAX_ARGS            6

// CPU State Structure
struct cpu_state {
    uint64_t rax, rbx, rcx, rdx;
    uint64_t rsi, rdi, rbp, rsp;
    uint64_t r8, r9, r10, r11;
    uint64_t r12, r13, r14, r15;
    uint64_t rip, rflags;
    uint16_t cs, ds, es, fs, gs, ss;
} __attribute__((packed));

// Architecture-specific Functions
void arch_init(void);
void arch_enable_interrupts(void);
void arch_disable_interrupts(void);
void arch_halt(void);
void arch_reboot(void);

// Memory Management Functions
void arch_init_paging(void);
void arch_flush_tlb(void);
void arch_invlpg(uint64_t addr);

// CPU Feature Detection
uint32_t arch_get_cpu_features(void);
const char* arch_get_cpu_vendor(void);

#endif // ARCH_X86_64_H