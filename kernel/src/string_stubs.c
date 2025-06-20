/*
 * FG-OS Basic String Functions
 * Phase 6: Process Management System
 * 
 * Developed by: Faiz Nasir
 * Company: FGCompany Official
 * 
 * Basic string manipulation functions for kernel use.
 */

#include "../include/kernel.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

/**
 * @brief Copy at most n characters from src to dest
 * 
 * @param dest Destination buffer
 * @param src Source string
 * @param n Maximum number of characters to copy
 * @return Pointer to dest
 */
char* strncpy(char* dest, const char* src, size_t n) {
    char* result = dest;
    
    while (n > 0 && *src != '\0') {
        *dest++ = *src++;
        n--;
    }
    
    while (n > 0) {
        *dest++ = '\0';
        n--;
    }
    
    return result;
}

/**
 * @brief Set memory to a specific value
 * 
 * @param ptr Pointer to memory
 * @param value Value to set
 * @param size Number of bytes to set
 * @return Pointer to memory
 */
void* memset(void* ptr, int value, size_t size) {
    unsigned char* p = (unsigned char*)ptr;
    unsigned char val = (unsigned char)value;
    
    while (size--) {
        *p++ = val;
    }
    
    return ptr;
}

/**
 * @brief Copy memory from source to destination
 * 
 * @param dest Destination pointer
 * @param src Source pointer
 * @param n Number of bytes to copy
 * @return Pointer to destination
 */
void* memcpy(void* dest, const void* src, size_t n) {
    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;
    
    while (n--) {
        *d++ = *s++;
    }
    
    return dest;
}

/**
 * @brief Compare two memory blocks
 * 
 * @param ptr1 First memory block
 * @param ptr2 Second memory block
 * @param num Number of bytes to compare
 * @return 0 if equal, negative if ptr1 < ptr2, positive if ptr1 > ptr2
 */
int memcmp(const void* ptr1, const void* ptr2, size_t num) {
    const unsigned char* p1 = (const unsigned char*)ptr1;
    const unsigned char* p2 = (const unsigned char*)ptr2;
    
    while (num--) {
        if (*p1 != *p2) {
            return *p1 - *p2;
        }
        p1++;
        p2++;
    }
    
    return 0;
}

/**
 * @brief Kernel panic function - stops execution with error message
 * 
 * @param fmt Format string for error message
 * @param ... Variable arguments for format string
 */
__noreturn void panic(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    
    // In a real kernel, this would halt the system
    // For demonstration, we'll print the message and exit
    printf("\n[KERNEL PANIC] ");
    vprintf(fmt, args);
    printf("\nSystem halted.\n");
    
    va_end(args);
    exit(1);
} 