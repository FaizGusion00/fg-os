/**
 * @file interrupt.h
 * @brief Main interrupt handling interface for FG-OS
 * 
 * This file provides the main interface for interrupt handling, including
 * hardware interrupts, exceptions, software interrupts, and timer management.
 * 
 * @author Faiz Nasir
 * @company FGCompany Official
 * @version 1.0.0
 * @date 2024
 * @copyright Copyright (c) 2024 FGCompany Official. All rights reserved.
 */

#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__

#include <types.h>
#include "idt.h"
#include "../arch/x86_64/arch.h"

/**
 * @brief PIC (Programmable Interrupt Controller) ports
 */
#define PIC1_COMMAND    0x20    /**< Master PIC command port */
#define PIC1_DATA       0x21    /**< Master PIC data port */
#define PIC2_COMMAND    0xA0    /**< Slave PIC command port */
#define PIC2_DATA       0xA1    /**< Slave PIC data port */

/**
 * @brief PIC commands
 */
#define PIC_EOI         0x20    /**< End of Interrupt */
#define PIC_ICW1_INIT   0x11    /**< ICW1: Initialize + ICW4 needed */
#define PIC_ICW1_SINGLE 0x02    /**< ICW1: Single mode (no slave) */
#define PIC_ICW1_LEVEL  0x08    /**< ICW1: Level triggered mode */
#define PIC_ICW1_EDGE   0x00    /**< ICW1: Edge triggered mode */

/**
 * @brief Timer constants
 */
#define TIMER_FREQUENCY     1000        /**< Timer frequency in Hz (1ms) */
#define TIMER_DIVISOR       1193180     /**< PIT base frequency */
#define TIMER_RELOAD_VALUE  (TIMER_DIVISOR / TIMER_FREQUENCY)

/**
 * @brief Exception error codes
 */
#define EXCEPTION_ERROR_CODE_PRESENT    0x01    /**< Exception has error code */
#define EXCEPTION_ERROR_CODE_ABSENT     0x00    /**< Exception has no error code */

/**
 * @brief Page fault error code bits
 */
#define PAGE_FAULT_PRESENT      0x01    /**< Page present */
#define PAGE_FAULT_WRITE        0x02    /**< Write access */
#define PAGE_FAULT_USER         0x04    /**< User mode access */
#define PAGE_FAULT_RESERVED     0x08    /**< Reserved bit violation */
#define PAGE_FAULT_INSTRUCTION  0x10    /**< Instruction fetch */

/**
 * @brief Exception information structure
 */
typedef struct {
    const char*     name;           /**< Exception name */
    const char*     description;    /**< Exception description */
    bool            has_error_code; /**< Whether exception provides error code */
    uint8_t         vector;         /**< Exception vector number */
    uint32_t        count;          /**< Exception occurrence count */
} exception_info_t;

/**
 * @brief Hardware interrupt information
 */
typedef struct {
    const char*     name;           /**< Interrupt name */
    const char*     description;    /**< Interrupt description */
    uint8_t         irq;            /**< IRQ number */
    uint8_t         vector;         /**< Interrupt vector */
    uint32_t        count;          /**< Interrupt count */
    bool            enabled;        /**< Interrupt enabled state */
    uint64_t        last_time;      /**< Last interrupt time */
    uint64_t        total_time;     /**< Total processing time */
} hardware_interrupt_info_t;

/**
 * @brief Timer management structure
 */
typedef struct {
    uint64_t        ticks;              /**< Total timer ticks */
    uint64_t        milliseconds;       /**< Milliseconds since boot */
    uint64_t        seconds;            /**< Seconds since boot */
    uint32_t        frequency;          /**< Timer frequency in Hz */
    bool            initialized;        /**< Timer initialization status */
    uint64_t        last_tick_time;     /**< Last tick timestamp */
    uint32_t        tick_overruns;      /**< Tick overrun count */
} timer_manager_t;

/**
 * @brief Interrupt context structure for nested interrupts
 */
typedef struct interrupt_context {
    struct cpu_state        cpu_state;     /**< Saved CPU state */
    uint8_t                 vector;        /**< Interrupt vector */
    uint64_t                error_code;    /**< Error code (if applicable) */
    uint64_t                timestamp;     /**< Interrupt timestamp */
    struct interrupt_context* prev;       /**< Previous context (for nesting) */
} interrupt_context_t;

