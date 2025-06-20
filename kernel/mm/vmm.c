/*
 * FG-OS Virtual Memory Manager (VMM)
 * Phase 5: Memory Management Implementation
 * 
 * Developed by: Faiz Nasir
 * Company: FGCompany Official
 * 
 * Virtual memory management with page table handling.
 */

#include <kernel.h>
#include <types.h>
#include "../mm/memory.h"
#include "../arch/x86_64/arch.h"

// Page table structures
static uint64_t *kernel_pml4 = NULL;
static uint64_t kernel_cr3 = 0;

// Virtual memory areas list
static struct vm_area *vm_areas = NULL;
static uint32_t vm_area_count = 0;

/**
 * Initialize the Virtual Memory Manager
 * @return 0 on success, negative error code on failure
 */
int vmm_init(void) {
    KINFO("Initializing Virtual Memory Manager...");
    
    // Allocate page for PML4 (Page Map Level 4)
    uint64_t pml4_phys = pmm_alloc_page();
    if (pml4_phys == 0) {
        KERROR("VMM: Failed to allocate PML4 page");
        return -1;
    }
    
    kernel_pml4 = (uint64_t*)pml4_phys;
    kernel_cr3 = pml4_phys;
    
    // Initialize PML4 with zeros
    memory_set(kernel_pml4, 0, PAGE_SIZE);
    
    // Identity map first 4GB for kernel
    KINFO("VMM: Setting up identity mapping for kernel...");
    for (uint64_t addr = 0; addr < 0x100000000UL; addr += PAGE_SIZE) {
        int result = vmm_map_page(addr, addr, PTE_PRESENT | PTE_WRITABLE);
        if (result != 0) {
            KERROR("VMM: Failed to identity map page 0x%016lX", addr);
            return -1;
        }
    }
    
    // Map kernel to higher half
    KINFO("VMM: Mapping kernel to higher half...");
    for (uint64_t phys = 0; phys < 0x10000000UL; phys += PAGE_SIZE) {
        uint64_t virt = KERNEL_BASE + phys;
        int result = vmm_map_page(virt, phys, PTE_PRESENT | PTE_WRITABLE);
        if (result != 0) {
            KERROR("VMM: Failed to map kernel page 0x%016lX -> 0x%016lX", virt, phys);
            return -1;
        }
    }
    
    KINFO("VMM: Initialization complete");
    return 0;
}

/**
 * Map a virtual page to a physical page
 * @param virtual_addr Virtual address to map
 * @param physical_addr Physical address to map to
 * @param flags Page table entry flags
 * @return 0 on success, negative error code on failure
 */
int vmm_map_page(uint64_t virtual_addr, uint64_t physical_addr, uint32_t flags) {
    // Extract page table indices from virtual address
    uint64_t pml4_index = (virtual_addr >> 39) & 0x1FF;
    uint64_t pdp_index = (virtual_addr >> 30) & 0x1FF;
    uint64_t pd_index = (virtual_addr >> 21) & 0x1FF;
    uint64_t pt_index = (virtual_addr >> 12) & 0x1FF;
    
    // Get PML4 entry
    uint64_t *pml4 = kernel_pml4;
    if (!(pml4[pml4_index] & PTE_PRESENT)) {
        // Allocate page directory pointer table
        uint64_t pdp_phys = pmm_alloc_page();
        if (pdp_phys == 0) {
            KERROR("VMM: Failed to allocate PDP table");
            return -1;
        }
        pml4[pml4_index] = pdp_phys | PTE_PRESENT | PTE_WRITABLE;
        memory_set((void*)pdp_phys, 0, PAGE_SIZE);
    }
    
    // Get PDP entry
    uint64_t *pdp = (uint64_t*)(pml4[pml4_index] & ~0xFFFUL);
    if (!(pdp[pdp_index] & PTE_PRESENT)) {
        // Allocate page directory
        uint64_t pd_phys = pmm_alloc_page();
        if (pd_phys == 0) {
            KERROR("VMM: Failed to allocate PD table");
            return -1;
        }
        pdp[pdp_index] = pd_phys | PTE_PRESENT | PTE_WRITABLE;
        memory_set((void*)pd_phys, 0, PAGE_SIZE);
    }
    
    // Get PD entry
    uint64_t *pd = (uint64_t*)(pdp[pdp_index] & ~0xFFFUL);
    if (!(pd[pd_index] & PTE_PRESENT)) {
        // Allocate page table
        uint64_t pt_phys = pmm_alloc_page();
        if (pt_phys == 0) {
            KERROR("VMM: Failed to allocate PT table");
            return -1;
        }
        pd[pd_index] = pt_phys | PTE_PRESENT | PTE_WRITABLE;
        memory_set((void*)pt_phys, 0, PAGE_SIZE);
    }
    
    // Set page table entry
    uint64_t *pt = (uint64_t*)(pd[pd_index] & ~0xFFFUL);
    pt[pt_index] = (physical_addr & ~0xFFFUL) | flags;
    
    // Invalidate TLB entry
    arch_invlpg(virtual_addr);
    
    return 0;
}

