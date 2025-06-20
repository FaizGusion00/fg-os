/**
 * @file panic.h
 * @brief Kernel panic handling for FG-OS
 * 
 * This file contains definitions and functions for handling kernel panics,
 * including stack traces, error reporting, and system halt procedures.
 * 
 * @author Faiz Nasir
 * @company FGCompany Official
 * @version 1.0.0
 * @date 2024
 * @copyright © 2024 FGCompany Official. All rights reserved.
 */

#ifndef __PANIC_H__
#define __PANIC_H__

#include "types.h"

// Panic types
enum panic_type {
    PANIC_GENERAL = 0,          /**< General kernel panic */
    PANIC_MEMORY,               /**< Memory-related panic */
    PANIC_HARDWARE,             /**< Hardware-related panic */
    PANIC_FILESYSTEM,           /**< File system panic */
    PANIC_NETWORK,              /**< Network-related panic */
    PANIC_SECURITY,             /**< Security violation panic */
    PANIC_ASSERTION,            /**< Assertion failure */
    PANIC_STACKOVERFLOW,        /**< Stack overflow */
    PANIC_DEADLOCK,             /**< Deadlock detected */
    PANIC_CORRUPTION            /**< Data corruption detected */
};

// Stack frame structure for stack traces
struct stack_frame {
    struct stack_frame *next;   /**< Next frame in stack */
    void               *addr;   /**< Return address */
};

// Panic information structure
struct panic_info {
    enum panic_type type;       /**< Type of panic */
    const char     *message;    /**< Panic message */
    const char     *file;       /**< Source file where panic occurred */
    const char     *function;   /**< Function where panic occurred */
    uint32_t        line;       /**< Line number where panic occurred */
    uint64_t        error_code; /**< Error code (if applicable) */
    uint64_t        timestamp;  /**< Panic timestamp */
    uint32_t        cpu_id;     /**< CPU ID where panic occurred */
    uint64_t        stack_ptr;  /**< Stack pointer at panic */
    uint64_t        registers[16]; /**< CPU registers at panic */
};

// Panic handler function type
typedef void (*panic_handler_t)(struct panic_info *info);

// Main panic function (noreturn)
__noreturn void panic(const char *fmt, ...);

// Typed panic functions
__noreturn void panic_memory(const char *fmt, ...);
__noreturn void panic_hardware(const char *fmt, ...);
__noreturn void panic_filesystem(const char *fmt, ...);
__noreturn void panic_network(const char *fmt, ...);
__noreturn void panic_security(const char *fmt, ...);
__noreturn void panic_assertion(const char *expr, const char *file, 
                                const char *function, uint32_t line);
__noreturn void panic_stackoverflow(void);
__noreturn void panic_deadlock(const char *resource);
__noreturn void panic_corruption(const char *data_type);

// Panic with specific information
__noreturn void panic_with_info(enum panic_type type, const char *file,
                                const char *function, uint32_t line,
                                uint64_t error_code, const char *fmt, ...);

// Stack trace functions
void print_stack_trace(void);
void print_stack_trace_from(void *frame_ptr);
int  capture_stack_trace(void **buffer, int max_frames);
void print_registers(uint64_t *registers);

// Panic handler management
void register_panic_handler(panic_handler_t handler);
void unregister_panic_handler(panic_handler_t handler);

// System state functions
void save_system_state(struct panic_info *info);
void print_system_info(void);
void print_memory_usage(void);
void print_process_info(void);

// Panic recovery (for non-critical errors)
int  try_recover_from_error(enum panic_type type, uint64_t error_code);
void set_panic_on_oops(bool enable);

// Debug functions
void dump_memory(void *addr, size_t size);
void dump_kernel_log(void);
void save_crash_dump(struct panic_info *info);

