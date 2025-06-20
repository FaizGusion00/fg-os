/**
 * @file interrupt.c
 * @brief Main interrupt handling implementation for FG-OS
 * 
 * This file implements the complete interrupt handling system including PIC
 * management, timer handling, exception processing, and hardware interrupt
 * management.
 * 
 * @author Faiz Nasir
 * @company FGCompany Official
 * @version 1.0.0
 * @date 2024
 * @copyright Copyright (c) 2024 FGCompany Official. All rights reserved.
 */

#include "interrupt.h"
#include "idt.h"
#include "../include/kernel.h"
#include "../arch/x86_64/arch.h"
#include "../sched/scheduler.h"

// Global variables
static timer_manager_t g_timer_manager;
static exception_info_t g_exceptions[32];
static hardware_interrupt_info_t g_hardware_interrupts[16];
static interrupt_context_t* g_current_context = NULL;
static interrupt_priority_t g_current_priority = INTERRUPT_PRIORITY_NORMAL;

// PIC I/O functions (simplified for demonstration)
static inline void outb(uint16_t port, uint8_t value) {
    // In a real kernel, this would be inline assembly
    // For demonstration, we'll just track the operation
    (void)port; (void)value;
}

static inline uint8_t inb(uint16_t port) {
    // In a real kernel, this would be inline assembly
    // For demonstration, we'll return 0
    (void)port;
    return 0;
}

/**
 * @brief Initialize exception information table
 */
static void init_exception_table(void) {
    const struct {
        uint8_t vector;
        const char* name;
        const char* description;
        bool has_error_code;
    } exception_data[] = {
        {0x00, "Divide Error", "Division by zero or overflow", false},
        {0x01, "Debug", "Debug exception", false},
        {0x02, "NMI", "Non-maskable interrupt", false},
        {0x03, "Breakpoint", "Breakpoint instruction", false},
        {0x04, "Overflow", "Overflow instruction", false},
        {0x05, "Bound Range", "BOUND range exceeded", false},
        {0x06, "Invalid Opcode", "Invalid or undefined opcode", false},
        {0x07, "Device Not Available", "FPU not available", false},
        {0x08, "Double Fault", "Double fault", true},
        {0x09, "Coprocessor Overrun", "Coprocessor segment overrun", false},
        {0x0A, "Invalid TSS", "Invalid Task State Segment", true},
        {0x0B, "Segment Not Present", "Segment not present", true},
        {0x0C, "Stack Fault", "Stack segment fault", true},
        {0x0D, "General Protection", "General protection fault", true},
        {0x0E, "Page Fault", "Page fault", true},
        {0x0F, "Reserved", "Intel reserved", false},
        {0x10, "FPU Error", "x87 FPU floating-point error", false},
        {0x11, "Alignment Check", "Alignment check", true},
        {0x12, "Machine Check", "Machine check", false},
        {0x13, "SIMD Exception", "SIMD floating-point exception", false}
    };
    
    memset(g_exceptions, 0, sizeof(g_exceptions));
    
    for (size_t i = 0; i < sizeof(exception_data) / sizeof(exception_data[0]); i++) {
        uint8_t vector = exception_data[i].vector;
        g_exceptions[vector].vector = vector;
        g_exceptions[vector].name = exception_data[i].name;
        g_exceptions[vector].description = exception_data[i].description;
        g_exceptions[vector].has_error_code = exception_data[i].has_error_code;
        g_exceptions[vector].count = 0;
    }
}

/**
 * @brief Initialize hardware interrupt information table
 */
