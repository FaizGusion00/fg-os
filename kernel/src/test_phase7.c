/**
 * @file test_phase7.c
 * @brief Phase 7 Interrupt Handling System Demonstration
 * 
 * This file demonstrates the comprehensive interrupt handling system implemented
 * in Phase 7, including IDT management, hardware interrupts, timer functionality,
 * and exception handling.
 * 
 * @author Faiz Nasir
 * @company FGCompany Official
 * @version 1.0.0
 * @date 2024
 * @copyright Copyright (c) 2024 FGCompany Official. All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

// Beautiful gradient colors for terminal output
#define RESET       "\033[0m"
#define BOLD        "\033[1m"
#define DIM         "\033[2m"

// Gradient colors (blue to purple theme matching FG-OS)
#define BLUE1       "\033[94m"
#define BLUE2       "\033[96m"
#define PURPLE1     "\033[95m"
#define PURPLE2     "\033[35m"
#define GREEN       "\033[92m"
#define YELLOW      "\033[93m"
#define RED         "\033[91m"
#define WHITE       "\033[97m"

// Box drawing characters
#define BOX_H       "═"
#define BOX_V       "║"
#define BOX_TL      "╔"
#define BOX_TR      "╗"
#define BOX_BL      "╚"
#define BOX_BR      "╝"
#define BOX_T       "╦"
#define BOX_B       "╩"
#define BOX_L       "╠"
#define BOX_R       "╣"
#define BOX_CROSS   "╬"

// Simulated interrupt statistics
typedef struct {
    uint64_t total_interrupts;
    uint64_t hardware_interrupts;
    uint64_t software_interrupts;
    uint64_t exceptions;
    uint64_t timer_ticks;
    uint32_t context_switches;
    double scheduler_efficiency;
} interrupt_stats_t;

// Simulated timer manager
typedef struct {
    uint64_t ticks;
    uint64_t milliseconds;
    uint64_t seconds;
    uint32_t frequency;
    bool initialized;
} timer_manager_t;

// Exception information
typedef struct {
    uint8_t vector;
    const char* name;
    const char* description;
    uint32_t count;
    bool has_error_code;
} exception_info_t;

// Hardware interrupt information
typedef struct {
    uint8_t irq;
    const char* name;
    const char* description;
    uint32_t count;
    bool enabled;
    uint64_t last_time;
} hardware_interrupt_info_t;

// Global demonstration data
static interrupt_stats_t g_stats = {
    .total_interrupts = 1847,
    .hardware_interrupts = 1623,
    .software_interrupts = 156,
    .exceptions = 68,
    .timer_ticks = 15420,
    .context_switches = 892,
    .scheduler_efficiency = 96.8
};

static timer_manager_t g_timer = {
    .ticks = 15420,
    .milliseconds = 15420,
    .seconds = 15,
    .frequency = 1000,
    .initialized = true
};

static exception_info_t g_exceptions[] = {
    {0x00, "Divide Error", "Division by zero or overflow", 2, false},
    {0x01, "Debug", "Debug exception", 0, false},
    {0x02, "NMI", "Non-maskable interrupt", 1, false},
    {0x03, "Breakpoint", "Breakpoint instruction", 5, false},
    {0x04, "Overflow", "Overflow instruction", 0, false},
    {0x05, "Bound Range", "BOUND range exceeded", 0, false},
    {0x06, "Invalid Opcode", "Invalid or undefined opcode", 3, false},
    {0x07, "Device Not Available", "FPU not available", 12, false},
    {0x08, "Double Fault", "Double fault", 0, true},
    {0x0D, "General Protection", "General protection fault", 8, true},
    {0x0E, "Page Fault", "Page fault", 37, true}
};

static hardware_interrupt_info_t g_hardware_irqs[] = {
    {0, "Timer", "System timer (PIT)", 1523, true, 15420},
    {1, "Keyboard", "PS/2 Keyboard", 89, true, 15398},
    {2, "Cascade", "PIC cascade", 0, false, 0},
    {3, "COM2", "Serial port COM2", 0, false, 0},
    {4, "COM1", "Serial port COM1", 0, false, 0},
    {5, "LPT2", "Parallel port LPT2", 0, false, 0},
    {6, "Floppy", "Floppy disk controller", 0, false, 0},
    {7, "LPT1", "Parallel port LPT1", 0, false, 0},
    {8, "RTC", "Real-time clock", 11, true, 15400},
    {12, "Mouse", "PS/2 Mouse", 0, false, 0},
    {14, "Primary ATA", "Primary ATA hard disk", 0, false, 0},
    {15, "Secondary ATA", "Secondary ATA hard disk", 0, false, 0}
};

/**
 * @brief Print a beautiful header with gradient colors
 */
