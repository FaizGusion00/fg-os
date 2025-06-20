/*
 * FG-OS Phase 5 Memory Management Tests
 * 
 * Developed by: Faiz Nasir
 * Company: FGCompany Official
 * 
 * Comprehensive tests for memory management implementation.
 */

#include "../../tests/include/test_framework.h"
#include "../../kernel/mm/memory.h"
#include <kernel.h>

// Test fixtures
static struct memory_region test_regions[4];
static bool memory_initialized = false;

/**
 * Set up test environment
 */
static void setup_memory_tests(void) {
    if (memory_initialized) return;
    
    // Create test memory regions
    test_regions[0].start = 0x100000;   // 1MB
    test_regions[0].size = 0x800000;    // 8MB
    test_regions[0].type = MEMORY_TYPE_AVAILABLE;
    
    test_regions[1].start = 0x1000000;  // 16MB
    test_regions[1].size = 0x1000000;   // 16MB
    test_regions[1].type = MEMORY_TYPE_AVAILABLE;
    
    test_regions[2].start = 0x3000000;  // 48MB
    test_regions[2].size = 0x2000000;   // 32MB
    test_regions[2].type = MEMORY_TYPE_AVAILABLE;
    
    test_regions[3].start = 0x6000000;  // 96MB
    test_regions[3].size = 0x1000000;   // 16MB
    test_regions[3].type = MEMORY_TYPE_RESERVED;
    
    memory_initialized = true;
}

/**
 * Test Physical Memory Manager (PMM) initialization
 */
static void test_pmm_init(void) {
    TEST_CASE("PMM Initialization");
    
    setup_memory_tests();
    
    // Test valid initialization
    int result = pmm_init(test_regions, 3); // Only available regions
    ASSERT_EQ(result, 0, "PMM initialization should succeed");
    
    // Test invalid parameters
    result = pmm_init(NULL, 0);
    ASSERT_NE(result, 0, "PMM should fail with NULL regions");
    
    TEST_PASS();
}

/**
 * Test Physical Memory Manager allocation
 */
static void test_pmm_allocation(void) {
    TEST_CASE("PMM Page Allocation");
    
    // Test single page allocation
    uint64_t page1 = pmm_alloc_page();
    ASSERT_NE(page1, 0, "Should allocate a page");
    ASSERT_EQ(page1 % PAGE_SIZE, 0, "Page should be aligned");
    
    // Test multiple page allocation
    uint64_t pages = pmm_alloc_pages(4);
    ASSERT_NE(pages, 0, "Should allocate multiple pages");
    ASSERT_EQ(pages % PAGE_SIZE, 0, "Pages should be aligned");
    
    // Test allocation limits
    struct memory_stats *stats = get_memory_stats();
    ASSERT_GT(stats->total_physical, 0, "Should have total memory");
    ASSERT_GT(stats->available_physical, 0, "Should have available memory");
    
    // Free allocated pages
    pmm_free_page(page1);
    pmm_free_pages(pages, 4);
    
    TEST_PASS();
}

/**
 * Test Virtual Memory Manager (VMM) initialization
 */
static void test_vmm_init(void) {
    TEST_CASE("VMM Initialization");
    
    int result = vmm_init();
    ASSERT_EQ(result, 0, "VMM initialization should succeed");
    
    TEST_PASS();
}

/**
 * Test Virtual Memory Manager page mapping
 */
static void test_vmm_mapping(void) {
    TEST_CASE("VMM Page Mapping");
    
    uint64_t virtual_addr = 0x40000000;  // 1GB
    uint64_t physical_addr = pmm_alloc_page();
    ASSERT_NE(physical_addr, 0, "Should allocate physical page");
    
    // Test page mapping
    int result = vmm_map_page(virtual_addr, physical_addr, PTE_PRESENT | PTE_WRITABLE);
    ASSERT_EQ(result, 0, "Page mapping should succeed");
    
    // Test virtual to physical translation
    uint64_t translated = vmm_get_physical(virtual_addr);
    ASSERT_EQ(translated, physical_addr, "Translation should match");
    
    // Test page unmapping
    vmm_unmap_page(virtual_addr);
    uint64_t after_unmap = vmm_get_physical(virtual_addr);
    ASSERT_EQ(after_unmap, 0, "Page should be unmapped");
    
    pmm_free_page(physical_addr);
    
    TEST_PASS();
}

/**
 * Test Kernel Heap allocation
 */
