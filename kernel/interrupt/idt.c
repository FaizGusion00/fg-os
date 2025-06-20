/**
 * @file idt.c
 * @brief Interrupt Descriptor Table (IDT) implementation for FG-OS
 * 
 * This file implements the IDT management system, providing functions to set up
 * interrupt gates, register handlers, and manage interrupt statistics.
 * 
 * @author Faiz Nasir
 * @company FGCompany Official
 * @version 1.0.0
 * @date 2024
 * @copyright Copyright (c) 2024 FGCompany Official. All rights reserved.
 */

#include "idt.h"
#include "interrupt.h"
#include "../include/kernel.h"
#include "../arch/x86_64/arch.h"
#include <stddef.h>

// Global interrupt manager
static interrupt_manager_t g_interrupt_manager;

// Default exception names
static const char* exception_names[32] = {
    "Divide by Zero",
    "Debug Exception",
    "Non-Maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Reserved",
    "x87 FPU Error",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Virtualization Exception",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved"
};

/**
 * @brief Default exception handler
 * 
 * Handles unregistered exceptions with panic or logging
 * 
 * @param vector Exception vector number
 * @param error_code Error code (if applicable)
 * @param context CPU context at time of exception
 */
static void default_exception_handler(uint8_t vector, uint64_t error_code, struct cpu_state* context) {
    const char* name = (vector < 32) ? exception_names[vector] : "Unknown Exception";
    
    printf("\n[EXCEPTION] %s (Vector: 0x%02X)\n", name, vector);
    printf("Error Code: 0x%016llX\n", error_code);
    printf("RIP: 0x%016llX\n", context->rip);
    printf("RSP: 0x%016llX\n", context->rsp);
    printf("RFLAGS: 0x%016llX\n", context->rflags);
    
    // Update statistics
    g_interrupt_manager.stats.exceptions++;
    g_interrupt_manager.stats.total_interrupts++;
    g_interrupt_manager.stats.interrupt_frequency[vector]++;
    
    // For critical exceptions, panic the system
    if (vector == EXCEPTION_DOUBLE_FAULT || 
        vector == EXCEPTION_MACHINE_CHECK ||
        vector == EXCEPTION_GENERAL_PROTECTION) {
        panic("Critical exception occurred: %s", name);
    }
    
    printf("System continuing...\n");
}

/**
 * @brief Default interrupt handler
 * 
 * Handles unregistered interrupts
 * 
 * @param vector Interrupt vector number
 * @param error_code Error code (always 0 for interrupts)
 * @param context CPU context at time of interrupt
 */
static void default_interrupt_handler(uint8_t vector, uint64_t error_code, struct cpu_state* context) {
    // Update statistics
    g_interrupt_manager.stats.total_interrupts++;
    g_interrupt_manager.stats.interrupt_frequency[vector]++;
    
    if (vector >= IRQ_TIMER && vector <= IRQ_SECONDARY_ATA) {
        g_interrupt_manager.stats.hardware_interrupts++;
        // Send EOI for hardware interrupts
        pic_send_eoi(vector - IRQ_TIMER);
    } else if (vector >= INT_SYSCALL) {
        g_interrupt_manager.stats.software_interrupts++;
    } else {
        g_interrupt_manager.stats.spurious_interrupts++;
    }
    
    printf("[DEBUG] Unhandled interrupt: Vector 0x%02X\n", vector);
}

/**
 * @brief Set an IDT entry
 * 
 * @param vector Interrupt vector number (0-255)
 * @param handler Address of interrupt handler
 * @param selector Code segment selector
 * @param type_attr Type and attribute flags
 * @return 0 on success, negative error code on failure
 */
int idt_set_entry(uint8_t vector, uint64_t handler, uint16_t selector, uint8_t type_attr) {
    if (g_interrupt_manager.state == INTERRUPT_SYSTEM_DISABLED) {
        return -1; // System not initialized
    }
    
    idt_entry_t* entry = &g_interrupt_manager.idt[vector];
    
    // Split handler address into parts
    entry->offset_low  = (uint16_t)(handler & 0xFFFF);
    entry->offset_mid  = (uint16_t)((handler >> 16) & 0xFFFF);
    entry->offset_high = (uint32_t)((handler >> 32) & 0xFFFFFFFF);
    
    entry->selector    = selector;
    entry->ist         = 0;  // No IST for now
    entry->type_attr   = type_attr;
    entry->reserved    = 0;
    
    return 0;
}