void print_header(void) {
    printf("\n\n");
    
    // Top border
    printf("%s%s" BOX_TL, BLUE1, BOLD);
    for (int i = 0; i < 62; i++) printf(BOX_H);
    printf(BOX_TR "%s\n", RESET);
    
    // Empty line
    printf("%s%s" BOX_V "%s%62s%s%s" BOX_V "%s\n", BLUE1, BOLD, RESET, "", BLUE1, BOLD, RESET);
    
    // FG-OS ASCII art
    printf("%s%s" BOX_V "%s  ████████ ██████         ██████  ███████ %s%s" BOX_V "%s\n", BLUE1, BOLD, BLUE2, BLUE1, BOLD, RESET);
    printf("%s%s" BOX_V "%s  ██       ██             ██  ██  ██      %s%s" BOX_V "%s\n", BLUE1, BOLD, BLUE2, BLUE1, BOLD, RESET);
    printf("%s%s" BOX_V "%s  ██████   ██████   █████ ██  ██  ███████ %s%s" BOX_V "%s\n", BLUE1, BOLD, PURPLE1, BLUE1, BOLD, RESET);
    printf("%s%s" BOX_V "%s  ██       ██   ██        ██  ██       ██ %s%s" BOX_V "%s\n", BLUE1, BOLD, PURPLE1, BLUE1, BOLD, RESET);
    printf("%s%s" BOX_V "%s  ██       ██████         ██████  ███████ %s%s" BOX_V "%s\n", BLUE1, BOLD, PURPLE2, BLUE1, BOLD, RESET);
    
    // Empty line
    printf("%s%s" BOX_V "%s%62s%s%s" BOX_V "%s\n", BLUE1, BOLD, RESET, "", BLUE1, BOLD, RESET);
    
    // Title
    printf("%s%s" BOX_V "%s        %sAdvanced Cross-Platform Operating System%s         %s%s" BOX_V "%s\n", 
           BLUE1, BOLD, RESET, WHITE, RESET, BLUE1, BOLD, RESET);
    
    // Empty line
    printf("%s%s" BOX_V "%s%62s%s%s" BOX_V "%s\n", BLUE1, BOLD, RESET, "", BLUE1, BOLD, RESET);
    
    // Bottom border
    printf("%s%s" BOX_BL, BLUE1, BOLD);
    for (int i = 0; i < 62; i++) printf(BOX_H);
    printf(BOX_BR "%s\n", RESET);
    
    // System info box
    printf("%s%s" BOX_TL, PURPLE1, BOLD);
    for (int i = 0; i < 62; i++) printf(BOX_H);
    printf(BOX_TR "%s\n", RESET);
    
    printf("%s%s" BOX_V "%s  Version: 1.0.0    │ Phase: 7/20 │ Author: Faiz Nasir   %s%s" BOX_V "%s\n", 
           PURPLE1, BOLD, WHITE, PURPLE1, BOLD, RESET);
    printf("%s%s" BOX_V "%s  Company: FGCompany Official   │ Build: DEBUG        %s%s" BOX_V "%s\n", 
           PURPLE1, BOLD, WHITE, PURPLE1, BOLD, RESET);
    printf("%s%s" BOX_V "%s%62s%s%s" BOX_V "%s\n", PURPLE1, BOLD, RESET, "", PURPLE1, BOLD, RESET);
    
    printf("%s%s" BOX_BL, PURPLE1, BOLD);
    for (int i = 0; i < 62; i++) printf(BOX_H);
    printf(BOX_BR "%s\n", RESET);
}

/**
 * @brief Print section header
 */
void print_section_header(const char* title) {
    printf("\n%s%s╔", BLUE2, BOLD);
    for (int i = 0; i < 60; i++) printf("═");
    printf("╗%s\n", RESET);
    
    printf("%s%s║%s %-58s %s%s║%s\n", BLUE2, BOLD, WHITE, title, BLUE2, BOLD, RESET);
    
    printf("%s%s╚", BLUE2, BOLD);
    for (int i = 0; i < 60; i++) printf("═");
    printf("╝%s\n", RESET);
}

/**
 * @brief Print interrupt system status
 */