/**
 * @brief Interrupt priority levels
 */
typedef enum {
    INTERRUPT_PRIORITY_LOW = 0,        /**< Low priority */
    INTERRUPT_PRIORITY_NORMAL,         /**< Normal priority */
    INTERRUPT_PRIORITY_HIGH,           /**< High priority */
    INTERRUPT_PRIORITY_CRITICAL,       /**< Critical priority */
    INTERRUPT_PRIORITY_NMI             /**< Non-maskable interrupt */
} interrupt_priority_t;

// Function declarations

/**
 * @brief Initialize the complete interrupt handling system
 * 
 * Sets up IDT, PIC, exception handlers, and timer management.
 * 
 * @return 0 on success, negative error code on failure
 */
int interrupt_init(void);

/**
 * @brief Shutdown the interrupt handling system
 * 
 * Cleanly disables all interrupts and frees resources.
 */
void interrupt_shutdown(void);

/**
 * @brief Initialize the Programmable Interrupt Controller (PIC)
 * 
 * Remaps the PIC to avoid conflicts with CPU exceptions.
 * 
 * @return 0 on success, negative error code on failure
 */
int pic_init(void);

/**
 * @brief Send End of Interrupt (EOI) to PIC
 * 
 * @param irq IRQ number that was serviced
 */
void pic_send_eoi(uint8_t irq);

/**
 * @brief Enable a specific IRQ
 * 
 * @param irq IRQ number to enable (0-15)
 */
void pic_enable_irq(uint8_t irq);

/**
 * @brief Disable a specific IRQ
 * 
 * @param irq IRQ number to disable (0-15)
 */
void pic_disable_irq(uint8_t irq);

/**
 * @brief Get IRQ mask from PIC
 * 
 * @return Current IRQ mask (16-bit)
 */
uint16_t pic_get_irq_mask(void);

/**
 * @brief Set IRQ mask for PIC
 * 
 * @param mask IRQ mask to set (16-bit)
 */
void pic_set_irq_mask(uint16_t mask);

/**
 * @brief Initialize timer interrupt
 * 
 * Sets up the Programmable Interval Timer (PIT) for regular timer interrupts.
 * 
 * @param frequency Timer frequency in Hz
 * @return 0 on success, negative error code on failure
 */
int timer_init(uint32_t frequency);

/**
 * @brief Get current timer ticks
 * 
 * @return Number of timer ticks since boot
 */
uint64_t timer_get_ticks(void);

/**
 * @brief Get current uptime in milliseconds
 * 
 * @return Milliseconds since boot
 */
uint64_t timer_get_uptime_ms(void);

/**
 * @brief Get current uptime in seconds
 * 
 * @return Seconds since boot
 */
uint64_t timer_get_uptime_sec(void);

/**
 * @brief Sleep for specified milliseconds
 * 
 * @param ms Milliseconds to sleep
 */
void timer_sleep_ms(uint32_t ms);

/**
 * @brief Sleep for specified microseconds
 * 
 * @param us Microseconds to sleep
 */
void timer_sleep_us(uint32_t us);

/**
 * @brief Register exception handler
 * 
 * @param exception Exception vector number
 * @param handler Exception handler function
 * @return 0 on success, negative error code on failure
 */
int exception_register_handler(uint8_t exception, interrupt_handler_t handler);

/**
 * @brief Register hardware interrupt handler
 * 
 * @param irq IRQ number
 * @param handler Interrupt handler function
 * @return 0 on success, negative error code on failure
 */
int irq_register_handler(uint8_t irq, interrupt_handler_t handler);

/**
 * @brief Unregister hardware interrupt handler
 * 
 * @param irq IRQ number
 * @return 0 on success, negative error code on failure
 */
int irq_unregister_handler(uint8_t irq);

/**
 * @brief Trigger a software interrupt
 * 
 * @param vector Interrupt vector to trigger
 * @param data Optional data to pass to handler
 */
void software_interrupt(uint8_t vector, uint64_t data);

/**
 * @brief Get exception information
 * 
 * @param exception Exception vector number
 * @return Pointer to exception information, NULL if invalid
 */
const exception_info_t* exception_get_info(uint8_t exception);

