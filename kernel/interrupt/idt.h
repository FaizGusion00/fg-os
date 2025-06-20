/**
 * @file idt.h
 * @brief Interrupt Descriptor Table (IDT) management for FG-OS
 * 
 * This file provides structures and functions for managing the x86_64 IDT,
 * including interrupt gates, trap gates, and task gates for hardware and
 * software interrupts.
 * 
 * @author Faiz Nasir
 * @company FGCompany Official
 * @version 1.0.0
 * @date 2024
 * @copyright Copyright (c) 2024 FGCompany Official. All rights reserved.
 */

#ifndef __IDT_H__
#define __IDT_H__

#include <types.h>
#include "../arch/x86_64/arch.h"

/**
 * @brief Maximum number of IDT entries
 * Intel x86_64 supports 256 interrupt vectors (0-255)
 */
#define IDT_ENTRIES 256

/**
 * @brief IDT Gate Types
 */
#define IDT_GATE_INTERRUPT  0x8E    /**< Interrupt Gate (IF cleared) */
#define IDT_GATE_TRAP       0x8F    /**< Trap Gate (IF unchanged) */
#define IDT_GATE_TASK       0x85    /**< Task Gate (deprecated in x86_64) */
#define IDT_GATE_CALL       0x8C    /**< Call Gate */

/**
 * @brief IDT Privilege Levels
 */
#define IDT_DPL_KERNEL      0x00    /**< Descriptor Privilege Level 0 (Kernel) */
#define IDT_DPL_USER        0x60    /**< Descriptor Privilege Level 3 (User) */

/**
 * @brief Standard x86_64 Exception Vectors
 * Note: Using definitions from arch.h to avoid conflicts
 */
// Exception vectors are defined in arch/x86_64/arch.h

/**
 * @brief Hardware Interrupt Vectors (PIC remapped)
 */
#define IRQ_TIMER               0x20    /**< Timer interrupt (IRQ 0) */
#define IRQ_KEYBOARD            0x21    /**< Keyboard interrupt (IRQ 1) */
#define IRQ_CASCADE             0x22    /**< Cascade (never raised) (IRQ 2) */
#define IRQ_COM2                0x23    /**< COM2 (IRQ 3) */
#define IRQ_COM1                0x24    /**< COM1 (IRQ 4) */
#define IRQ_LPT2                0x25    /**< LPT2 (IRQ 5) */
#define IRQ_FLOPPY              0x26    /**< Floppy disk (IRQ 6) */
#define IRQ_LPT1                0x27    /**< LPT1 (IRQ 7) */
#define IRQ_RTC                 0x28    /**< Real-time clock (IRQ 8) */
#define IRQ_FREE1               0x29    /**< Free (IRQ 9) */
#define IRQ_FREE2               0x2A    /**< Free (IRQ 10) */
#define IRQ_FREE3               0x2B    /**< Free (IRQ 11) */
#define IRQ_MOUSE               0x2C    /**< PS/2 mouse (IRQ 12) */
#define IRQ_COPROCESSOR         0x2D    /**< Math coprocessor (IRQ 13) */
#define IRQ_PRIMARY_ATA         0x2E    /**< Primary ATA (IRQ 14) */
#define IRQ_SECONDARY_ATA       0x2F    /**< Secondary ATA (IRQ 15) */

/**
 * @brief Software Interrupt Vectors
 */
#define INT_SYSCALL             0x80    /**< System call interrupt */
#define INT_YIELD               0x81    /**< Process yield interrupt */
#define INT_IPI                 0x82    /**< Inter-processor interrupt */

/**
 * @brief IDT Entry structure for x86_64
 * 
 * Each IDT entry is 16 bytes and contains information about how to handle
 * a specific interrupt or exception vector.
 */
typedef struct __attribute__((packed)) {
    uint16_t    offset_low;     /**< Lower 16 bits of handler address */
    uint16_t    selector;       /**< Code segment selector */
    uint8_t     ist;            /**< Interrupt Stack Table index (0-7) */
    uint8_t     type_attr;      /**< Type and attributes */
    uint16_t    offset_mid;     /**< Middle 16 bits of handler address */
    uint32_t    offset_high;    /**< Upper 32 bits of handler address */
    uint32_t    reserved;       /**< Reserved (must be zero) */
} idt_entry_t;

/**
 * @brief IDT Pointer structure for LIDT instruction
 */
typedef struct __attribute__((packed)) {
    uint16_t    limit;          /**< Size of IDT - 1 */
    uint64_t    base;           /**< Base address of IDT */
} idt_ptr_t;

/**
 * @brief Interrupt handler function pointer type
 * 
 * @param vector Interrupt vector number
 * @param error_code Error code (if applicable)
 * @param context CPU context at time of interrupt
 */
typedef void (*interrupt_handler_t)(uint8_t vector, uint64_t error_code, struct cpu_state* context);