static void init_hardware_interrupt_table(void) {
    const struct {
        uint8_t irq;
        const char* name;
        const char* description;
    } irq_data[] = {
        {0, "Timer", "System timer (PIT)"},
        {1, "Keyboard", "PS/2 Keyboard"},
        {2, "Cascade", "PIC cascade (never raised)"},
        {3, "COM2", "Serial port COM2"},
        {4, "COM1", "Serial port COM1"},
        {5, "LPT2", "Parallel port LPT2"},
        {6, "Floppy", "Floppy disk controller"},
        {7, "LPT1", "Parallel port LPT1"},
        {8, "RTC", "Real-time clock"},
        {9, "Free", "Free IRQ (ACPI/SCSI/NIC)"},
        {10, "Free", "Free IRQ (SCSI/NIC)"},
        {11, "Free", "Free IRQ (SCSI/NIC)"},
        {12, "Mouse", "PS/2 Mouse"},
        {13, "Coprocessor", "Math coprocessor"},
        {14, "Primary ATA", "Primary ATA hard disk"},
        {15, "Secondary ATA", "Secondary ATA hard disk"}
    };
    
    memset(g_hardware_interrupts, 0, sizeof(g_hardware_interrupts));
    
    for (size_t i = 0; i < 16; i++) {
        g_hardware_interrupts[i].irq = irq_data[i].irq;
        g_hardware_interrupts[i].name = irq_data[i].name;
        g_hardware_interrupts[i].description = irq_data[i].description;
        g_hardware_interrupts[i].vector = IRQ_TIMER + i;
        g_hardware_interrupts[i].count = 0;
        g_hardware_interrupts[i].enabled = false;
        g_hardware_interrupts[i].last_time = 0;
        g_hardware_interrupts[i].total_time = 0;
    }
}

/**
 * @brief Timer interrupt handler
 */
static void timer_interrupt_handler(uint8_t vector, uint64_t error_code, struct cpu_state* context) {
    (void)vector; (void)error_code; (void)context;
    
    g_timer_manager.ticks++;
    g_timer_manager.milliseconds = g_timer_manager.ticks * (1000 / g_timer_manager.frequency);
    g_timer_manager.seconds = g_timer_manager.milliseconds / 1000;
    
    // Update hardware interrupt statistics
    g_hardware_interrupts[0].count++;
    g_hardware_interrupts[0].last_time = g_timer_manager.ticks;
    
    // Send EOI to PIC
    pic_send_eoi(0);
    
    // Call scheduler for preemptive multitasking
    if (scheduler_is_enabled()) {
        scheduler_tick();
    }
}

/**
 * @brief Keyboard interrupt handler
 */
static void keyboard_interrupt_handler(uint8_t vector, uint64_t error_code, struct cpu_state* context) {
    (void)vector; (void)error_code; (void)context;
    
    // Read keyboard scancode (simplified)
    uint8_t scancode = inb(0x60);
    
    // Update statistics
    g_hardware_interrupts[1].count++;
    g_hardware_interrupts[1].last_time = g_timer_manager.ticks;
    
    // For now, just acknowledge the interrupt
    printf("[DEBUG] Keyboard interrupt: scancode 0x%02X\n", scancode);
    
    // Send EOI to PIC
    pic_send_eoi(1);
}

/**
 * @brief Page fault exception handler
 */
static void page_fault_handler(uint8_t vector, uint64_t error_code, struct cpu_state* context) {
    uint64_t fault_address;
    
    // Get fault address from CR2
    __asm__ volatile ("mov %%cr2, %0" : "=r" (fault_address));
    
    g_exceptions[EXCEPTION_PAGE_FAULT].count++;
    
    printf("\n[PAGE FAULT] Address: 0x%016llX\n", fault_address);
    printf("Error Code: 0x%016llX (", error_code);
    
    if (error_code & PAGE_FAULT_PRESENT) printf("Protection ");
    else printf("Not Present ");
    
    if (error_code & PAGE_FAULT_WRITE) printf("Write ");
    else printf("Read ");
    
    if (error_code & PAGE_FAULT_USER) printf("User ");
    else printf("Supervisor ");
    
    if (error_code & PAGE_FAULT_RESERVED) printf("Reserved ");
    if (error_code & PAGE_FAULT_INSTRUCTION) printf("Instruction ");
    
    printf(")\n");
    printf("RIP: 0x%016llX\n", context->rip);
    
    // For now, just continue (in a real kernel, this would handle the fault)
    printf("Page fault handled (continuing)\n");
}