/**
 * @brief Register an interrupt handler
 * 
 * @param vector Interrupt vector number
 * @param handler Interrupt handler function
 * @return 0 on success, negative error code on failure
 */
int idt_register_handler(uint8_t vector, interrupt_handler_t handler) {
    if (g_interrupt_manager.state == INTERRUPT_SYSTEM_DISABLED) {
        return -1;
    }
    
    if (handler == NULL) {
        return -2;
    }
    
    uint64_t flags = interrupts_disable();
    g_interrupt_manager.handlers[vector] = handler;
    interrupts_restore(flags);
    
    return 0;
}

/**
 * @brief Unregister an interrupt handler
 * 
 * @param vector Interrupt vector number
 * @return 0 on success, negative error code on failure
 */
int idt_unregister_handler(uint8_t vector) {
    if (g_interrupt_manager.state == INTERRUPT_SYSTEM_DISABLED) {
        return -1;
    }
    
    uint64_t flags = interrupts_disable();
    
    // Set to default handler based on vector type
    if (vector < 32) {
        g_interrupt_manager.handlers[vector] = default_exception_handler;
    } else {
        g_interrupt_manager.handlers[vector] = default_interrupt_handler;
    }
    
    interrupts_restore(flags);
    return 0;
}

/**
 * @brief Load the IDT into the processor
 */
void idt_load(void) {
    __asm__ volatile ("lidt %0" : : "m" (g_interrupt_manager.idt_ptr) : "memory");
}

/**
 * @brief Enable interrupts globally
 */
void interrupts_enable(void) {
    if (g_interrupt_manager.disabled_count > 0) {
        g_interrupt_manager.disabled_count--;
    }
    
    if (g_interrupt_manager.disabled_count == 0) {
        __asm__ volatile ("sti" : : : "memory");
    }
}

/**
 * @brief Disable interrupts globally
 * 
 * @return Previous interrupt state
 */
uint64_t interrupts_disable(void) {
    uint64_t flags;
    __asm__ volatile (
        "pushfq\n\t"
        "popq %0\n\t"
        "cli"
        : "=r" (flags)
        :
        : "memory"
    );
    
    g_interrupt_manager.disabled_count++;
    return flags;
}

/**
 * @brief Restore interrupt state
 * 
 * @param state Previous interrupt state
 */
void interrupts_restore(uint64_t state) {
    if (state & 0x200) { // IF flag was set
        interrupts_enable();
    }
}

/**
 * @brief Check if interrupts are enabled
 * 
 * @return true if interrupts are enabled
 */
bool interrupts_enabled(void) {
    uint64_t flags;
    __asm__ volatile (
        "pushfq\n\t"
        "popq %0"
        : "=r" (flags)
        :
        : "memory"
    );
    
    return (flags & 0x200) != 0; // Check IF flag
}

/**
 * @brief Initialize the Interrupt Descriptor Table
 * 
 * @return 0 on success, negative error code on failure
 */
int idt_init(void) {
    // Initialize interrupt manager
    memset(&g_interrupt_manager, 0, sizeof(interrupt_manager_t));
    g_interrupt_manager.state = INTERRUPT_SYSTEM_INITIALIZING;
    
    // Set up IDT pointer
    g_interrupt_manager.idt_ptr.limit = sizeof(g_interrupt_manager.idt) - 1;
    g_interrupt_manager.idt_ptr.base = (uint64_t)&g_interrupt_manager.idt;
    
    // Initialize all handlers to default
    for (int i = 0; i < IDT_ENTRIES; i++) {
        if (i < 32) {
            g_interrupt_manager.handlers[i] = default_exception_handler;
        } else {
            g_interrupt_manager.handlers[i] = default_interrupt_handler;
        }
    }
    
    // Set up exception handlers (assembly stubs)
    // Note: These would normally point to assembly stubs that save state
    // For now, we'll use placeholder addresses
    uint64_t kernel_code_selector = 0x08; // Assume kernel code segment
    
    // CPU Exceptions (0-31)
    for (int i = 0; i < 32; i++) {
        uint8_t type = IDT_GATE_INTERRUPT | IDT_DPL_KERNEL;
        idt_set_entry(i, (uint64_t)default_exception_handler, kernel_code_selector, type);
    }
    
    // Hardware Interrupts (32-47) - will be set up by interrupt_init()
    for (int i = 32; i < 48; i++) {
        uint8_t type = IDT_GATE_INTERRUPT | IDT_DPL_KERNEL;
        idt_set_entry(i, (uint64_t)default_interrupt_handler, kernel_code_selector, type);
    }
    
    // Software Interrupts (48-255)
    for (int i = 48; i < IDT_ENTRIES; i++) {
        uint8_t type = IDT_GATE_INTERRUPT | IDT_DPL_USER; // Allow user access
        idt_set_entry(i, (uint64_t)default_interrupt_handler, kernel_code_selector, type);
    }
    
    // Load IDT
    idt_load();
    
    g_interrupt_manager.state = INTERRUPT_SYSTEM_ENABLED;
    
    printf("[INFO] IDT initialized with %d entries\n", IDT_ENTRIES);
    return 0;
}

