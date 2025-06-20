/*
 * FG-OS Console Stub Functions
 * Phase 3: Bootloader Development
 * 
 * Developed by: Faiz Nasir
 * Company: FGCompany Official
 * 
 * Simple console stub functions for Phase 3 testing.
 * Full console implementation will be in Phase 9.
 */

#include <kernel.h>
#include <types.h>

/**
 * Stub printf implementation for Phase 3
 * @param fmt Format string
 * @return Number of characters printed (always 0 for stub)
 */
int printf(const char *fmt, ...) {
    // Phase 3: Stub implementation
    // Real implementation will be added in Phase 9 (Device Drivers)
    (void)fmt; // Suppress unused parameter warning
    return 0;
}

/**
 * Stub putchar implementation for Phase 3
 * @param c Character to print
 * @return Character printed
 */
int putchar(int c) {
    // Phase 3: Stub implementation
    return c;
}

/**
 * Stub puts implementation for Phase 3
 * @param s String to print
 * @return Number of characters printed (always 0 for stub)
 */
int puts(const char *s) {
    // Phase 3: Stub implementation
    (void)s; // Suppress unused parameter warning
    return 0;
} 