/**
 * @brief General protection fault handler
 */
static void gpf_handler(uint8_t vector, uint64_t error_code, struct cpu_state* context) {
    g_exceptions[EXCEPTION_GENERAL_PROTECTION].count++;
    
    printf("\n[GENERAL PROTECTION FAULT]\n");
    printf("Error Code: 0x%016llX\n", error_code);
    printf("RIP: 0x%016llX\n", context->rip);
    printf("CS: 0x%016llX\n", context->cs);
    printf("RFLAGS: 0x%016llX\n", context->rflags);
    
    if (error_code != 0) {
        printf("Segment Selector: 0x%04llX\n", error_code & 0xFFFF);
        printf("Table: %s\n", (error_code & 0x04) ? "LDT" : "GDT");
        printf("External: %s\n", (error_code & 0x01) ? "Yes" : "No");
    }
    
    panic("General Protection Fault - System Halted");
}

/**
 * @brief Initialize the Programmable Interrupt Controller
 */
int pic_init(void) {
    printf("[INFO] Initializing PIC...\n");
    
    // Save current masks
    uint8_t master_mask = inb(PIC1_DATA);
    uint8_t slave_mask = inb(PIC2_DATA);
    
    // Initialize master PIC
    outb(PIC1_COMMAND, PIC_ICW1_INIT | PIC_ICW1_EDGE);  // ICW1
    outb(PIC1_DATA, IRQ_TIMER);                         // ICW2: Master offset
    outb(PIC1_DATA, 0x04);                              // ICW3: Slave at IRQ2
    outb(PIC1_DATA, 0x01);                              // ICW4: 8086 mode
    
    // Initialize slave PIC
    outb(PIC2_COMMAND, PIC_ICW1_INIT | PIC_ICW1_EDGE);  // ICW1
    outb(PIC2_DATA, IRQ_RTC);                           // ICW2: Slave offset
    outb(PIC2_DATA, 0x02);                              // ICW3: Cascade identity
    outb(PIC2_DATA, 0x01);                              // ICW4: 8086 mode
    
    // Restore masks (disable all interrupts initially)
    outb(PIC1_DATA, 0xFF);
    outb(PIC2_DATA, 0xFF);
    
    printf("[INFO] PIC initialized and remapped\n");
    return 0;
}

/**
 * @brief Send End of Interrupt to PIC
 */
void pic_send_eoi(uint8_t irq) {
    if (irq >= 8) {
        outb(PIC2_COMMAND, PIC_EOI);  // Send EOI to slave
    }
    outb(PIC1_COMMAND, PIC_EOI);      // Send EOI to master
}

/**
 * @brief Enable a specific IRQ
 */
void pic_enable_irq(uint8_t irq) {
    if (irq >= 16) return;
    
    uint16_t port;
    uint8_t mask;
    
    if (irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }
    
    mask = inb(port) & ~(1 << irq);
    outb(port, mask);
    
    g_hardware_interrupts[irq].enabled = true;
}

/**
 * @brief Disable a specific IRQ
 */
void pic_disable_irq(uint8_t irq) {
    if (irq >= 16) return;
    
    uint16_t port;
    uint8_t mask;
    
    if (irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }
    
    mask = inb(port) | (1 << irq);
    outb(port, mask);
    
    g_hardware_interrupts[irq].enabled = false;
}

/**
 * @brief Get IRQ mask from PIC
 */
uint16_t pic_get_irq_mask(void) {
    uint8_t master = inb(PIC1_DATA);
    uint8_t slave = inb(PIC2_DATA);
    return (slave << 8) | master;
}

/**
 * @brief Set IRQ mask for PIC
 */
void pic_set_irq_mask(uint16_t mask) {
    outb(PIC1_DATA, mask & 0xFF);
    outb(PIC2_DATA, (mask >> 8) & 0xFF);
}

