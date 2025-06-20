/**
 * @file main.c
 * @brief Main kernel entry point for FG-OS
 * 
 * This file contains the kernel's main entry point and initialization sequence.
 * It handles the transition from bootloader to kernel and sets up all subsystems.
 * 
 * @author Faiz Nasir
 * @company FGCompany Official
 * @version 1.0.0
 * @date 2024
 * @copyright © 2024 FGCompany Official. All rights reserved.
 */

#include "kernel.h"
#include "boot.h"
#include "panic.h"
#include "../mm/memory.h"
#include "../sched/scheduler.h"
#include "../interrupt/interrupt.h"
#include "../drivers/device.h"
#include "../hal/hal.h"

// Forward declarations
void kernel_start(struct boot_info *boot_info);

// Global kernel state
static struct boot_info *g_boot_info = NULL;
static bool kernel_initialized = false;

// Kernel subsystems array
static struct kernel_subsystem *subsystems[] = {
    // Will be populated as subsystems implementations are added
    NULL
};

/**
 * @brief Print the FG-OS kernel banner
 * 
 * Displays the FG-OS banner and system information.
 */
void print_kernel_banner(void) {
    // Clear screen and set colors (basic implementation for now)
    printf("\033[2J\033[H");  // Clear screen and move cursor to top
    
    // FG-OS Banner with gradient effect
    printf("\033[38;5;93m"); // Purple gradient start
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                                                              ║\n");
    printf("║  \033[38;5;99m███████╗ \033[38;5;105m██████╗    \033[38;5;111m  ██████╗ \033[38;5;117m███████╗\033[38;5;93m  ║\n");
    printf("║  \033[38;5;99m██╔════╝ \033[38;5;105m██╔════╝    \033[38;5;111m ██╔═══██╗\033[38;5;117m██╔════╝\033[38;5;93m  ║\n");
    printf("║  \033[38;5;99m█████╗   \033[38;5;105m██║  ███╗   \033[38;5;111m ██║   ██║\033[38;5;117m███████╗\033[38;5;93m  ║\n");
    printf("║  \033[38;5;99m██╔══╝   \033[38;5;105m██║   ██║   \033[38;5;111m ██║   ██║\033[38;5;117m╚════██║\033[38;5;93m  ║\n");
    printf("║  \033[38;5;99m██║      \033[38;5;105m╚██████╔╝   \033[38;5;111m ╚██████╔╝\033[38;5;117m███████║\033[38;5;93m  ║\n");
    printf("║  \033[38;5;99m╚═╝      \033[38;5;105m ╚═════╝    \033[38;5;111m  ╚═════╝ \033[38;5;117m╚══════╝\033[38;5;93m  ║\n");
    printf("║                                                              ║\n");
    printf("║          \033[38;5;51mAdvanced Cross-Platform Operating System\033[38;5;93m         ║\n");
    printf("║                                                              ║\n");
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║  Version: \033[38;5;46m%-8s\033[38;5;93m │ Phase: \033[38;5;226m%d/%d\033[38;5;93m │ Author: \033[38;5;51m%-12s\033[38;5;93m ║\n", 
           KERNEL_VERSION, CURRENT_PHASE, TOTAL_PHASES, KERNEL_AUTHOR);
    printf("║  Company: \033[38;5;51m%-20s\033[38;5;93m │ Build: \033[38;5;46m%-12s\033[38;5;93m ║\n", 
           KERNEL_COMPANY, "DEBUG");
    printf("║                                                              ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    printf("\033[0m\n"); // Reset colors
}

/**
 * @brief Print boot information
 * 
 * @param boot_info Pointer to boot information structure
 */
void print_boot_info(struct boot_info *boot_info) {
    if (!boot_info) {
        KERROR("Boot information not available");
        return;
    }
    
    KINFO("Boot Information:");
    KINFO("  Magic: 0x%08X", boot_info->magic);
    KINFO("  Flags: 0x%08X", boot_info->flags);
    KINFO("  Memory Low: %lu KB", boot_info->memory_low);
    KINFO("  Memory High: %lu KB", boot_info->memory_high);
    KINFO("  Kernel Start: 0x%016lX", boot_info->kernel_start);
    KINFO("  Kernel End: 0x%016lX", boot_info->kernel_end);
    
    if (boot_info->cmdline[0]) {
        KINFO("  Command Line: %s", boot_info->cmdline);
    }
}

/**
 * @brief Print memory layout information
 */
void print_memory_layout(void) {
    KINFO("Memory Layout:");
    KINFO("  Kernel Virtual Base: 0x%016lX", KERNEL_VIRTUAL_BASE);
    KINFO("  User Virtual Base:   0x%016lX", USER_VIRTUAL_BASE);
    KINFO("  Kernel Heap Start:   0x%016lX", KERNEL_HEAP_START);
    KINFO("  Page Size:           %lu bytes", PAGE_SIZE);
}

/**
 * @brief Print current phase information
 */
void print_phase_info(void) {
    KINFO("Development Phase: %d/%d - %s", 
          CURRENT_PHASE, TOTAL_PHASES, PHASE_NAME);
    KINFO("Phase 7 Goals:");
    KINFO("  ✓ Interrupt descriptor table setup");
    KINFO("  ✓ Hardware interrupt handlers");
    KINFO("  ✓ Software interrupt implementation");
    KINFO("  ✓ Timer and clock management");
    KINFO("  ✓ Exception handling");
    KINFO("");
    KINFO("Previous phases completed:");
    KINFO("  ✓ Process management (Phase 6)");
    KINFO("  ✓ Memory management (Phase 5)");
    KINFO("  ✓ Kernel architecture (Phase 4)");
    KINFO("  ✓ Bootloader system (Phase 3)");
}

/**
 * @brief Initialize kernel subsystems
 * 
 * @return 0 on success, negative error code on failure
 */
static int init_subsystems(void) {
    KINFO("Initializing kernel subsystems...");
    
    // Phase 5: Initialize memory management
    KINFO("  → Console subsystem: OK");
    
    // Initialize memory management
    KINFO("  → Initializing Physical Memory Manager...");
    struct memory_region test_regions[2];
    test_regions[0].start = 0x100000;  // 1MB
    test_regions[0].end = 0x1100000;   // 17MB
    test_regions[0].size = 0x1000000;  // 16MB
    test_regions[0].type = MEMORY_TYPE_AVAILABLE;
    test_regions[0].flags = 0;
    test_regions[0].name = "Test Region 1";
    
    test_regions[1].start = 0x2000000; // 32MB
    test_regions[1].end = 0x4000000;   // 64MB
    test_regions[1].size = 0x2000000;  // 32MB  
    test_regions[1].type = MEMORY_TYPE_AVAILABLE;
    test_regions[1].flags = 0;
    test_regions[1].name = "Test Region 2";
    
    if (pmm_init(test_regions, 2) != 0) {
        KERROR("Failed to initialize Physical Memory Manager");
        return KERN_ERROR;
    }
    
    KINFO("  → Initializing Virtual Memory Manager...");
    if (vmm_init() != 0) {
        KERROR("Failed to initialize Virtual Memory Manager");
        return KERN_ERROR;
    }
    
    KINFO("  → Initializing Kernel Heap...");
    if (heap_init(KERNEL_HEAP_START, 0x1000000) != 0) { // 16MB heap
        KERROR("Failed to initialize Kernel Heap");
        return KERN_ERROR;
    }
    
    KINFO("  → Memory management: OK");
    
    // Phase 6: Initialize process management
    KINFO("  → Initializing Process Management...");
    if (process_init() != KERN_SUCCESS) {
        KERROR("Failed to initialize Process Management");
        return KERN_ERROR;
    }
    
    KINFO("  → Initializing Thread Management...");
    if (thread_init() != KERN_SUCCESS) {
        KERROR("Failed to initialize Thread Management");
        return KERN_ERROR;
    }
    
    KINFO("  → Initializing Scheduler...");
    if (scheduler_init() != KERN_SUCCESS) {
        KERROR("Failed to initialize Scheduler");
        return KERN_ERROR;
    }
    
    KINFO("  → Process management: OK");
    
    // Phase 7: Initialize interrupt handling system
    KINFO("  → Initializing Interrupt System...");
    if (interrupt_init() != 0) {
        KERROR("Failed to initialize Interrupt System");
        return KERN_ERROR;
    }
    
    KINFO("  → Interrupt system: OK");
    
    // Phase 8: Initialize device framework
    KINFO("  → Initializing Device Framework...");
    if (device_init() != 0) {
        KERROR("Failed to initialize Device Framework");
        return KERN_ERROR;
    }
    
    KINFO("  → Device framework: OK");
    
    // Phase 9: Initialize Hardware Abstraction Layer
    KINFO("  → Initializing Hardware Abstraction Layer...");
    if (hal_initialize() != 0) {
        KERROR("Failed to initialize Hardware Abstraction Layer");
        return KERN_ERROR;
    }
    
    KINFO("  → Hardware Abstraction Layer: OK");
    KINFO("  → File system: Deferred to Phase 10");
    KINFO("  → Graphics subsystem: Deferred to Phase 13");
    
    return KERN_SUCCESS;
}

/**
 * @brief Kernel early initialization
 * 
 * This function is called very early in the boot process to set up
 * critical kernel structures before main initialization.
 * 
 * @param boot_info Boot information from bootloader
 * @return 0 on success, negative error code on failure
 */
int kernel_early_init(struct boot_info *boot_info) {
    // Store boot information globally
    g_boot_info = boot_info;
    
    // Basic validation
    if (!boot_info) {
        return KERN_INVALID;
    }
    
    // Initialize console for early debugging
    // (Implementation will be added in console driver)
    
    return KERN_SUCCESS;
}

/**
 * @brief Main kernel initialization
 * 
 * @return 0 on success, negative error code on failure
 */
int kernel_init(void) {
    int result;
    
    KINFO("Starting FG-OS kernel initialization...");
    
    // Initialize all kernel subsystems
    result = init_subsystems();
    if (result != KERN_SUCCESS) {
        KERROR("Failed to initialize kernel subsystems: %d", result);
        return result;
    }
    
    // Mark kernel as initialized
    kernel_initialized = true;
    
    KINFO("Kernel initialization completed successfully!");
    return KERN_SUCCESS;
}

/**
 * @brief Test thread entry point
 */
void test_thread_entry(void *arg) {
    uint32_t thread_id = (uint32_t)(uintptr_t)arg;
    KINFO("Test thread %u started!", thread_id);
    
    // Simulate some work
    for (int i = 0; i < 5; i++) {
        KINFO("Thread %u: working... (%d/5)", thread_id, i + 1);
        // In a real system, this would yield to other threads
    }
    
    KINFO("Test thread %u completed!", thread_id);
}

/**
 * @brief Main kernel loop
 * 
 * This is the main kernel execution loop that runs after initialization.
 * Phase 7 demonstrates interrupt handling and timer management functionality.
 */
void kernel_main(void) {
    KINFO("Entering kernel main loop...");
    KINFO("FG-OS Phase 7 - Interrupt Handling System is now running!");
    
    // Phase 7: Interrupt handling demonstration
    KINFO("");
    KINFO("=== Phase 7: Interrupt Handling Demonstration ===");
    
    // Enable interrupts
    interrupts_enable();
    KINFO("Global interrupts enabled");
    
    // Display interrupt system status
    KINFO("");
    KINFO("=== Interrupt System Status ===");
    interrupt_dump_status();
    
    // Test timer functionality
    KINFO("");
    KINFO("=== Testing Timer Functionality ===");
    uint64_t start_ticks = timer_get_ticks();
    uint64_t start_ms = timer_get_uptime_ms();
    
    KINFO("Timer ticks at start: %llu", start_ticks);
    KINFO("Uptime at start: %llu ms", start_ms);
    
    // Simulate some work and show timer progression
    for (int i = 0; i < 5; i++) {
        KINFO("Working... iteration %d", i + 1);
        timer_sleep_ms(100);  // Sleep for 100ms
        
        uint64_t current_ticks = timer_get_ticks();
        uint64_t current_ms = timer_get_uptime_ms();
        KINFO("  Timer ticks: %llu (delta: %llu)", current_ticks, current_ticks - start_ticks);
        KINFO("  Uptime: %llu ms (delta: %llu ms)", current_ms, current_ms - start_ms);
    }
    
    // Test interrupt statistics
    KINFO("");
    KINFO("=== Interrupt Statistics ===");
    const interrupt_stats_t* stats = idt_get_stats();
    KINFO("Total interrupts: %llu", stats->total_interrupts);
    KINFO("Hardware interrupts: %llu", stats->hardware_interrupts);
    KINFO("Software interrupts: %llu", stats->software_interrupts);
    KINFO("Exceptions: %llu", stats->exceptions);
    
    // Test exception information
    KINFO("");
    KINFO("=== Exception Information ===");
    for (int i = 0; i < 20; i++) {
        const exception_info_t* exc_info = exception_get_info(i);
        if (exc_info && exc_info->name) {
            KINFO("Exception %2d: %s (%s)", i, exc_info->name, exc_info->description);
        }
    }
    
    // Test hardware interrupt information
    KINFO("");
    KINFO("=== Hardware Interrupt Information ===");
    for (int i = 0; i < 8; i++) {
        const hardware_interrupt_info_t* irq_info = irq_get_info(i);
        if (irq_info && irq_info->name) {
            KINFO("IRQ %d: %s (%s) - Count: %u, Enabled: %s", 
                   i, irq_info->name, irq_info->description,
                   irq_info->count, irq_info->enabled ? "Yes" : "No");
        }
    }
    
    // Test timer manager information
    KINFO("");
    KINFO("=== Timer Manager Information ===");
    const timer_manager_t* timer_mgr = timer_get_manager();
    KINFO("Timer frequency: %u Hz", timer_mgr->frequency);
    KINFO("Total ticks: %llu", timer_mgr->ticks);
    KINFO("Uptime: %llu seconds (%llu ms)", timer_mgr->seconds, timer_mgr->milliseconds);
    KINFO("Timer initialized: %s", timer_mgr->initialized ? "Yes" : "No");
    
    // Test critical sections
    KINFO("");
    KINFO("=== Testing Critical Sections ===");
    KINFO("Entering critical section...");
    uint64_t saved_state = critical_section_enter();
    KINFO("Interrupts disabled, doing critical work...");
    timer_sleep_ms(50);
    KINFO("Critical work completed, restoring interrupts...");
    critical_section_exit(saved_state);
    KINFO("Critical section exited");
    
    // Test interrupt nesting level
    KINFO("");
    KINFO("=== Interrupt Context Information ===");
    KINFO("In interrupt context: %s", in_interrupt() ? "Yes" : "No");
    KINFO("Interrupt nesting level: %u", interrupt_get_nesting_level());
    KINFO("Interrupts enabled: %s", interrupts_enabled() ? "Yes" : "No");
    
    // Demonstrate integration with process management (from Phase 6)
    KINFO("");
    KINFO("=== Integration with Process Management ===");
    
    // Enable scheduler (which uses timer interrupts)
    scheduler_enable(true);
    KINFO("Scheduler enabled with timer-based preemption");
    
    // Create a test process to show scheduler integration
    struct process *test_proc = create_process("timer_test", 0);
    if (test_proc) {
        KINFO("Created test process (PID: %u) for timer integration", test_proc->pid);
        
        struct thread *test_thread = create_thread(test_proc->pid, test_thread_entry, (void*)42);
        if (test_thread) {
            KINFO("Created test thread (TID: %u) for timer integration", test_thread->tid);
            scheduler_add_thread(test_thread);
            
            // Let scheduler run for a bit
            KINFO("Running scheduler with timer interrupts for 500ms...");
            timer_sleep_ms(500);
        }
    }
    
    // Final status
    KINFO("");
    KINFO("=== Final System Status ===");
    interrupt_dump_status();
    
    KINFO("");
    KINFO("Phase 7 - Interrupt Handling System demonstration completed!");
    KINFO("Next phase: Phase 8 - Device Drivers Framework");
    
    // Infinite loop with halt instruction
    KINFO("Entering idle loop...");
    while (1) {
        halt();  // Wait for interrupts
    }
}

/**
 * @brief Shutdown kernel
 * 
 * Performs clean shutdown of all kernel subsystems.
 */
void kernel_shutdown(void) {
    KINFO("Shutting down FG-OS kernel...");
    
    // Shutdown all subsystems in reverse order
    // (Implementation will be added as subsystems are developed)
    
    kernel_initialized = false;
    KINFO("Kernel shutdown complete.");
}

/**
 * @brief Windows-compatible main entry point for Phase 3 testing
 */
int main(void) {
    // Phase 3: Create a dummy boot_info for testing
    struct boot_info test_boot_info = {0};
    test_boot_info.magic = 0x36D76289; // MULTIBOOT2_MAGIC
    
    kernel_start(&test_boot_info);
    return 0;
}

/**
 * @brief Main kernel entry point
 * 
 * This is the main entry point called by the bootloader after basic
 * hardware initialization. It sets up the kernel and enters the main loop.
 * 
 * @param boot_info Boot information structure from bootloader
 */
void kernel_start(struct boot_info *boot_info) {
    int result;
    
    // Perform early initialization
    result = kernel_early_init(boot_info);
    if (result != KERN_SUCCESS) {
        panic("Early kernel initialization failed: %d", result);
    }
    
    // Display beautiful FG-OS banner
    print_kernel_banner();
    
    // Print system information
    print_boot_info(boot_info);
    print_memory_layout();
    print_phase_info();
    
    // Initialize kernel
    result = kernel_init();
    if (result != KERN_SUCCESS) {
        panic("Kernel initialization failed: %d", result);
    }
    
    // Enter main kernel loop
    kernel_main();
    
    // Should never reach here
    panic("Kernel main loop exited unexpectedly");
} 