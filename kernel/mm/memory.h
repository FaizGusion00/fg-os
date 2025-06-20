/*
 * FG-OS Memory Management Header
 * Phase 4: Kernel Architecture Design
 * 
 * Developed by: Faiz Nasir
 * Company: FGCompany Official
 * 
 * Memory management architecture and interface definitions.
 */

#ifndef MEMORY_H
#define MEMORY_H

#include <types.h>

// Memory Types
typedef enum {
    MEMORY_TYPE_AVAILABLE = 1,
    MEMORY_TYPE_RESERVED,
    MEMORY_TYPE_ACPI_RECLAIMABLE,
    MEMORY_TYPE_ACPI_NVS,
    MEMORY_TYPE_BAD,
    MEMORY_TYPE_KERNEL,
    MEMORY_TYPE_STACK,
    MEMORY_TYPE_HEAP
} memory_type_t;

// Memory types enum (memory_region struct is in types.h)

// Page Frame Allocator
struct page_frame {
    uint64_t physical_addr;     // Physical address
    uint32_t ref_count;         // Reference count
    uint32_t flags;             // Page flags
    struct page_frame *next;    // Next free page
};

// Virtual Memory Area
struct vm_area {
    uint64_t start;             // Virtual start address
    uint64_t end;               // Virtual end address
    uint32_t flags;             // Protection flags
    uint32_t type;              // Area type
    struct vm_area *next;       // Next area
};

// Memory Statistics
struct memory_stats {
    uint64_t total_physical;    // Total physical memory
    uint64_t available_physical;// Available physical memory
    uint64_t used_physical;     // Used physical memory
    uint64_t total_virtual;     // Total virtual memory
    uint64_t used_virtual;      // Used virtual memory
    uint32_t page_faults;       // Page fault count
    uint32_t allocations;       // Allocation count
};

// Memory Management Functions

// Physical Memory Management
int pmm_init(struct memory_region *regions, size_t count);
uint64_t pmm_alloc_page(void);
void pmm_free_page(uint64_t page);
uint64_t pmm_alloc_pages(size_t count);
void pmm_free_pages(uint64_t start, size_t count);

// Virtual Memory Management
int vmm_init(void);
int vmm_map_page(uint64_t virtual_addr, uint64_t physical_addr, uint32_t flags);
void vmm_unmap_page(uint64_t virtual_addr);
uint64_t vmm_get_physical(uint64_t virtual_addr);
int vmm_protect(uint64_t start, size_t size, uint32_t flags);

// Heap Management
int heap_init(uint64_t start, size_t initial_size);
void* kmalloc(size_t size);
void* kcalloc(size_t count, size_t size);
void* krealloc(void* ptr, size_t size);
void kfree(void* ptr);

// Memory Utilities
void memory_copy(void* dest, const void* src, size_t size);
void memory_set(void* dest, int value, size_t size);
int memory_compare(const void* ptr1, const void* ptr2, size_t size);

// Memory Information
struct memory_stats* get_memory_stats(void);
void print_physical_memory_layout(void);
void print_memory_stats(void);

// Memory Protection Flags
#define MEM_READ            (1 << 0)
#define MEM_WRITE           (1 << 1)
#define MEM_EXECUTE         (1 << 2)
#define MEM_USER            (1 << 3)
#define MEM_CACHED          (1 << 4)
#define MEM_WRITE_THROUGH   (1 << 5)

// Memory Area Types
#define MEM_AREA_KERNEL     1
#define MEM_AREA_USER       2
#define MEM_AREA_STACK      3
#define MEM_AREA_HEAP       4
#define MEM_AREA_SHARED     5

// Memory Allocation Flags
#define KMALLOC_ZERO        (1 << 0)  // Zero-initialize memory
#define KMALLOC_ATOMIC      (1 << 1)  // Atomic allocation
#define KMALLOC_DMA         (1 << 2)  // DMA-capable memory

#endif // MEMORY_H 