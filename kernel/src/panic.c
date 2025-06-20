/**
 * @file panic.c
 * @brief Kernel panic implementation for FG-OS
 * 
 * This file implements the kernel panic system including error handling,
 * stack traces, and system halt procedures for Phase 2.
 * 
 * @author Faiz Nasir
 * @company FGCompany Official
 * @version 1.0.0
 * @date 2024
 * @copyright © 2024 FGCompany Official. All rights reserved.
 */

#include "kernel.h"
#include "panic.h"
#include "boot.h"

// Global panic configuration
struct panic_config panic_config = {
    .enable_stack_trace = true,
    .enable_register_dump = true,
    .enable_memory_dump = false,
    .enable_crash_dump = false,
    .reboot_on_panic = false,
    .panic_timeout = 10
};

// Panic state
static bool in_panic = false;
static uint32_t panic_count = 0;

/**
 * @brief Simple printf implementation for panic situations
 * 
 * Basic printf for when the normal console might not be available.
 * This is a minimal implementation for Phase 2.
 */
static void panic_printf(const char *fmt, ...) {
    // For Phase 2, we'll use a very basic implementation
    // Later phases will implement proper console handling
    
    // TODO: Implement basic VGA text mode output
    // For now, this is a placeholder that would output to serial/console
    
    // In a real implementation, this would write directly to VGA memory
    // or serial port for debugging
}

/**
 * @brief Print panic banner with gradient colors
 */
static void print_panic_banner(void) {
    panic_printf("\033[91m"); // Bright red
    panic_printf("╔══════════════════════════════════════════════════════════════╗\n");
    panic_printf("║                                                              ║\n");
    panic_printf("║   \033[95m██████╗  \033[96m █████╗ \033[97m███╗   ██╗\033[93m██╗ \033[92m██████╗\033[91m        ║\n");
    panic_printf("║   \033[95m██╔══██╗\033[96m██╔══██╗\033[97m████╗  ██║\033[93m██║\033[92m██╔════╝\033[91m        ║\n");
    panic_printf("║   \033[95m██████╔╝\033[96m███████║\033[97m██╔██╗ ██║\033[93m██║\033[92m██║\033[91m             ║\n");
    panic_printf("║   \033[95m██╔═══╝ \033[96m██╔══██║\033[97m██║╚██╗██║\033[93m██║\033[92m██║\033[91m             ║\n");
    panic_printf("║   \033[95m██║     \033[96m██║  ██║\033[97m██║ ╚████║\033[93m██║\033[92m╚██████╗\033[91m        ║\n");
    panic_printf("║   \033[95m╚═╝     \033[96m╚═╝  ╚═╝\033[97m╚═╝  ╚═══╝\033[93m╚═╝\033[92m ╚═════╝\033[91m        ║\n");
    panic_printf("║                                                              ║\n");
    panic_printf("║                    \033[97mKERNEL PANIC DETECTED\033[91m                    ║\n");
    panic_printf("║                                                              ║\n");
    panic_printf("╚══════════════════════════════════════════════════════════════╝\n");
    panic_printf("\033[0m\n"); // Reset colors
}

/**
 * @brief Get panic type name
 */
static const char *get_panic_type_name(enum panic_type type) {
    switch (type) {
        case PANIC_GENERAL:         return "General";
        case PANIC_MEMORY:          return "Memory";
        case PANIC_HARDWARE:        return "Hardware";
        case PANIC_FILESYSTEM:      return "Filesystem";
        case PANIC_NETWORK:         return "Network";
        case PANIC_SECURITY:        return "Security";
        case PANIC_ASSERTION:       return "Assertion";
        case PANIC_STACKOVERFLOW:   return "Stack Overflow";
        case PANIC_DEADLOCK:        return "Deadlock";
        case PANIC_CORRUPTION:      return "Corruption";
        default:                    return "Unknown";
    }
}

/**
 * @brief Print basic stack trace (Phase 2 implementation)
 */