/**
 * @brief Get hardware interrupt information
 * 
 * @param irq IRQ number
 * @return Pointer to interrupt information, NULL if invalid
 */
const hardware_interrupt_info_t* irq_get_info(uint8_t irq);

/**
 * @brief Get timer manager information
 * 
 * @return Pointer to timer manager structure
 */
const timer_manager_t* timer_get_manager(void);

/**
 * @brief Enter critical section (disable interrupts)
 * 
 * @return Previous interrupt state
 */
uint64_t critical_section_enter(void);

/**
 * @brief Exit critical section (restore interrupts)
 * 
 * @param state Previous interrupt state from critical_section_enter()
 */
void critical_section_exit(uint64_t state);

/**
 * @brief Handle nested interrupt entry
 * 
 * @param vector Interrupt vector
 * @param error_code Error code (if applicable)
 * @param context CPU context
 * @return Interrupt context for this interrupt
 */
interrupt_context_t* interrupt_enter(uint8_t vector, uint64_t error_code, struct cpu_state* context);

/**
 * @brief Handle nested interrupt exit
 * 
 * @param ctx Interrupt context from interrupt_enter()
 */
void interrupt_exit(interrupt_context_t* ctx);

/**
 * @brief Check if we're currently in an interrupt
 * 
 * @return true if in interrupt context, false otherwise
 */
bool in_interrupt(void);

/**
 * @brief Get current interrupt nesting level
 * 
 * @return Current nesting level (0 = not in interrupt)
 */
uint32_t interrupt_get_nesting_level(void);

/**
 * @brief Set interrupt priority for current CPU
 * 
 * @param priority Interrupt priority level
 */
void interrupt_set_priority(interrupt_priority_t priority);

/**
 * @brief Get interrupt priority for current CPU
 * 
 * @return Current interrupt priority level
 */
interrupt_priority_t interrupt_get_priority(void);

/**
 * @brief Handle spurious interrupts
 * 
 * @param vector Spurious interrupt vector
 */
void handle_spurious_interrupt(uint8_t vector);

/**
 * @brief Dump interrupt system status for debugging
 */
void interrupt_dump_status(void);

/**
 * @brief Reset all interrupt statistics
 */
void interrupt_reset_statistics(void);

// Assembly function prototypes (implemented in interrupt_asm.s)

/**
 * @brief Assembly interrupt handler stubs
 * These are implemented in assembly and call the C handlers
 */
extern void isr0(void);   // Divide by zero
extern void isr1(void);   // Debug
extern void isr2(void);   // Non-maskable interrupt
extern void isr3(void);   // Breakpoint
extern void isr4(void);   // Overflow
extern void isr5(void);   // Bound range exceeded
extern void isr6(void);   // Invalid opcode
extern void isr7(void);   // Device not available
extern void isr8(void);   // Double fault
extern void isr10(void);  // Invalid TSS
extern void isr11(void);  // Segment not present
extern void isr12(void);  // Stack fault
extern void isr13(void);  // General protection fault
extern void isr14(void);  // Page fault
extern void isr16(void);  // x87 FPU error
extern void isr17(void);  // Alignment check
extern void isr18(void);  // Machine check
extern void isr19(void);  // SIMD floating-point exception

// IRQ handlers
extern void irq0(void);   // Timer
extern void irq1(void);   // Keyboard
extern void irq2(void);   // Cascade
extern void irq3(void);   // COM2
extern void irq4(void);   // COM1
extern void irq5(void);   // LPT2
extern void irq6(void);   // Floppy
extern void irq7(void);   // LPT1
extern void irq8(void);   // RTC
extern void irq9(void);   // Free
extern void irq10(void);  // Free
extern void irq11(void);  // Free
extern void irq12(void);  // Mouse
extern void irq13(void);  // Coprocessor
extern void irq14(void);  // Primary ATA
extern void irq15(void);  // Secondary ATA

/**
 * @brief Common interrupt handler entry point
 * Called by all assembly interrupt stubs
 * 
 * @param vector Interrupt vector number
 * @param error_code Error code (0 if not applicable)
 * @param context CPU context structure
 */
void interrupt_common_handler(uint8_t vector, uint64_t error_code, struct cpu_state* context);

#endif /* __INTERRUPT_H__ */ 