/**
 * @brief Get interrupt statistics
 * 
 * @return Pointer to interrupt statistics
 */
const interrupt_stats_t* idt_get_stats(void) {
    return &g_interrupt_manager.stats;
}

/**
 * @brief Reset interrupt statistics
 */
void idt_reset_stats(void) {
    memset(&g_interrupt_manager.stats, 0, sizeof(interrupt_stats_t));
}

/**
 * @brief Get interrupt system state
 * 
 * @return Current system state
 */
interrupt_system_state_t idt_get_state(void) {
    return g_interrupt_manager.state;
}

/**
 * @brief Dump IDT information for debugging
 * 
 * @param start_vector Starting vector to dump
 * @param count Number of vectors to dump
 */
void idt_dump(uint8_t start_vector, uint8_t count) {
    printf("\n=== IDT Dump (Vectors %d-%d) ===\n", start_vector, start_vector + count - 1);
    
    for (uint8_t i = 0; i < count && (start_vector + i) < IDT_ENTRIES; i++) {
        uint8_t vector = start_vector + i;
        idt_entry_t* entry = &g_interrupt_manager.idt[vector];
        
        uint64_t handler_addr = entry->offset_low | 
                               ((uint64_t)entry->offset_mid << 16) |
                               ((uint64_t)entry->offset_high << 32);
        
        printf("Vector %3d: Handler=0x%016llX, Selector=0x%04X, Type=0x%02X, Count=%u\n",
               vector, handler_addr, entry->selector, entry->type_attr,
               g_interrupt_manager.stats.interrupt_frequency[vector]);
    }
    
    printf("=== IDT Statistics ===\n");
    printf("Total Interrupts: %llu\n", g_interrupt_manager.stats.total_interrupts);
    printf("Exceptions: %llu\n", g_interrupt_manager.stats.exceptions);
    printf("Hardware Interrupts: %llu\n", g_interrupt_manager.stats.hardware_interrupts);
    printf("Software Interrupts: %llu\n", g_interrupt_manager.stats.software_interrupts);
    printf("Spurious Interrupts: %llu\n", g_interrupt_manager.stats.spurious_interrupts);
    printf("Nested Level: %u\n", g_interrupt_manager.nested_level);
    printf("System State: %d\n", g_interrupt_manager.state);
}

/**
 * @brief Common interrupt handler dispatcher
 * 
 * This is called by all assembly interrupt stubs
 * 
 * @param vector Interrupt vector number
 * @param error_code Error code (if applicable)
 * @param context CPU context
 */
void interrupt_common_handler(uint8_t vector, uint64_t error_code, struct cpu_state* context) {
    // Update nesting level
    g_interrupt_manager.nested_level++;
    
    // Call registered handler
    if (g_interrupt_manager.handlers[vector]) {
        g_interrupt_manager.handlers[vector](vector, error_code, context);
    }
    
    // Update nesting level
    g_interrupt_manager.nested_level--;
}

/**
 * @brief Get interrupt manager (for internal use)
 * 
 * @return Pointer to interrupt manager
 */
static interrupt_manager_t* idt_get_manager_internal(void) {
    return &g_interrupt_manager;
} 