void print_stack_trace(void) {
    if (!panic_config.enable_stack_trace) {
        return;
    }
    
    panic_printf("\n[STACK TRACE]\n");
    panic_printf("Stack trace functionality will be implemented in Phase 8\n");
    panic_printf("(Interrupt Handling System)\n");
    
    // Phase 2: Basic stack pointer information
    uint64_t rsp;
    __asm__ volatile("movq %%rsp, %0" : "=r"(rsp));
    panic_printf("Current stack pointer: 0x%016lX\n", rsp);
}

/**
 * @brief Print system information during panic
 */
void print_system_info(void) {
    panic_printf("\n[SYSTEM INFORMATION]\n");
    panic_printf("Kernel: %s v%s\n", KERNEL_NAME, KERNEL_VERSION);
    panic_printf("Author: %s\n", KERNEL_AUTHOR);
    panic_printf("Company: %s\n", KERNEL_COMPANY);
    panic_printf("Phase: %d/%d - %s\n", CURRENT_PHASE, TOTAL_PHASES, PHASE_NAME);
    panic_printf("Panic Count: %u\n", panic_count);
    
    // Basic CPU information
    panic_printf("Architecture: x86_64\n");
    
    // Phase 2: Basic memory information will be added in Phase 6
    panic_printf("Memory Management: Not yet implemented (Phase 6)\n");
    panic_printf("Process Management: Not yet implemented (Phase 7)\n");
}

/**
 * @brief Emergency halt - stops all CPUs
 */
void emergency_halt(void) {
    // Disable interrupts
    cli();
    
    panic_printf("System halted. Please restart your computer.\n");
    
    // Halt forever
    while (1) {
        halt();
    }
}

/**
 * @brief Emergency reboot (Phase 2 - basic implementation)
 */
void emergency_reboot(void) {
    panic_printf("Rebooting system...\n");
    
    // Phase 2: Basic reboot implementation
    // Later phases will implement proper ACPI reboot
    cli();
    
    // Try keyboard controller reboot
    // This is a legacy method that works on most systems
    uint8_t good = 0x02;
    while (good & 0x02) {
        // Read keyboard controller status
        __asm__ volatile("inb $0x64, %0" : "=a"(good));
    }
    
    // Send reboot command to keyboard controller
    __asm__ volatile("outb %%al, $0x64" : : "a"(0xFE));
    
    // If that didn't work, halt
    emergency_halt();
}

/**
 * @brief Main panic implementation
 */
__noreturn void panic(const char *fmt, ...) {
    // Prevent recursive panics
    if (in_panic) {
        panic_printf("Double panic detected! Halting system...\n");
        emergency_halt();
    }
    
    in_panic = true;
    panic_count++;
    
    // Disable interrupts immediately
    cli();
    
    // Print panic banner
    print_panic_banner();
    
    // Print panic message
    panic_printf("\n[PANIC MESSAGE]\n");
    // TODO: Implement proper varargs handling
    panic_printf("Panic: %s\n", fmt);
    
    // Print system information
    print_system_info();
    
    // Print stack trace if enabled
    print_stack_trace();
    
    panic_printf("\n[RECOVERY OPTIONS]\n");
    if (panic_config.reboot_on_panic) {
        panic_printf("System will reboot in %u seconds...\n", panic_config.panic_timeout);
        
        // Simple delay loop (very approximate)
        for (uint64_t i = 0; i < panic_config.panic_timeout * 1000000000UL; i++) {
            __asm__ volatile("nop");
        }
        
        emergency_reboot();
    } else {
        panic_printf("Manual restart required.\n");
        emergency_halt();
    }
}

/**
 * @brief Panic with specific type and location information
 */
