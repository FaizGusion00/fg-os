/*
 * FG-OS Physical Memory Manager (PMM)
 * Phase 5: Memory Management Implementation
 * 
 * Developed by: Faiz Nasir
 * Company: FGCompany Official
 * 
 * Physical memory management implementation with page frame allocation.
 */

#include <kernel.h>
#include <types.h>
#include "../mm/memory.h"

// Physical memory statistics
static struct memory_stats pmm_stats = {0};

// Page frame bitmap for tracking allocated pages
static uint8_t *page_bitmap = NULL;
static uint64_t total_pages = 0;
static uint64_t free_pages = 0;
static uint64_t bitmap_size = 0;

// Memory regions detected during boot
static struct memory_region memory_regions[32];
static size_t region_count = 0;

// Forward declarations for internal functions
static void pmm_mark_page_used(uint64_t page_number);
static void pmm_mark_page_free(uint64_t page_number);
static bool pmm_is_page_free(uint64_t page_number);

/**
 * Initialize the Physical Memory Manager
 * @param regions Array of memory regions detected at boot
 * @param count Number of memory regions
 * @return 0 on success, negative error code on failure
 */
int pmm_init(struct memory_region *regions, size_t count) {
    if (!regions || count == 0) {
        KERROR("PMM: Invalid memory regions provided");
        return -1;
    }
    
    KINFO("Initializing Physical Memory Manager...");
    
    // Copy memory regions
    region_count = count > 32 ? 32 : count;
    for (size_t i = 0; i < region_count; i++) {
        memory_regions[i] = regions[i];
        KINFO("PMM: Region %zu: 0x%016lX - 0x%016lX (%s)", 
              i, regions[i].start, regions[i].end,
              regions[i].type == MEMORY_TYPE_AVAILABLE ? "Available" : "Reserved");
    }
    
    // Calculate total memory and page count
    uint64_t total_memory = 0;
    for (size_t i = 0; i < region_count; i++) {
        if (memory_regions[i].type == MEMORY_TYPE_AVAILABLE) {
            total_memory += memory_regions[i].size;
        }
    }
    
    total_pages = total_memory / PAGE_SIZE;
    bitmap_size = (total_pages + 7) / 8; // Round up to nearest byte
    free_pages = 0;
    
    KINFO("PMM: Total memory: %lu MB (%lu pages)", 
          total_memory / (1024 * 1024), total_pages);
    
    // Phase 5: For testing, use a simple approach
    // Allocate bitmap at a fixed location
    page_bitmap = (uint8_t*)0x100000; // 1MB mark
    
    if (!page_bitmap) {
        KERROR("PMM: Cannot allocate page bitmap");
        return -1;
    }
    
    // Initialize bitmap (all pages marked as used initially)
    memory_set(page_bitmap, 0xFF, bitmap_size);
    
    // Mark available pages as free
    for (size_t i = 0; i < region_count; i++) {
        if (memory_regions[i].type == MEMORY_TYPE_AVAILABLE) {
            uint64_t start_page = memory_regions[i].start / PAGE_SIZE;
            uint64_t page_count = memory_regions[i].size / PAGE_SIZE;
            
            for (uint64_t page = start_page; page < start_page + page_count; page++) {
                pmm_mark_page_free(page);
            }
        }
    }
    
    // Initialize statistics
    pmm_stats.total_physical = total_memory;
    pmm_stats.available_physical = free_pages * PAGE_SIZE;
    pmm_stats.used_physical = (total_pages - free_pages) * PAGE_SIZE;
    
    KINFO("PMM: Initialization complete - %lu pages free, %lu pages used",
          free_pages, total_pages - free_pages);
    
    return 0;
}

/**
 * Mark a page as used in the bitmap
 * @param page_number Physical page number
 */
static void pmm_mark_page_used(uint64_t page_number) {
    if (page_number >= total_pages) return;
    
    uint64_t byte_index = page_number / 8;
    uint8_t bit_index = page_number % 8;
    
    if (!(page_bitmap[byte_index] & (1 << bit_index))) {
        // Page was free, now marking as used
        page_bitmap[byte_index] |= (1 << bit_index);
        free_pages--;
        pmm_stats.used_physical += PAGE_SIZE;
        pmm_stats.available_physical -= PAGE_SIZE;
    }
}

/**
 * Mark a page as free in the bitmap
 * @param page_number Physical page number
 */
static void pmm_mark_page_free(uint64_t page_number) {
    if (page_number >= total_pages) return;
    
    uint64_t byte_index = page_number / 8;
    uint8_t bit_index = page_number % 8;
    
    if (page_bitmap[byte_index] & (1 << bit_index)) {
        // Page was used, now marking as free
        page_bitmap[byte_index] &= ~(1 << bit_index);
        free_pages++;
        pmm_stats.used_physical -= PAGE_SIZE;
        pmm_stats.available_physical += PAGE_SIZE;
    }
}

/**
 * Check if a page is free
 * @param page_number Physical page number
 * @return true if page is free, false if used
 */
static bool pmm_is_page_free(uint64_t page_number) {
    if (page_number >= total_pages) return false;
    
    uint64_t byte_index = page_number / 8;
    uint8_t bit_index = page_number % 8;
    
    return !(page_bitmap[byte_index] & (1 << bit_index));
}

/**
 * Allocate a single physical page
 * @return Physical address of allocated page, or 0 if no pages available
 */