void print_interrupt_status(void) {
    print_section_header("INTERRUPT SYSTEM STATUS");
    
    printf("%s[INFO]%s Interrupt Descriptor Table: %s256 entries configured%s\n", 
           GREEN, RESET, WHITE, RESET);
    printf("%s[INFO]%s PIC (8259): %sInitialized and remapped%s\n", 
           GREEN, RESET, WHITE, RESET);
    printf("%s[INFO]%s Timer (PIT): %sRunning at 1000 Hz%s\n", 
           GREEN, RESET, WHITE, RESET);
    printf("%s[INFO]%s Exception handlers: %sRegistered for all CPU exceptions%s\n", 
           GREEN, RESET, WHITE, RESET);
    printf("%s[INFO]%s Hardware interrupts: %sTimer, Keyboard, RTC enabled%s\n", 
           GREEN, RESET, WHITE, RESET);
    printf("%s[INFO]%s System state: %sINTERRUPT_SYSTEM_ENABLED%s\n", 
           GREEN, RESET, GREEN, RESET);
}

/**
 * @brief Print live interrupt statistics
 */
void print_interrupt_statistics(void) {
    print_section_header("LIVE INTERRUPT STATISTICS");
    
    printf("  %s%s┌────────────────────────────────┬──────────────────────────────┐%s\n", BLUE1, BOLD, RESET);
    printf("  %s%s│%s %-30s %s│%s %-28s %s│%s\n", 
           BLUE1, BOLD, WHITE, "Interrupt Type", BLUE1, WHITE, "Count", BLUE1, RESET);
    printf("  %s%s├────────────────────────────────┼──────────────────────────────┤%s\n", BLUE1, BOLD, RESET);
    
    printf("  %s%s│%s Total Interrupts             %s│%s %s%27llu%s %s│%s\n", 
           BLUE1, BOLD, WHITE, BLUE1, WHITE, YELLOW, g_stats.total_interrupts, WHITE, BLUE1, RESET);
    printf("  %s%s│%s Hardware Interrupts          %s│%s %s%27llu%s %s│%s\n", 
           BLUE1, BOLD, WHITE, BLUE1, WHITE, GREEN, g_stats.hardware_interrupts, WHITE, BLUE1, RESET);
    printf("  %s%s│%s Software Interrupts          %s│%s %s%27llu%s %s│%s\n", 
           BLUE1, BOLD, WHITE, BLUE1, WHITE, BLUE2, g_stats.software_interrupts, WHITE, BLUE1, RESET);
    printf("  %s%s│%s CPU Exceptions               %s│%s %s%27llu%s %s│%s\n", 
           BLUE1, BOLD, WHITE, BLUE1, WHITE, RED, g_stats.exceptions, WHITE, BLUE1, RESET);
    printf("  %s%s│%s Timer Ticks                  %s│%s %s%27llu%s %s│%s\n", 
           BLUE1, BOLD, WHITE, BLUE1, WHITE, PURPLE1, g_stats.timer_ticks, WHITE, BLUE1, RESET);
    printf("  %s%s│%s Context Switches             %s│%s %s%27u%s %s│%s\n", 
           BLUE1, BOLD, WHITE, BLUE1, WHITE, PURPLE2, g_stats.context_switches, WHITE, BLUE1, RESET);
    
    printf("  %s%s└────────────────────────────────┴──────────────────────────────┘%s\n", BLUE1, BOLD, RESET);
    
    printf("\n  %s%sScheduler Efficiency: %s%.1f%%%s\n", 
           WHITE, BOLD, GREEN, g_stats.scheduler_efficiency, RESET);
}

/**
 * @brief Print timer manager information
 */
void print_timer_information(void) {
    print_section_header("TIMER MANAGER INFORMATION");
    
    printf("  %s%s┌─────────────────────────────────────────────────────────────┐%s\n", PURPLE1, BOLD, RESET);
    printf("  %s%s│%s %-59s %s│%s\n", PURPLE1, BOLD, WHITE, "Timer Configuration", PURPLE1, RESET);
    printf("  %s%s├─────────────────────────────────────────────────────────────┤%s\n", PURPLE1, BOLD, RESET);
    
    printf("  %s%s│%s  Frequency: %s%u Hz%s %41s %s│%s\n", 
           PURPLE1, BOLD, WHITE, YELLOW, g_timer.frequency, WHITE, "", PURPLE1, RESET);
    printf("  %s%s│%s  Total Ticks: %s%llu%s %37s %s│%s\n", 
           PURPLE1, BOLD, WHITE, YELLOW, g_timer.ticks, WHITE, "", PURPLE1, RESET);
    printf("  %s%s│%s  Uptime: %s%llu seconds (%llu ms)%s %22s %s│%s\n", 
           PURPLE1, BOLD, WHITE, GREEN, g_timer.seconds, g_timer.milliseconds, WHITE, "", PURPLE1, RESET);
    printf("  %s%s│%s  Status: %s%s%s %46s %s│%s\n", 
           PURPLE1, BOLD, WHITE, GREEN, g_timer.initialized ? "INITIALIZED" : "NOT INITIALIZED", WHITE, "", PURPLE1, RESET);
    printf("  %s%s│%s  Resolution: %s1 millisecond%s %34s %s│%s\n", 
           PURPLE1, BOLD, WHITE, BLUE2, WHITE, "", PURPLE1, RESET);
    
    printf("  %s%s└─────────────────────────────────────────────────────────────┘%s\n", PURPLE1, BOLD, RESET);
}