__noreturn void panic_with_info(enum panic_type type, const char *file,
                                const char *function, uint32_t line,
                                uint64_t error_code, const char *fmt, ...) {
    // Prevent recursive panics
    if (in_panic) {
        panic_printf("Double panic detected! Halting system...\n");
        emergency_halt();
    }
    
    in_panic = true;
    panic_count++;
    
    // Disable interrupts immediately
    cli();
    
    // Print panic banner
    print_panic_banner();
    
    // Print detailed panic information
    panic_printf("\n[PANIC DETAILS]\n");
    panic_printf("Type: %s\n", get_panic_type_name(type));
    panic_printf("File: %s\n", file ? file : "unknown");
    panic_printf("Function: %s\n", function ? function : "unknown");
    panic_printf("Line: %u\n", line);
    if (error_code != 0) {
        panic_printf("Error Code: 0x%016lX\n", error_code);
    }
    panic_printf("Message: %s\n", fmt);
    
    // Print system information
    print_system_info();
    
    // Print stack trace if enabled
    print_stack_trace();
    
    panic_printf("\n[RECOVERY OPTIONS]\n");
    if (panic_config.reboot_on_panic) {
        panic_printf("System will reboot in %u seconds...\n", panic_config.panic_timeout);
        
        // Simple delay loop
        for (uint64_t i = 0; i < panic_config.panic_timeout * 1000000000UL; i++) {
            __asm__ volatile("nop");
        }
        
        emergency_reboot();
    } else {
        panic_printf("Manual restart required.\n");
        emergency_halt();
    }
}

/**
 * @brief Assertion panic
 */
__noreturn void panic_assertion(const char *expr, const char *file,
                                const char *function, uint32_t line) {
    panic_with_info(PANIC_ASSERTION, file, function, line, 0,
                    "Assertion failed: %s", expr);
}

/**
 * @brief Set panic configuration
 */
void set_panic_config(struct panic_config *config) {
    if (config) {
        panic_config = *config;
    }
}

/**
 * @brief Get current panic configuration
 */
void get_panic_config(struct panic_config *config) {
    if (config) {
        *config = panic_config;
    }
}

/**
 * @brief Phase 2 panic system initialization
 */
void panic_init(void) {
    // Initialize panic system for Phase 2
    in_panic = false;
    panic_count = 0;
    
    // Set default configuration for development
    panic_config.enable_stack_trace = true;
    panic_config.enable_register_dump = true;
    panic_config.enable_memory_dump = false;
    panic_config.enable_crash_dump = false;
    panic_config.reboot_on_panic = false;  // Don't auto-reboot during development
    panic_config.panic_timeout = 10;
}

// Stub implementations for functions declared in header but not yet implemented

void print_stack_trace_from(void *frame_ptr) {
    panic_printf("Stack trace from specific frame not yet implemented (Phase 8)\n");
}

int capture_stack_trace(void **buffer, int max_frames) {
    return 0; // Not implemented in Phase 2
}

void print_registers(uint64_t *registers) {
    panic_printf("Register dump not yet implemented (Phase 8)\n");
}

void register_panic_handler(panic_handler_t handler) {
    // Panic handler registration will be implemented in later phases
}

void unregister_panic_handler(panic_handler_t handler) {
    // Panic handler unregistration will be implemented in later phases
}

void save_system_state(struct panic_info *info) {
    // System state saving will be implemented in later phases
}

void print_memory_usage(void) {
    panic_printf("Memory usage information not available (Phase 6)\n");
}

void print_process_info(void) {
    panic_printf("Process information not available (Phase 7)\n");
}

int try_recover_from_error(enum panic_type type, uint64_t error_code) {
    return -1; // No recovery implemented in Phase 2
}

void set_panic_on_oops(bool enable) {
    // OOPS handling will be implemented in later phases
}

void dump_memory(void *addr, size_t size) {
    panic_printf("Memory dump not yet implemented\n");
}

void dump_kernel_log(void) {
    panic_printf("Kernel log dump not yet implemented\n");
}

void save_crash_dump(struct panic_info *info) {
    panic_printf("Crash dump saving not yet implemented\n");
}

void enable_panic_feature(uint32_t feature) {
    // Feature management will be implemented in later phases
}

void disable_panic_feature(uint32_t feature) {
    // Feature management will be implemented in later phases
}

void emergency_shutdown(void) {
    panic_printf("Emergency shutdown not yet implemented\n");
    emergency_halt();
} 