/**
 * Unmap a virtual page
 * @param virtual_addr Virtual address to unmap
 */
void vmm_unmap_page(uint64_t virtual_addr) {
    // Extract page table indices
    uint64_t pml4_index = (virtual_addr >> 39) & 0x1FF;
    uint64_t pdp_index = (virtual_addr >> 30) & 0x1FF;
    uint64_t pd_index = (virtual_addr >> 21) & 0x1FF;
    uint64_t pt_index = (virtual_addr >> 12) & 0x1FF;
    
    // Navigate page tables
    uint64_t *pml4 = kernel_pml4;
    if (!(pml4[pml4_index] & PTE_PRESENT)) return;
    
    uint64_t *pdp = (uint64_t*)(pml4[pml4_index] & ~0xFFFUL);
    if (!(pdp[pdp_index] & PTE_PRESENT)) return;
    
    uint64_t *pd = (uint64_t*)(pdp[pdp_index] & ~0xFFFUL);
    if (!(pd[pd_index] & PTE_PRESENT)) return;
    
    uint64_t *pt = (uint64_t*)(pd[pd_index] & ~0xFFFUL);
    
    // Clear page table entry
    pt[pt_index] = 0;
    
    // Invalidate TLB entry
    arch_invlpg(virtual_addr);
}

/**
 * Get physical address for a virtual address
 * @param virtual_addr Virtual address to translate
 * @return Physical address, or 0 if not mapped
 */
uint64_t vmm_get_physical(uint64_t virtual_addr) {
    // Extract page table indices
    uint64_t pml4_index = (virtual_addr >> 39) & 0x1FF;
    uint64_t pdp_index = (virtual_addr >> 30) & 0x1FF;
    uint64_t pd_index = (virtual_addr >> 21) & 0x1FF;
    uint64_t pt_index = (virtual_addr >> 12) & 0x1FF;
    uint64_t offset = virtual_addr & 0xFFF;
    
    // Navigate page tables
    uint64_t *pml4 = kernel_pml4;
    if (!(pml4[pml4_index] & PTE_PRESENT)) return 0;
    
    uint64_t *pdp = (uint64_t*)(pml4[pml4_index] & ~0xFFFUL);
    if (!(pdp[pdp_index] & PTE_PRESENT)) return 0;
    
    uint64_t *pd = (uint64_t*)(pdp[pdp_index] & ~0xFFFUL);
    if (!(pd[pd_index] & PTE_PRESENT)) return 0;
    
    uint64_t *pt = (uint64_t*)(pd[pd_index] & ~0xFFFUL);
    if (!(pt[pt_index] & PTE_PRESENT)) return 0;
    
    return (pt[pt_index] & ~0xFFFUL) | offset;
}

/**
 * Change protection flags for a memory region
 * @param start Starting virtual address
 * @param size Size of region in bytes
 * @param flags New protection flags
 * @return 0 on success, negative error code on failure
 */
int vmm_protect(uint64_t start, size_t size, uint32_t flags) {
    uint64_t end = start + size;
    
    // Align to page boundaries
    start = start & ~0xFFFUL;
    end = (end + 0xFFF) & ~0xFFFUL;
    
    for (uint64_t addr = start; addr < end; addr += PAGE_SIZE) {
        uint64_t phys = vmm_get_physical(addr);
        if (phys != 0) {
            // Remap with new flags
            vmm_unmap_page(addr);
            int result = vmm_map_page(addr, phys, flags);
            if (result != 0) {
                KERROR("VMM: Failed to change protection for page 0x%016lX", addr);
                return -1;
            }
        }
    }
    
    return 0;
}

/**
 * Create a new virtual memory area
 * @param start Starting virtual address
 * @param size Size in bytes
 * @param flags Protection flags
 * @param type Area type
 * @return Pointer to VM area, or NULL on failure
 */
struct vm_area* vmm_create_area(uint64_t start, size_t size, uint32_t flags, uint32_t type) {
    // Phase 5: Simple implementation without full VMA management
    // This is a placeholder for more advanced VMA management
    
    KINFO("VMM: Created VMA at 0x%016lX, size %zu bytes, type %u", start, size, type);
    return NULL; // Placeholder
}

/**
 * Destroy a virtual memory area
 * @param area VM area to destroy
 */
void vmm_destroy_area(struct vm_area *area) {
    if (!area) return;
    
    // Phase 5: Placeholder implementation
    KDEBUG("VMM: Destroyed VMA");
} 