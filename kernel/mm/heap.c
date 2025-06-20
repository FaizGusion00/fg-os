/*
 * FG-OS Kernel Heap Allocator
 * Phase 5: Memory Management Implementation
 * 
 * Developed by: Faiz Nasir
 * Company: FGCompany Official
 * 
 * Simple kernel heap implementation with block allocation.
 */

#include <kernel.h>
#include <types.h>
#include "../mm/memory.h"

// Heap block header
struct heap_block {
    size_t size;                // Size of the block (including header)
    bool allocated;             // true if allocated, false if free
    struct heap_block *next;    // Next block in the list
    struct heap_block *prev;    // Previous block in the list
    uint32_t magic;            // Magic number for corruption detection
};

#define HEAP_MAGIC 0xDEADBEEF
#define HEAP_BLOCK_HEADER_SIZE sizeof(struct heap_block)
#define HEAP_MIN_BLOCK_SIZE 64
#define HEAP_ALIGNMENT 16

// Heap management structure
static struct {
    uint64_t start;             // Heap start address
    uint64_t end;               // Heap end address
    size_t size;                // Total heap size
    size_t used;                // Used heap size
    size_t free;                // Free heap size
    struct heap_block *first;   // First block
    uint32_t allocations;       // Total allocation count
    uint32_t frees;             // Total free count
} heap_info = {0};

// Forward declarations
static struct heap_block* find_free_block(size_t size);
static void split_block(struct heap_block *block, size_t size);
static void merge_blocks(struct heap_block *block);
static bool is_valid_block(struct heap_block *block);

/**
 * Initialize the kernel heap
 * @param start Starting address of heap
 * @param initial_size Initial heap size
 * @return 0 on success, negative error code on failure
 */