uint64_t pmm_alloc_page(void) {
    if (free_pages == 0) {
        KWARN("PMM: No free pages available");
        return 0;
    }
    
    // Simple linear search for free page
    for (uint64_t page = 0; page < total_pages; page++) {
        if (pmm_is_page_free(page)) {
            pmm_mark_page_used(page);
            pmm_stats.allocations++;
            
            uint64_t physical_addr = page * PAGE_SIZE;
            
            // Zero out the allocated page for security
            memory_set((void*)physical_addr, 0, PAGE_SIZE);
            
            return physical_addr;
        }
    }
    
    KERROR("PMM: Page allocation failed despite having %lu free pages", free_pages);
    return 0;
}

/**
 * Free a single physical page
 * @param page Physical address of page to free
 */
void pmm_free_page(uint64_t page) {
    if (page == 0 || page % PAGE_SIZE != 0) {
        KWARN("PMM: Invalid page address for free: 0x%016lX", page);
        return;
    }
    
    uint64_t page_number = page / PAGE_SIZE;
    if (pmm_is_page_free(page_number)) {
        KWARN("PMM: Attempt to free already free page: 0x%016lX", page);
        return;
    }
    
    pmm_mark_page_free(page_number);
    
    KDEBUG("PMM: Freed page 0x%016lX", page);
}

/**
 * Allocate multiple contiguous physical pages
 * @param count Number of pages to allocate
 * @return Physical address of first allocated page, or 0 if allocation failed
 */
uint64_t pmm_alloc_pages(size_t count) {
    if (count == 0) return 0;
    if (count == 1) return pmm_alloc_page();
    
    if (free_pages < count) {
        KWARN("PMM: Insufficient free pages for allocation (need %zu, have %lu)", 
              count, free_pages);
        return 0;
    }
    
    // Find contiguous free pages
    for (uint64_t start_page = 0; start_page <= total_pages - count; start_page++) {
        bool found_block = true;
        
        // Check if we have 'count' contiguous free pages starting at start_page
        for (size_t i = 0; i < count; i++) {
            if (!pmm_is_page_free(start_page + i)) {
                found_block = false;
                break;
            }
        }
        
        if (found_block) {
            // Allocate all pages in the block
            for (size_t i = 0; i < count; i++) {
                pmm_mark_page_used(start_page + i);
            }
            
            pmm_stats.allocations++;
            uint64_t physical_addr = start_page * PAGE_SIZE;
            
            // Zero out allocated pages
            memory_set((void*)physical_addr, 0, count * PAGE_SIZE);
            
            KDEBUG("PMM: Allocated %zu contiguous pages at 0x%016lX", count, physical_addr);
            return physical_addr;
        }
    }
    
    KWARN("PMM: Could not find %zu contiguous free pages", count);
    return 0;
}

/**
 * Free multiple contiguous physical pages
 * @param start Physical address of first page
 * @param count Number of pages to free
 */
void pmm_free_pages(uint64_t start, size_t count) {
    if (start == 0 || start % PAGE_SIZE != 0 || count == 0) {
        KWARN("PMM: Invalid parameters for free_pages: 0x%016lX, count %zu", start, count);
        return;
    }
    
    for (size_t i = 0; i < count; i++) {
        pmm_free_page(start + (i * PAGE_SIZE));
    }
    
    KDEBUG("PMM: Freed %zu pages starting at 0x%016lX", count, start);
}

/**
 * Get current memory statistics
 * @return Pointer to memory statistics structure
 */
struct memory_stats* get_memory_stats(void) {
    pmm_stats.available_physical = free_pages * PAGE_SIZE;
    pmm_stats.used_physical = (total_pages - free_pages) * PAGE_SIZE;
    return &pmm_stats;
}

/**
 * Print physical memory layout information
 */
void print_physical_memory_layout(void) {
    KINFO("=== Physical Memory Layout ===");
    KINFO("Total Physical Memory: %lu MB", pmm_stats.total_physical / (1024 * 1024));
    KINFO("Available Memory: %lu MB", pmm_stats.available_physical / (1024 * 1024));
    KINFO("Used Memory: %lu MB", pmm_stats.used_physical / (1024 * 1024));
    KINFO("Page Size: %d KB", PAGE_SIZE / 1024);
    KINFO("Total Pages: %lu", total_pages);
    KINFO("Free Pages: %lu", free_pages);
    KINFO("Used Pages: %lu", total_pages - free_pages);
    KINFO("Allocations: %u", pmm_stats.allocations);
    KINFO("================================");
}

/**
 * Print detailed memory statistics
 */
void print_memory_stats(void) {
    print_memory_layout();
    
    KINFO("=== Memory Regions ===");
    for (size_t i = 0; i < region_count; i++) {
        const char *type_str = "Unknown";
        switch (memory_regions[i].type) {
            case MEMORY_TYPE_AVAILABLE: type_str = "Available"; break;
            case MEMORY_TYPE_RESERVED: type_str = "Reserved"; break;
            case MEMORY_TYPE_KERNEL: type_str = "Kernel"; break;
            case MEMORY_TYPE_STACK: type_str = "Stack"; break;
            case MEMORY_TYPE_HEAP: type_str = "Heap"; break;
        }
        
        KINFO("Region %zu: 0x%016lX - 0x%016lX (%s, %lu MB)",
              i, memory_regions[i].start, 
              memory_regions[i].start + memory_regions[i].size,
              type_str, memory_regions[i].size / (1024 * 1024));
    }
    KINFO("======================");
} 