// Panic macros
#define PANIC(fmt, ...) \
    panic_with_info(PANIC_GENERAL, __FILE__, __FUNCTION__, __LINE__, 0, fmt, ##__VA_ARGS__)

#define PANIC_MEMORY(fmt, ...) \
    panic_with_info(PANIC_MEMORY, __FILE__, __FUNCTION__, __LINE__, 0, fmt, ##__VA_ARGS__)

#define PANIC_HARDWARE(fmt, ...) \
    panic_with_info(PANIC_HARDWARE, __FILE__, __FUNCTION__, __LINE__, 0, fmt, ##__VA_ARGS__)

#define PANIC_FS(fmt, ...) \
    panic_with_info(PANIC_FILESYSTEM, __FILE__, __FUNCTION__, __LINE__, 0, fmt, ##__VA_ARGS__)

#define PANIC_NET(fmt, ...) \
    panic_with_info(PANIC_NETWORK, __FILE__, __FUNCTION__, __LINE__, 0, fmt, ##__VA_ARGS__)

#define PANIC_SECURITY(fmt, ...) \
    panic_with_info(PANIC_SECURITY, __FILE__, __FUNCTION__, __LINE__, 0, fmt, ##__VA_ARGS__)

#define PANIC_ASSERT(expr) \
    do { \
        if (!(expr)) { \
            panic_assertion(#expr, __FILE__, __FUNCTION__, __LINE__); \
        } \
    } while (0)

#define PANIC_ON_ERROR(error_code, fmt, ...) \
    panic_with_info(PANIC_GENERAL, __FILE__, __FUNCTION__, __LINE__, error_code, fmt, ##__VA_ARGS__)

// BUG detection macros
#define BUG() \
    panic("BUG detected at %s:%d in %s", __FILE__, __LINE__, __FUNCTION__)

#define BUG_ON(condition) \
    do { \
        if (unlikely(condition)) { \
            panic("BUG condition '%s' triggered at %s:%d in %s", \
                  #condition, __FILE__, __LINE__, __FUNCTION__); \
        } \
    } while (0)

// Warning macros (non-fatal)
#define WARN(condition, fmt, ...) \
    do { \
        if (unlikely(condition)) { \
            printf("[WARN] " fmt " at %s:%d in %s\n", \
                   ##__VA_ARGS__, __FILE__, __LINE__, __FUNCTION__); \
        } \
    } while (0)

#define WARN_ON(condition) \
    WARN(condition, "Warning condition '%s' triggered", #condition)

#define WARN_ONCE(condition, fmt, ...) \
    do { \
        static bool __warned = false; \
        if (unlikely(condition) && !__warned) { \
            __warned = true; \
            printf("[WARN] " fmt " at %s:%d in %s\n", \
                   ##__VA_ARGS__, __FILE__, __LINE__, __FUNCTION__); \
        } \
    } while (0)

// Utility macros
#define unlikely(x) __builtin_expect(!!(x), 0)
#define likely(x)   __builtin_expect(!!(x), 1)

// Panic reason codes
#define PANIC_REASON_UNKNOWN            0x0000
#define PANIC_REASON_OUT_OF_MEMORY      0x0001
#define PANIC_REASON_NULL_POINTER       0x0002
#define PANIC_REASON_INVALID_ADDRESS    0x0003
#define PANIC_REASON_STACK_OVERFLOW     0x0004
#define PANIC_REASON_DOUBLE_FAULT       0x0005
#define PANIC_REASON_TRIPLE_FAULT       0x0006
#define PANIC_REASON_CPU_EXCEPTION      0x0007
#define PANIC_REASON_HARDWARE_FAILURE   0x0008
#define PANIC_REASON_FILESYSTEM_ERROR   0x0009
#define PANIC_REASON_NETWORK_ERROR      0x000A
#define PANIC_REASON_SECURITY_VIOLATION 0x000B
#define PANIC_REASON_DEADLOCK          0x000C
#define PANIC_REASON_CORRUPTION        0x000D

// Panic configuration
struct panic_config {
    bool enable_stack_trace;    /**< Enable stack traces on panic */
    bool enable_register_dump;  /**< Enable register dumps */
    bool enable_memory_dump;    /**< Enable memory dumps */
    bool enable_crash_dump;     /**< Enable crash dump saving */
    bool reboot_on_panic;       /**< Reboot system on panic */
    uint32_t panic_timeout;     /**< Timeout before reboot (seconds) */
};

extern struct panic_config panic_config;

// Panic configuration functions
void set_panic_config(struct panic_config *config);
void get_panic_config(struct panic_config *config);
void enable_panic_feature(uint32_t feature);
void disable_panic_feature(uint32_t feature);

// Emergency functions
void emergency_halt(void);
void emergency_reboot(void);
void emergency_shutdown(void);

#endif /* __PANIC_H__ */ 