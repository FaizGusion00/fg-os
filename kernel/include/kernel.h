/**
 * @file kernel.h
 * @brief Main kernel header file for FG-OS
 * 
 * This file contains the core kernel definitions, macros, and function prototypes.
 * It serves as the central include for all kernel modules.
 * 
 * @author Faiz Nasir
 * @company FGCompany Official
 * @version 1.0.0
 * @date 2024
 * @copyright © 2024 FGCompany Official. All rights reserved.
 */

#ifndef __KERNEL_H__
#define __KERNEL_H__

#include "types.h"

// Kernel version information
#define KERNEL_NAME         "FG-OS"
#define KERNEL_VERSION      "1.0.0"
#define KERNEL_AUTHOR       "Faiz Nasir"
#define KERNEL_COMPANY      "FGCompany Official"
#define KERNEL_COPYRIGHT    "© 2024 FGCompany Official. All rights reserved."

// Development phase tracking
#define CURRENT_PHASE       11
#define TOTAL_PHASES        20
#define PHASE_NAME          "File System Implementation"

// Kernel configuration
#define KERNEL_STACK_SIZE   0x4000          // 16KB kernel stack
#define USER_STACK_SIZE     0x100000        // 1MB user stack
#define HEAP_START          0x1000000       // 16MB heap start
#define HEAP_SIZE           0x10000000      // 256MB heap size

// Memory layout constants
#define KERNEL_VIRTUAL_BASE 0xFFFFFFFF80000000UL
#define USER_VIRTUAL_BASE   0x0000000000400000UL
#define KERNEL_HEAP_START   0xFFFFFFFF90000000UL

// Boot information structure
struct boot_info {
    uint32_t magic;                 /**< Boot magic number */
    uint32_t flags;                 /**< Boot flags */
    uint64_t memory_low;            /**< Lower memory size */
    uint64_t memory_high;           /**< Higher memory size */
    uint64_t kernel_start;          /**< Kernel start address */
    uint64_t kernel_end;            /**< Kernel end address */
    uint64_t initrd_start;          /**< Initial ramdisk start */
    uint64_t initrd_end;            /**< Initial ramdisk end */
    char     cmdline[256];          /**< Kernel command line */
} __attribute__((packed));

// Kernel subsystem status
enum subsystem_status {
    SUBSYS_UNINITIALIZED = 0,      /**< Subsystem not initialized */
    SUBSYS_INITIALIZING,           /**< Subsystem initializing */
    SUBSYS_RUNNING,                /**< Subsystem running normally */
    SUBSYS_ERROR,                  /**< Subsystem in error state */
    SUBSYS_SHUTDOWN                /**< Subsystem shut down */
};

// Kernel subsystem information
struct kernel_subsystem {
    const char           *name;     /**< Subsystem name */
    enum subsystem_status status;   /**< Current status */
    int (*init)(void);              /**< Initialization function */
    void (*shutdown)(void);         /**< Shutdown function */
    uint64_t             init_time; /**< Initialization timestamp */
};

// Forward declarations
struct boot_info;

// Kernel initialization functions
int  kernel_early_init(struct boot_info *boot_info);
int  kernel_init(void);
void kernel_main(void);
void kernel_shutdown(void);

// Subsystem management
int  register_subsystem(struct kernel_subsystem *subsys);
int  init_subsystem(const char *name);
void shutdown_subsystem(const char *name);
void shutdown_all_subsystems(void);

// Kernel information functions
void print_kernel_banner(void);
void print_boot_info(struct boot_info *boot_info);
void print_memory_layout(void);
void print_phase_info(void);

// Utility macros
#define ALIGN_UP(addr, align)   (((addr) + (align) - 1) & ~((align) - 1))
#define ALIGN_DOWN(addr, align) ((addr) & ~((align) - 1))
#define ARRAY_SIZE(arr)         (sizeof(arr) / sizeof((arr)[0]))
#define MIN(a, b)               ((a) < (b) ? (a) : (b))
#define MAX(a, b)               ((a) > (b) ? (a) : (b))

// Kernel assertion macro
#ifdef DEBUG
    #define KASSERT(expr) \
        do { \
            if (!(expr)) { \
                panic("Assertion failed: %s at %s:%d", #expr, __FILE__, __LINE__); \
            } \
        } while (0)
#else
    #define KASSERT(expr) ((void)0)
#endif

// Kernel debug macros
#ifdef DEBUG
    #define KDEBUG(fmt, ...) \
        printf("[DEBUG] %s:%d: " fmt "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
    #define KDEBUG(fmt, ...) ((void)0)
#endif

// Kernel info, warning, and error macros
#define KINFO(fmt, ...) \
    printf("[INFO] " fmt "\n", ##__VA_ARGS__)

#define KWARN(fmt, ...) \
    printf("[WARN] " fmt "\n", ##__VA_ARGS__)

#define KERROR(fmt, ...) \
    printf("[ERROR] " fmt "\n", ##__VA_ARGS__)

// Memory barriers and atomic operations
#define mb()    __asm__ __volatile__("mfence" ::: "memory")
#define rmb()   __asm__ __volatile__("lfence" ::: "memory")
#define wmb()   __asm__ __volatile__("sfence" ::: "memory")

// Kernel section attributes
#define __init      __attribute__((section(".init.text")))
#define __initdata  __attribute__((section(".init.data")))
#define __exit      __attribute__((section(".exit.text")))
#define __exitdata  __attribute__((section(".exit.data")))

// Function attributes
#define __noreturn  __attribute__((noreturn))
#define __packed    __attribute__((packed))
#define __aligned(x) __attribute__((aligned(x)))
#define __unused    __attribute__((unused))
#define __used      __attribute__((used))

// Parameter marking macros
#define UNUSED(x)   ((void)(x))

// Interrupt control
#define cli()       __asm__ __volatile__("cli" ::: "memory")
#define sti()       __asm__ __volatile__("sti" ::: "memory")
#define halt()      __asm__ __volatile__("hlt")

// Save and restore interrupt state
static inline uint64_t save_flags(void) {
    uint64_t flags;
    __asm__ __volatile__("pushfq; popq %0" : "=r"(flags) :: "memory");
    return flags;
}

static inline void restore_flags(uint64_t flags) {
    __asm__ __volatile__("pushq %0; popfq" :: "r"(flags) : "memory");
}

// Critical section helpers
#define local_irq_save(flags) \
    do { \
        flags = save_flags(); \
        cli(); \
    } while (0)

#define local_irq_restore(flags) \
    restore_flags(flags)

// Kernel panic function
__noreturn void panic(const char *fmt, ...);

// Basic I/O functions (implemented in console driver)
int printf(const char *fmt, ...);
int putchar(int c);
int puts(const char *s);

// Basic string functions (implemented in string_stubs.c)
char* strncpy(char* dest, const char* src, size_t n);
void* memset(void* ptr, int value, size_t size);
int memcmp(const void* ptr1, const void* ptr2, size_t num);
void* memcpy(void* dest, const void* src, size_t n);

// System functions (panic already declared above as __noreturn)

#endif /* __KERNEL_H__ */ 