/**
 * @brief Initialize timer interrupt
 */
int timer_init(uint32_t frequency) {
    printf("[INFO] Initializing timer at %d Hz...\n", frequency);
    
    g_timer_manager.frequency = frequency;
    g_timer_manager.ticks = 0;
    g_timer_manager.milliseconds = 0;
    g_timer_manager.seconds = 0;
    g_timer_manager.last_tick_time = 0;
    g_timer_manager.tick_overruns = 0;
    g_timer_manager.initialized = true;
    
    // Calculate timer divisor
    uint32_t divisor = TIMER_DIVISOR / frequency;
    
    // Set PIT to mode 3 (square wave generator)
    outb(0x43, 0x36);  // Command port
    outb(0x40, divisor & 0xFF);         // Low byte
    outb(0x40, (divisor >> 8) & 0xFF);  // High byte
    
    // Register timer interrupt handler
    idt_register_handler(IRQ_TIMER, timer_interrupt_handler);
    
    // Enable timer IRQ
    pic_enable_irq(0);
    
    printf("[INFO] Timer initialized successfully\n");
    return 0;
}

/**
 * @brief Initialize the complete interrupt handling system
 */
int interrupt_init(void) {
    printf("[INFO] Initializing interrupt handling system...\n");
    
    // Initialize IDT first
    int result = idt_init();
    if (result != 0) {
        printf("[ERROR] Failed to initialize IDT: %d\n", result);
        return result;
    }
    
    // Initialize exception and interrupt tables
    init_exception_table();
    init_hardware_interrupt_table();
    
    // Initialize PIC
    result = pic_init();
    if (result != 0) {
        printf("[ERROR] Failed to initialize PIC: %d\n", result);
        return result;
    }
    
    // Register critical exception handlers
    idt_register_handler(EXCEPTION_PAGE_FAULT, page_fault_handler);
    idt_register_handler(EXCEPTION_GENERAL_PROTECTION, gpf_handler);
    idt_register_handler(IRQ_KEYBOARD, keyboard_interrupt_handler);
    
    // Initialize timer (1000 Hz = 1ms intervals)
    result = timer_init(TIMER_FREQUENCY);
    if (result != 0) {
        printf("[ERROR] Failed to initialize timer: %d\n", result);
        return result;
    }
    
    printf("[INFO] Interrupt system initialized successfully\n");
    printf("[INFO]   ΓåÆ IDT: %d entries configured\n", IDT_ENTRIES);
    printf("[INFO]   ΓåÆ PIC: Remapped and initialized\n");
    printf("[INFO]   ΓåÆ Timer: %d Hz frequency\n", TIMER_FREQUENCY);
    printf("[INFO]   ΓåÆ Exception handlers: Registered\n");
    
    return 0;
}

/**
 * @brief Shutdown the interrupt handling system
 */
void interrupt_shutdown(void) {
    printf("[INFO] Shutting down interrupt system...\n");
    
    // Disable all interrupts
    interrupts_disable();
    
    // Disable all IRQs
    pic_set_irq_mask(0xFFFF);
    
    g_timer_manager.initialized = false;
    
    printf("[INFO] Interrupt system shutdown complete\n");
}

/**
 * @brief Get current timer ticks
 */
uint64_t timer_get_ticks(void) {
    return g_timer_manager.ticks;
}

/**
 * @brief Get current uptime in milliseconds
 */
uint64_t timer_get_uptime_ms(void) {
    return g_timer_manager.milliseconds;
}

/**
 * @brief Get current uptime in seconds
 */
uint64_t timer_get_uptime_sec(void) {
    return g_timer_manager.seconds;
}

/**
 * @brief Sleep for specified milliseconds
 */
void timer_sleep_ms(uint32_t ms) {
    uint64_t start = timer_get_uptime_ms();
    while ((timer_get_uptime_ms() - start) < ms) {
        // Busy wait - in a real kernel, this would yield to scheduler
        __asm__ volatile ("pause");
    }
}