int heap_init(uint64_t start, size_t initial_size) {
    KINFO("Initializing kernel heap...");
    
    if (initial_size < PAGE_SIZE) {
        initial_size = PAGE_SIZE;
    }
    
    // Align size to page boundary
    initial_size = (initial_size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    
    heap_info.start = start;
    heap_info.end = start + initial_size;
    heap_info.size = initial_size;
    heap_info.used = 0;
    heap_info.free = initial_size;
    heap_info.allocations = 0;
    heap_info.frees = 0;
    
    // Create first free block
    heap_info.first = (struct heap_block*)start;
    heap_info.first->size = initial_size;
    heap_info.first->allocated = false;
    heap_info.first->next = NULL;
    heap_info.first->prev = NULL;
    heap_info.first->magic = HEAP_MAGIC;
    
    KINFO("Heap: Initialized at 0x%016lX, size %zu bytes", start, initial_size);
    return 0;
}

/**
 * Allocate memory from kernel heap
 * @param size Size in bytes to allocate
 * @return Pointer to allocated memory, or NULL on failure
 */
void* kmalloc(size_t size) {
    if (size == 0) return NULL;
    
    // Align size to HEAP_ALIGNMENT
    size = (size + HEAP_ALIGNMENT - 1) & ~(HEAP_ALIGNMENT - 1);
    
    // Add header size
    size_t total_size = size + HEAP_BLOCK_HEADER_SIZE;
    
    // Find free block
    struct heap_block *block = find_free_block(total_size);
    if (!block) {
        KWARN("kmalloc: No suitable free block found for %zu bytes", size);
        return NULL;
    }
    
    // Split block if it's too large
    if (block->size > total_size + HEAP_MIN_BLOCK_SIZE) {
        split_block(block, total_size);
    }
    
    // Mark block as allocated
    block->allocated = true;
    
    // Update statistics
    heap_info.used += block->size;
    heap_info.free -= block->size;
    heap_info.allocations++;
    
    // Return pointer after header
    void *ptr = (void*)((uint64_t)block + HEAP_BLOCK_HEADER_SIZE);
    
    KDEBUG("kmalloc: Allocated %zu bytes at %p", size, ptr);
    return ptr;
}

/**
 * Allocate zeroed memory from kernel heap
 * @param count Number of elements
 * @param size Size of each element
 * @return Pointer to allocated memory, or NULL on failure
 */
void* kcalloc(size_t count, size_t size) {
    size_t total_size = count * size;
    void *ptr = kmalloc(total_size);
    
    if (ptr) {
        memory_set(ptr, 0, total_size);
    }
    
    return ptr;
}

/**
 * Reallocate memory block
 * @param ptr Pointer to existing block
 * @param size New size
 * @return Pointer to reallocated memory, or NULL on failure
 */
void* krealloc(void* ptr, size_t size) {
    if (!ptr) return kmalloc(size);
    if (size == 0) {
        kfree(ptr);
        return NULL;
    }
    
    // Get block header
    struct heap_block *block = (struct heap_block*)((uint64_t)ptr - HEAP_BLOCK_HEADER_SIZE);
    
    if (!is_valid_block(block)) {
        KERROR("krealloc: Invalid block pointer %p", ptr);
        return NULL;
    }
    
    size_t old_size = block->size - HEAP_BLOCK_HEADER_SIZE;
    
    // If new size fits in current block, just return
    if (size <= old_size) {
        return ptr;
    }
    
    // Allocate new block
    void *new_ptr = kmalloc(size);
    if (!new_ptr) {
        return NULL;
    }
    
    // Copy old data
    memory_copy(new_ptr, ptr, old_size);
    
    // Free old block
    kfree(ptr);
    
    return new_ptr;
}

/**
 * Free memory block
 * @param ptr Pointer to memory to free
 */
void kfree(void* ptr) {
    if (!ptr) return;
    
    // Get block header
    struct heap_block *block = (struct heap_block*)((uint64_t)ptr - HEAP_BLOCK_HEADER_SIZE);
    
    if (!is_valid_block(block)) {
        KERROR("kfree: Invalid block pointer %p", ptr);
        return;
    }
    
    if (!block->allocated) {
        KWARN("kfree: Attempt to free already free block %p", ptr);
        return;
    }
    
    // Mark block as free
    block->allocated = false;
    
    // Update statistics
    heap_info.used -= block->size;
    heap_info.free += block->size;
    heap_info.frees++;
    
    // Merge with adjacent free blocks
    merge_blocks(block);
    
    KDEBUG("kfree: Freed block at %p", ptr);
}

/**
 * Find a free block of at least the specified size
 * @param size Minimum size required
 * @return Pointer to free block, or NULL if none found
 */
static struct heap_block* find_free_block(size_t size) {
    struct heap_block *block = heap_info.first;
    
    while (block) {
        if (!block->allocated && block->size >= size) {
            return block;
        }
        block = block->next;
    }
    
    return NULL;
}

/**
 * Split a block into two if it's larger than needed
 * @param block Block to split
 * @param size Size for the first block
 */
static void split_block(struct heap_block *block, size_t size) {
    if (block->size <= size + HEAP_MIN_BLOCK_SIZE) {
        return; // Not worth splitting
    }
    
    // Create new block for the remaining space
    struct heap_block *new_block = (struct heap_block*)((uint64_t)block + size);
    new_block->size = block->size - size;
    new_block->allocated = false;
    new_block->next = block->next;
    new_block->prev = block;
    new_block->magic = HEAP_MAGIC;
    
    // Update links
    if (block->next) {
        block->next->prev = new_block;
    }
    block->next = new_block;
    block->size = size;
}

/**
 * Merge a block with adjacent free blocks
 * @param block Block to merge
 */
static void merge_blocks(struct heap_block *block) {
    // Merge with next block if it's free
    while (block->next && !block->next->allocated) {
        struct heap_block *next = block->next;
        block->size += next->size;
        block->next = next->next;
        if (next->next) {
            next->next->prev = block;
        }
    }
    
    // Merge with previous block if it's free
    while (block->prev && !block->prev->allocated) {
        struct heap_block *prev = block->prev;
        prev->size += block->size;
        prev->next = block->next;
        if (block->next) {
            block->next->prev = prev;
        }
        block = prev;
    }
}

/**
 * Check if a block is valid
 * @param block Block to check
 * @return true if valid, false otherwise
 */
static bool is_valid_block(struct heap_block *block) {
    if (!block) return false;
    if (block->magic != HEAP_MAGIC) return false;
    if ((uint64_t)block < heap_info.start || (uint64_t)block >= heap_info.end) return false;
    return true;
}

/**
 * Print heap statistics
 */
void print_heap_stats(void) {
    KINFO("=== Kernel Heap Statistics ===");
    KINFO("Heap Start: 0x%016lX", heap_info.start);
    KINFO("Heap End: 0x%016lX", heap_info.end);
    KINFO("Total Size: %zu bytes (%zu KB)", heap_info.size, heap_info.size / 1024);
    KINFO("Used: %zu bytes (%zu KB)", heap_info.used, heap_info.used / 1024);
    KINFO("Free: %zu bytes (%zu KB)", heap_info.free, heap_info.free / 1024);
    KINFO("Utilization: %zu%%", (heap_info.used * 100) / heap_info.size);
    KINFO("Allocations: %u", heap_info.allocations);
    KINFO("Frees: %u", heap_info.frees);
    KINFO("==============================");
} 