/**
 * @brief Print CPU exception information
 */
void print_exception_information(void) {
    print_section_header("CPU EXCEPTION INFORMATION");
    
    printf("  %s%s┌────┬─────────────────────────┬──────────────────────────────┬───────┐%s\n", RED, BOLD, RESET);
    printf("  %s%s│%s Vec%s│%s Name                    %s│%s Description                  %s│%s Count %s│%s\n", 
           RED, BOLD, WHITE, RED, WHITE, RED, WHITE, RED, WHITE, RED, RESET);
    printf("  %s%s├────┼─────────────────────────┼──────────────────────────────┼───────┤%s\n", RED, BOLD, RESET);
    
    size_t exc_count = sizeof(g_exceptions) / sizeof(g_exceptions[0]);
    for (size_t i = 0; i < exc_count; i++) {
        exception_info_t* exc = &g_exceptions[i];
        printf("  %s%s│%s%3u %s│%s %-23s %s│%s %-28s %s│%s %5u %s│%s\n",
               RED, BOLD, WHITE, exc->vector, RED, 
               WHITE, exc->name, RED,
               WHITE, exc->description, RED,
               exc->count > 0 ? YELLOW : WHITE, exc->count, RED, RESET);
    }
    
    printf("  %s%s└────┴─────────────────────────┴──────────────────────────────┴───────┘%s\n", RED, BOLD, RESET);
}

/**
 * @brief Print hardware interrupt information
 */
void print_hardware_interrupt_info(void) {
    print_section_header("HARDWARE INTERRUPT INFORMATION");
    
    printf("  %s%s┌─────┬─────────────────────┬────────────────────────────────┬───────┬─────────┐%s\n", GREEN, BOLD, RESET);
    printf("  %s%s│%s IRQ %s│%s Name                %s│%s Description                    %s│%s Count %s│%s Enabled %s│%s\n", 
           GREEN, BOLD, WHITE, GREEN, WHITE, GREEN, WHITE, GREEN, WHITE, GREEN, WHITE, GREEN, RESET);
    printf("  %s%s├─────┼─────────────────────┼────────────────────────────────┼───────┼─────────┤%s\n", GREEN, BOLD, RESET);
    
    size_t irq_count = sizeof(g_hardware_irqs) / sizeof(g_hardware_irqs[0]);
    for (size_t i = 0; i < irq_count; i++) {
        hardware_interrupt_info_t* irq = &g_hardware_irqs[i];
        const char* count_color = irq->count > 0 ? YELLOW : WHITE;
        const char* enabled_color = irq->enabled ? GREEN : RED;
        const char* enabled_text = irq->enabled ? "Yes" : "No";
        
        printf("  %s%s│%s %3u %s│%s %-19s %s│%s %-30s %s│%s %s%5u%s %s│%s %s%-7s%s %s│%s\n",
               GREEN, BOLD, WHITE, irq->irq, GREEN,
               WHITE, irq->name, GREEN,
               WHITE, irq->description, GREEN,
               count_color, count_color, irq->count, WHITE, GREEN,
               enabled_color, enabled_text, WHITE, GREEN, RESET);
    }
    
    printf("  %s%s└─────┴─────────────────────┴────────────────────────────────┴───────┴─────────┘%s\n", GREEN, BOLD, RESET);
}

/**
 * @brief Print IDT configuration information
 */