/**
 * @brief Get exception information
 */
const exception_info_t* exception_get_info(uint8_t exception) {
    if (exception >= 32) return NULL;
    return &g_exceptions[exception];
}

/**
 * @brief Get hardware interrupt information
 */
const hardware_interrupt_info_t* irq_get_info(uint8_t irq) {
    if (irq >= 16) return NULL;
    return &g_hardware_interrupts[irq];
}

/**
 * @brief Get timer manager information
 */
const timer_manager_t* timer_get_manager(void) {
    return &g_timer_manager;
}

/**
 * @brief Enter critical section
 */
uint64_t critical_section_enter(void) {
    return interrupts_disable();
}

/**
 * @brief Exit critical section
 */
void critical_section_exit(uint64_t state) {
    interrupts_restore(state);
}

/**
 * @brief Check if we're in an interrupt
 */
bool in_interrupt(void) {
    const interrupt_stats_t* stats = idt_get_stats();
    return stats->nested_interrupts > 0;
}

/**
 * @brief Get current interrupt nesting level
 */
uint32_t interrupt_get_nesting_level(void) {
    const interrupt_stats_t* stats = idt_get_stats();
    return (uint32_t)stats->nested_interrupts;
}

/**
 * @brief Dump interrupt system status
 */
void interrupt_dump_status(void) {
    const interrupt_stats_t* stats = idt_get_stats();
    
    printf("\n=== Interrupt System Status ===\n");
    printf("System State: %d\n", idt_get_state());
    printf("Interrupts Enabled: %s\n", interrupts_enabled() ? "Yes" : "No");
    printf("Nesting Level: %u\n", interrupt_get_nesting_level());
    printf("Current Priority: %d\n", g_current_priority);
    
    printf("\n=== Statistics ===\n");
    printf("Total Interrupts: %llu\n", stats->total_interrupts);
    printf("Exceptions: %llu\n", stats->exceptions);
    printf("Hardware Interrupts: %llu\n", stats->hardware_interrupts);
    printf("Software Interrupts: %llu\n", stats->software_interrupts);
    printf("Spurious Interrupts: %llu\n", stats->spurious_interrupts);
    
    printf("\n=== Timer Information ===\n");
    printf("Ticks: %llu\n", g_timer_manager.ticks);
    printf("Uptime: %llu ms (%llu seconds)\n", 
           g_timer_manager.milliseconds, g_timer_manager.seconds);
    printf("Frequency: %u Hz\n", g_timer_manager.frequency);
    
    printf("\n=== Hardware Interrupts ===\n");
    for (int i = 0; i < 16; i++) {
        if (g_hardware_interrupts[i].count > 0 || g_hardware_interrupts[i].enabled) {
            printf("IRQ %2d (%s): %u interrupts, %s\n", 
                   i, g_hardware_interrupts[i].name, 
                   g_hardware_interrupts[i].count,
                   g_hardware_interrupts[i].enabled ? "Enabled" : "Disabled");
        }
    }
    
    printf("\n=== Recent Exceptions ===\n");
    for (int i = 0; i < 32; i++) {
        if (g_exceptions[i].count > 0) {
            printf("Exception %2d (%s): %u occurrences\n", 
                   i, g_exceptions[i].name, g_exceptions[i].count);
        }
    }
}

/**
 * @brief Register exception handler
 */
int exception_register_handler(uint8_t exception, interrupt_handler_t handler) {
    if (exception >= 32) return -1;
    return idt_register_handler(exception, handler);
}

/**
 * @brief Register hardware interrupt handler
 */
int irq_register_handler(uint8_t irq, interrupt_handler_t handler) {
    if (irq >= 16) return -1;
    return idt_register_handler(IRQ_TIMER + irq, handler);
}

/**
 * @brief Unregister hardware interrupt handler
 */
int irq_unregister_handler(uint8_t irq) {
    if (irq >= 16) return -1;
    return idt_unregister_handler(IRQ_TIMER + irq);
} 