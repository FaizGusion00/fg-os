/*
 * FG-OS Memory Utility Functions
 * Phase 5: Memory Management Implementation
 * 
 * Developed by: Faiz Nasir
 * Company: FGCompany Official
 * 
 * Memory manipulation and utility functions.
 */

#include <kernel.h>
#include <types.h>
#include "../mm/memory.h"

/**
 * Copy memory from source to destination
 * @param dest Destination buffer
 * @param src Source buffer
 * @param size Number of bytes to copy
 */
void memory_copy(void* dest, const void* src, size_t size) {
    if (!dest || !src || size == 0) return;
    
    uint8_t *d = (uint8_t*)dest;
    const uint8_t *s = (const uint8_t*)src;
    
    // Handle overlapping memory regions
    if (d < s) {
        // Copy forward
        for (size_t i = 0; i < size; i++) {
            d[i] = s[i];
        }
    } else if (d > s) {
        // Copy backward
        for (size_t i = size; i > 0; i--) {
            d[i - 1] = s[i - 1];
        }
    }
    // If d == s, no copying needed
}

/**
 * Set memory to a specific value
 * @param dest Destination buffer
 * @param value Value to set (only lower 8 bits used)
 * @param size Number of bytes to set
 */
void memory_set(void* dest, int value, size_t size) {
    if (!dest || size == 0) return;
    
    uint8_t *d = (uint8_t*)dest;
    uint8_t val = (uint8_t)value;
    
    // Optimized version for common cases
    if (size >= 8 && ((uint64_t)d % 8) == 0) {
        // 64-bit aligned destination
        uint64_t val64 = val;
        val64 |= val64 << 8;
        val64 |= val64 << 16;
        val64 |= val64 << 32;
        
        uint64_t *d64 = (uint64_t*)d;
        size_t count64 = size / 8;
        
        for (size_t i = 0; i < count64; i++) {
            d64[i] = val64;
        }
        
        // Handle remaining bytes
        d += count64 * 8;
        size -= count64 * 8;
    }
    
    // Set remaining bytes
    for (size_t i = 0; i < size; i++) {
        d[i] = val;
    }
}

/**
 * Compare two memory regions
 * @param ptr1 First memory region
 * @param ptr2 Second memory region
 * @param size Number of bytes to compare
 * @return 0 if equal, negative if ptr1 < ptr2, positive if ptr1 > ptr2
 */
int memory_compare(const void* ptr1, const void* ptr2, size_t size) {
    if (!ptr1 || !ptr2) {
        if (ptr1 == ptr2) return 0;
        return ptr1 ? 1 : -1;
    }
    
    const uint8_t *p1 = (const uint8_t*)ptr1;
    const uint8_t *p2 = (const uint8_t*)ptr2;
    
    for (size_t i = 0; i < size; i++) {
        if (p1[i] != p2[i]) {
            return (int)p1[i] - (int)p2[i];
        }
    }
    
    return 0;
}

/**
 * Find first occurrence of a value in memory
 * @param ptr Memory region to search
 * @param value Value to find (only lower 8 bits used)
 * @param size Size of memory region
 * @return Pointer to first occurrence, or NULL if not found
 */
void* memory_find(const void* ptr, int value, size_t size) {
    if (!ptr || size == 0) return NULL;
    
    const uint8_t *p = (const uint8_t*)ptr;
    uint8_t val = (uint8_t)value;
    
    for (size_t i = 0; i < size; i++) {
        if (p[i] == val) {
            return (void*)(p + i);
        }
    }
    
    return NULL;
}

/**
 * Zero out a memory region (security-oriented)
 * @param ptr Memory region to zero
 * @param size Size of region
 */
void memory_zero_secure(void* ptr, size_t size) {
    if (!ptr || size == 0) return;
    
    // Use volatile to prevent optimization
    volatile uint8_t *p = (volatile uint8_t*)ptr;
    
    for (size_t i = 0; i < size; i++) {
        p[i] = 0;
    }
}

/**
 * Check if memory region is all zeros
 * @param ptr Memory region to check
 * @param size Size of region
 * @return true if all zeros, false otherwise
 */
bool memory_is_zero(const void* ptr, size_t size) {
    if (!ptr || size == 0) return true;
    
    const uint8_t *p = (const uint8_t*)ptr;
    
    for (size_t i = 0; i < size; i++) {
        if (p[i] != 0) {
            return false;
        }
    }
    
    return true;
}

/**
 * Align address up to the nearest boundary
 * @param addr Address to align
 * @param alignment Alignment boundary (must be power of 2)
 * @return Aligned address
 */
uint64_t memory_align_up(uint64_t addr, uint64_t alignment) {
    if (alignment == 0) return addr;
    return (addr + alignment - 1) & ~(alignment - 1);
}

/**
 * Align address down to the nearest boundary
 * @param addr Address to align
 * @param alignment Alignment boundary (must be power of 2)
 * @return Aligned address
 */
uint64_t memory_align_down(uint64_t addr, uint64_t alignment) {
    if (alignment == 0) return addr;
    return addr & ~(alignment - 1);
}

/**
 * Check if address is aligned to boundary
 * @param addr Address to check
 * @param alignment Alignment boundary
 * @return true if aligned, false otherwise
 */
bool memory_is_aligned(uint64_t addr, uint64_t alignment) {
    if (alignment == 0) return true;
    return (addr & (alignment - 1)) == 0;
}

/**
 * Calculate checksum of memory region
 * @param ptr Memory region
 * @param size Size of region
 * @return Simple checksum value
 */
uint32_t memory_checksum(const void* ptr, size_t size) {
    if (!ptr || size == 0) return 0;
    
    const uint8_t *p = (const uint8_t*)ptr;
    uint32_t checksum = 0;
    
    for (size_t i = 0; i < size; i++) {
        checksum += p[i];
        checksum = (checksum << 1) | (checksum >> 31); // Rotate left
    }
    
    return checksum;
}

/**
 * Print memory region in hex dump format
 * @param ptr Memory region to dump
 * @param size Size of region
 * @param base_addr Base address for display
 */
void memory_dump(const void* ptr, size_t size, uint64_t base_addr) {
    if (!ptr || size == 0) return;
    
    const uint8_t *p = (const uint8_t*)ptr;
    
    KINFO("Memory dump at 0x%016lX (%zu bytes):", base_addr, size);
    
    for (size_t i = 0; i < size; i += 16) {
        // Print address
        printf("0x%016lX: ", base_addr + i);
        
        // Print hex bytes
        for (size_t j = 0; j < 16; j++) {
            if (i + j < size) {
                printf("%02X ", p[i + j]);
            } else {
                printf("   ");
            }
            
            if (j == 7) printf(" "); // Extra space in middle
        }
        
        printf(" |");
        
        // Print ASCII representation
        for (size_t j = 0; j < 16 && i + j < size; j++) {
            uint8_t c = p[i + j];
            if (c >= 32 && c <= 126) {
                printf("%c", c);
            } else {
                printf(".");
            }
        }
        
        printf("|\n");
    }
} 