/**
 * @brief Interrupt statistics structure
 */
typedef struct {
    uint64_t    total_interrupts;      /**< Total interrupt count */
    uint64_t    exceptions;            /**< Exception count */
    uint64_t    hardware_interrupts;   /**< Hardware interrupt count */
    uint64_t    software_interrupts;   /**< Software interrupt count */
    uint64_t    spurious_interrupts;   /**< Spurious interrupt count */
    uint64_t    nested_interrupts;     /**< Nested interrupt count */
    uint64_t    interrupt_overruns;    /**< Interrupt overrun count */
    uint64_t    max_interrupt_latency; /**< Maximum interrupt latency (microseconds) */
    uint64_t    avg_interrupt_latency; /**< Average interrupt latency (microseconds) */
    uint32_t    interrupt_frequency[IDT_ENTRIES]; /**< Per-vector interrupt frequency */
} interrupt_stats_t;

/**
 * @brief Interrupt system state
 */
typedef enum {
    INTERRUPT_SYSTEM_DISABLED = 0,    /**< Interrupt system disabled */
    INTERRUPT_SYSTEM_INITIALIZING,    /**< Interrupt system initializing */
    INTERRUPT_SYSTEM_ENABLED,         /**< Interrupt system enabled */
    INTERRUPT_SYSTEM_SUSPENDED,       /**< Interrupt system temporarily suspended */
    INTERRUPT_SYSTEM_ERROR            /**< Interrupt system in error state */
} interrupt_system_state_t;

/**
 * @brief Global interrupt management structure
 */
typedef struct {
    idt_entry_t                 idt[IDT_ENTRIES];           /**< IDT entries */
    idt_ptr_t                   idt_ptr;                    /**< IDT pointer */
    interrupt_handler_t         handlers[IDT_ENTRIES];      /**< Interrupt handlers */
    interrupt_stats_t           stats;                      /**< Interrupt statistics */
    interrupt_system_state_t    state;                      /**< System state */
    uint32_t                    nested_level;               /**< Current nesting level */
    uint64_t                    disabled_count;             /**< Disable/enable balance */
    bool                        pic_initialized;           /**< PIC initialization status */
    bool                        apic_available;             /**< APIC availability */
    bool                        apic_enabled;               /**< APIC enabled status */
} interrupt_manager_t;

// Function declarations

/**
 * @brief Initialize the Interrupt Descriptor Table
 * 
 * Sets up the IDT with default exception handlers and prepares the system
 * for interrupt handling.
 * 
 * @return 0 on success, negative error code on failure
 */
int idt_init(void);

/**
 * @brief Set an IDT entry
 * 
 * @param vector Interrupt vector number (0-255)
 * @param handler Address of interrupt handler
 * @param selector Code segment selector
 * @param type_attr Type and attribute flags
 * @return 0 on success, negative error code on failure
 */
int idt_set_entry(uint8_t vector, uint64_t handler, uint16_t selector, uint8_t type_attr);

/**
 * @brief Register an interrupt handler
 * 
 * @param vector Interrupt vector number
 * @param handler Interrupt handler function
 * @return 0 on success, negative error code on failure
 */
int idt_register_handler(uint8_t vector, interrupt_handler_t handler);

/**
 * @brief Unregister an interrupt handler
 * 
 * @param vector Interrupt vector number
 * @return 0 on success, negative error code on failure
 */
int idt_unregister_handler(uint8_t vector);

/**
 * @brief Load the IDT into the processor
 * 
 * Uses the LIDT instruction to load the IDT pointer into the processor.
 */
void idt_load(void);

/**
 * @brief Enable interrupts globally
 * 
 * Uses the STI instruction to enable maskable interrupts.
 */
void interrupts_enable(void);

/**
 * @brief Disable interrupts globally
 * 
 * Uses the CLI instruction to disable maskable interrupts.
 * 
 * @return Previous interrupt state (for restoration)
 */
uint64_t interrupts_disable(void);

/**
 * @brief Restore interrupt state
 * 
 * @param state Previous interrupt state from interrupts_disable()
 */
void interrupts_restore(uint64_t state);

/**
 * @brief Check if interrupts are enabled
 * 
 * @return true if interrupts are enabled, false otherwise
 */
bool interrupts_enabled(void);

/**
 * @brief Get interrupt statistics
 * 
 * @return Pointer to interrupt statistics structure
 */
const interrupt_stats_t* idt_get_stats(void);

/**
 * @brief Reset interrupt statistics
 */
void idt_reset_stats(void);

/**
 * @brief Get interrupt system state
 * 
 * @return Current interrupt system state
 */
interrupt_system_state_t idt_get_state(void);

/**
 * @brief Dump IDT information for debugging
 * 
 * @param start_vector Starting vector to dump
 * @param count Number of vectors to dump
 */
void idt_dump(uint8_t start_vector, uint8_t count);

#endif /* __IDT_H__ */ 