static void test_heap_allocation(void) {
    TEST_CASE("Kernel Heap Allocation");
    
    // Initialize heap
    int result = heap_init(KERNEL_HEAP_START, 0x100000); // 1MB heap
    ASSERT_EQ(result, 0, "Heap initialization should succeed");
    
    // Test basic allocation
    void *ptr1 = kmalloc(256);
    ASSERT_NE(ptr1, NULL, "Should allocate 256 bytes");
    
    void *ptr2 = kmalloc(512);
    ASSERT_NE(ptr2, NULL, "Should allocate 512 bytes");
    
    // Test calloc
    void *ptr3 = kcalloc(10, 32);
    ASSERT_NE(ptr3, NULL, "Should allocate zeroed memory");
    
    // Verify zeroed memory
    uint8_t *data = (uint8_t*)ptr3;
    bool is_zero = true;
    for (size_t i = 0; i < 320; i++) {
        if (data[i] != 0) {
            is_zero = false;
            break;
        }
    }
    ASSERT_TRUE(is_zero, "Calloc should return zeroed memory");
    
    // Test realloc
    void *ptr4 = krealloc(ptr1, 1024);
    ASSERT_NE(ptr4, NULL, "Should reallocate to larger size");
    
    // Free memory
    kfree(ptr2);
    kfree(ptr3);
    kfree(ptr4);
    
    TEST_PASS();
}

/**
 * Test memory utility functions
 */
static void test_memory_utils(void) {
    TEST_CASE("Memory Utility Functions");
    
    uint8_t src[64];
    uint8_t dest[64];
    
    // Initialize source data
    for (int i = 0; i < 64; i++) {
        src[i] = i;
    }
    
    // Test memory_set
    memory_set(dest, 0xFF, 64);
    bool all_set = true;
    for (int i = 0; i < 64; i++) {
        if (dest[i] != 0xFF) {
            all_set = false;
            break;
        }
    }
    ASSERT_TRUE(all_set, "memory_set should set all bytes");
    
    // Test memory_copy
    memory_copy(dest, src, 64);
    int cmp_result = memory_compare(dest, src, 64);
    ASSERT_EQ(cmp_result, 0, "Copied memory should match source");
    
    // Test memory alignment
    uint64_t addr = 0x12345;
    uint64_t aligned_up = memory_align_up(addr, 16);
    uint64_t aligned_down = memory_align_down(addr, 16);
    
    ASSERT_EQ(aligned_up, 0x12350, "Should align up correctly");
    ASSERT_EQ(aligned_down, 0x12340, "Should align down correctly");
    
    bool is_aligned = memory_is_aligned(0x12340, 16);
    ASSERT_TRUE(is_aligned, "Should detect alignment correctly");
    
    TEST_PASS();
}

/**
 * Test memory protection
 */
static void test_memory_protection(void) {
    TEST_CASE("Memory Protection");
    
    uint64_t virtual_addr = 0x50000000;  // 1.25GB
    uint64_t physical_addr = pmm_alloc_page();
    ASSERT_NE(physical_addr, 0, "Should allocate physical page");
    
    // Map page with read-only protection
    int result = vmm_map_page(virtual_addr, physical_addr, PTE_PRESENT);
    ASSERT_EQ(result, 0, "Page mapping should succeed");
    
    // Change protection to read-write
    result = vmm_protect(virtual_addr, PAGE_SIZE, PTE_PRESENT | PTE_WRITABLE);
    ASSERT_EQ(result, 0, "Protection change should succeed");
    
    // Clean up
    vmm_unmap_page(virtual_addr);
    pmm_free_page(physical_addr);
    
    TEST_PASS();
}

/**
 * Test memory fragmentation handling
 */
static void test_memory_fragmentation(void) {
    TEST_CASE("Memory Fragmentation Handling");
    
    void *ptrs[10];
    
    // Allocate several blocks
    for (int i = 0; i < 10; i++) {
        ptrs[i] = kmalloc(128 + (i * 64));
        ASSERT_NE(ptrs[i], NULL, "Should allocate block");
    }
    
    // Free every other block to create fragmentation
    for (int i = 1; i < 10; i += 2) {
        kfree(ptrs[i]);
        ptrs[i] = NULL;
    }
    
    // Try to allocate a larger block
    void *large_ptr = kmalloc(2048);
    ASSERT_NE(large_ptr, NULL, "Should handle fragmentation");
    
    // Clean up
    for (int i = 0; i < 10; i += 2) {
        if (ptrs[i]) kfree(ptrs[i]);
    }
    kfree(large_ptr);
    
    TEST_PASS();
}

/**
 * Run all Phase 5 memory management tests
 */
void run_phase5_memory_tests(void) {
    TEST_SUITE("Phase 5: Memory Management");
    
    test_pmm_init();
    test_pmm_allocation();
    test_vmm_init();
    test_vmm_mapping();
    test_heap_allocation();
    test_memory_utils();
    test_memory_protection();
    test_memory_fragmentation();
    
    TEST_SUITE_END();
} 