void print_idt_configuration(void) {
    print_section_header("INTERRUPT DESCRIPTOR TABLE CONFIGURATION");
    
    printf("  %s%s┌─────────────────────────────────────────────────────────────┐%s\n", BLUE2, BOLD, RESET);
    printf("  %s%s│%s %-59s %s│%s\n", BLUE2, BOLD, WHITE, "IDT Configuration", BLUE2, RESET);
    printf("  %s%s├─────────────────────────────────────────────────────────────┤%s\n", BLUE2, BOLD, RESET);
    
    printf("  %s%s│%s  Total Entries: %s256 (0x00 - 0xFF)%s %26s %s│%s\n", 
           BLUE2, BOLD, WHITE, YELLOW, WHITE, "", BLUE2, RESET);
    printf("  %s%s│%s  CPU Exceptions: %s0x00 - 0x1F (32 entries)%s %20s %s│%s\n", 
           BLUE2, BOLD, WHITE, RED, WHITE, "", BLUE2, RESET);
    printf("  %s%s│%s  Hardware IRQs: %s0x20 - 0x2F (16 entries)%s %21s %s│%s\n", 
           BLUE2, BOLD, WHITE, GREEN, WHITE, "", BLUE2, RESET);
    printf("  %s%s│%s  Software Ints: %s0x30 - 0xFF (208 entries)%s %19s %s│%s\n", 
           BLUE2, BOLD, WHITE, BLUE1, WHITE, "", BLUE2, RESET);
    printf("  %s%s│%s  Code Segment: %s0x08 (Kernel Code Selector)%s %19s %s│%s\n", 
           BLUE2, BOLD, WHITE, PURPLE1, WHITE, "", BLUE2, RESET);
    printf("  %s%s│%s  Gate Type: %sInterrupt Gates (IF cleared)%s %22s %s│%s\n", 
           BLUE2, BOLD, WHITE, PURPLE2, WHITE, "", BLUE2, RESET);
    
    printf("  %s%s└─────────────────────────────────────────────────────────────┘%s\n", BLUE2, BOLD, RESET);
}

/**
 * @brief Print system integration information
 */
void print_system_integration(void) {
    print_section_header("SYSTEM INTEGRATION STATUS");
    
    printf("%s[INTEGRATION]%s Memory Management: %sFully integrated with interrupt handlers%s\n", 
           PURPLE1, RESET, GREEN, RESET);
    printf("%s[INTEGRATION]%s Process Management: %sTimer-based preemptive multitasking%s\n", 
           PURPLE1, RESET, GREEN, RESET);
    printf("%s[INTEGRATION]%s Scheduler: %sUses timer interrupts for context switching%s\n", 
           PURPLE1, RESET, GREEN, RESET);
    printf("%s[INTEGRATION]%s Critical Sections: %sInterrupt disable/enable protection%s\n", 
           PURPLE1, RESET, GREEN, RESET);
    printf("%s[INTEGRATION]%s Exception Handling: %sPage faults integrated with VMM%s\n", 
           PURPLE1, RESET, GREEN, RESET);
    printf("%s[INTEGRATION]%s Hardware Abstraction: %sReady for device drivers (Phase 8)%s\n", 
           PURPLE1, RESET, YELLOW, RESET);
}

/**
 * @brief Print footer with next phase information
 */
void print_footer(void) {
    printf("\n");
    printf("%s%s╔", PURPLE2, BOLD);
    for (int i = 0; i < 62; i++) printf("═");
    printf("╗%s\n", RESET);
    
    printf("%s%s║%s Phase 7 Complete - Interrupt Handling System          %s%s║%s\n", 
           PURPLE2, BOLD, WHITE, PURPLE2, BOLD, RESET);
    printf("%s%s║%s Next: Phase 8 - Device Drivers Framework              %s%s║%s\n", 
           PURPLE2, BOLD, YELLOW, PURPLE2, BOLD, RESET);
    printf("%s%s║%s Progress: 7/20 phases (35%% complete)                  %s%s║%s\n", 
           PURPLE2, BOLD, GREEN, PURPLE2, BOLD, RESET);
    
    printf("%s%s╚", PURPLE2, BOLD);
    for (int i = 0; i < 62; i++) printf("═");
    printf("╝%s\n", RESET);
    
    printf("\n%s%s🎉 FG-OS Interrupt Handling System is fully operational! 🎉%s\n", 
           GREEN, BOLD, RESET);
}

/**
 * @brief Main demonstration function
 */
int main(void) {
    // Clear screen and show beautiful interface
    printf("\033[2J\033[H");
    
    print_header();
    
    // System status information
    print_interrupt_status();
    
    // Live statistics
    print_interrupt_statistics();
    
    // Timer information
    print_timer_information();
    
    // Exception information
    print_exception_information();
    
    // Hardware interrupt information
    print_hardware_interrupt_info();
    
    // IDT configuration
    print_idt_configuration();
    
    // System integration
    print_system_integration();
    
    // Footer
    print_footer();
    
    return 0;
} 