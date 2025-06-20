/**
 * @file hal.h
 * @brief Hardware Abstraction Layer (HAL) for FG-OS
 * 
 * This file provides a unified interface for accessing hardware functionality
 * across different platforms and architectures. The HAL abstracts platform-specific
 * code and provides a consistent API for higher-level kernel subsystems.
 * 
 * @author Faiz Nasir
 * @company FGCompany Official
 * @version 1.0.0
 * @date 2024
 * @copyright © 2024 FGCompany Official. All rights reserved.
 */

#ifndef __HAL_H__
#define __HAL_H__

#include "../include/types.h"

/**
 * @defgroup hal Hardware Abstraction Layer
 * @brief Hardware abstraction layer for cross-platform support
 * @{
 */

// HAL version information
#define HAL_VERSION_MAJOR   1
#define HAL_VERSION_MINOR   0
#define HAL_VERSION_PATCH   0
#define HAL_VERSION_STRING  "1.0.0"

// Platform identification
typedef enum {
    HAL_PLATFORM_UNKNOWN = 0,
    HAL_PLATFORM_X86_64,
    HAL_PLATFORM_X86_32,
    HAL_PLATFORM_ARM64,
    HAL_PLATFORM_ARM32,
    HAL_PLATFORM_RISCV64,
    HAL_PLATFORM_RISCV32
} hal_platform_t;

// Architecture features
typedef enum {
    HAL_FEATURE_MMU         = (1 << 0),   /**< Memory Management Unit */
    HAL_FEATURE_FPU         = (1 << 1),   /**< Floating Point Unit */
    HAL_FEATURE_SIMD        = (1 << 2),   /**< SIMD Instructions */
    HAL_FEATURE_MULTICORE   = (1 << 3),   /**< Multi-core support */
    HAL_FEATURE_VIRTUALIZATION = (1 << 4), /**< Virtualization support */
    HAL_FEATURE_ATOMIC      = (1 << 5),   /**< Atomic operations */
    HAL_FEATURE_CACHE       = (1 << 6),   /**< Cache management */
    HAL_FEATURE_DMA         = (1 << 7),   /**< DMA support */
    HAL_FEATURE_IOMMU       = (1 << 8),   /**< IOMMU support */
    HAL_FEATURE_SECURE_BOOT = (1 << 9)    /**< Secure boot */
} hal_feature_flags_t;

// HAL subsystem status
typedef enum {
    HAL_STATUS_UNINITIALIZED = 0,
    HAL_STATUS_INITIALIZING,
    HAL_STATUS_READY,
    HAL_STATUS_ERROR,
    HAL_STATUS_DEGRADED,
    HAL_STATUS_SHUTDOWN
} hal_status_t;

// CPU information structure
typedef struct {
    char        vendor_id[16];      /**< CPU vendor identification */
    char        brand_string[64];   /**< CPU brand string */
    uint32_t    family;             /**< CPU family */
    uint32_t    model;              /**< CPU model */
    uint32_t    stepping;           /**< CPU stepping */
    uint32_t    cache_line_size;    /**< Cache line size in bytes */
    uint32_t    l1_cache_size;      /**< L1 cache size in KB */
    uint32_t    l2_cache_size;      /**< L2 cache size in KB */
    uint32_t    l3_cache_size;      /**< L3 cache size in KB */
    uint32_t    core_count;         /**< Number of CPU cores */
    uint32_t    thread_count;       /**< Number of hardware threads */
    uint64_t    frequency_mhz;      /**< CPU frequency in MHz */
    uint32_t    features;           /**< CPU feature flags */
} hal_cpu_info_t;

// Memory information structure
typedef struct {
    uint64_t    total_physical;     /**< Total physical memory in bytes */
    uint64_t    available_physical; /**< Available physical memory */
    uint64_t    total_virtual;      /**< Total virtual memory space */
    uint64_t    kernel_reserved;    /**< Memory reserved for kernel */
    uint64_t    page_size;          /**< System page size */
    uint64_t    large_page_size;    /**< Large page size (if supported) */
    uint32_t    memory_type_count;  /**< Number of memory type regions */
    struct {
        uint64_t base;              /**< Base address */
        uint64_t length;            /**< Length in bytes */
        uint32_t type;              /**< Memory type */
        uint32_t attributes;        /**< Memory attributes */
    } memory_map[64];               /**< Memory map entries */
} hal_memory_info_t;

// Timer information structure
typedef struct {
    uint64_t    frequency_hz;       /**< Timer frequency in Hz */
    uint64_t    resolution_ns;      /**< Timer resolution in nanoseconds */
    uint32_t    capabilities;       /**< Timer capabilities */
    bool        high_precision;     /**< High precision timer available */
    bool        monotonic;          /**< Monotonic timer available */
    bool        real_time;          /**< Real-time clock available */
} hal_timer_info_t;

// Interrupt controller information
typedef struct {
    uint32_t    max_vectors;        /**< Maximum interrupt vectors */
    uint32_t    active_vectors;     /**< Currently active vectors */
    uint32_t    controller_type;    /**< Interrupt controller type */
    bool        msi_support;        /**< MSI support available */
    bool        msix_support;       /**< MSI-X support available */
    bool        level_triggered;    /**< Level-triggered interrupts */
    bool        edge_triggered;     /**< Edge-triggered interrupts */
} hal_interrupt_info_t;

// System information structure
typedef struct {
    hal_platform_t      platform;      /**< Platform type */
    uint32_t            features;       /**< Available features */
    hal_cpu_info_t      cpu;           /**< CPU information */
    hal_memory_info_t   memory;        /**< Memory information */
    hal_timer_info_t    timer;         /**< Timer information */
    hal_interrupt_info_t interrupt;    /**< Interrupt information */
    uint64_t            uptime_ms;     /**< System uptime in milliseconds */
    uint64_t            boot_time;     /**< Boot timestamp */
} hal_system_info_t;

// Performance counters
typedef struct {
    uint64_t    cpu_cycles;         /**< CPU cycles elapsed */
    uint64_t    instructions;       /**< Instructions executed */
    uint64_t    cache_hits;         /**< Cache hits */
    uint64_t    cache_misses;       /**< Cache misses */
    uint64_t    tlb_hits;           /**< TLB hits */
    uint64_t    tlb_misses;         /**< TLB misses */
    uint64_t    context_switches;   /**< Context switches */
    uint64_t    page_faults;        /**< Page faults */
    uint64_t    interrupts;         /**< Interrupt count */
    uint64_t    system_calls;       /**< System call count */
} hal_performance_counters_t;

// HAL statistics
typedef struct {
    uint64_t    initialization_time_us; /**< Initialization time in microseconds */
    uint64_t    api_calls_total;        /**< Total HAL API calls */
    uint64_t    api_calls_per_second;   /**< API calls per second */
    uint64_t    memory_allocations;     /**< Memory allocations through HAL */
    uint64_t    io_operations;          /**< I/O operations */
    uint64_t    dma_transfers;          /**< DMA transfers */
    uint64_t    cache_operations;       /**< Cache operations */
    uint64_t    atomic_operations;      /**< Atomic operations */
    uint32_t    error_count;            /**< Error count */
    uint32_t    warning_count;          /**< Warning count */
} hal_statistics_t;

// HAL function pointers for platform-specific implementations
typedef struct {
    // Initialization and cleanup
    int (*init)(void);
    void (*shutdown)(void);
    int (*reset)(void);
    
    // CPU operations
    void (*cpu_halt)(void);
    void (*cpu_pause)(void);
    uint64_t (*cpu_get_cycles)(void);
    uint32_t (*cpu_get_id)(void);
    
    // Memory operations
    void* (*memory_map)(uint64_t physical, size_t size, uint32_t flags);
    void (*memory_unmap)(void* virtual_addr, size_t size);
    void (*memory_barrier)(void);
    void (*cache_flush)(void* addr, size_t size);
    void (*cache_invalidate)(void* addr, size_t size);
    
    // Interrupt operations
    void (*interrupt_enable)(void);
    void (*interrupt_disable)(void);
    uint64_t (*interrupt_save_disable)(void);
    void (*interrupt_restore)(uint64_t flags);
    int (*interrupt_register)(uint32_t vector, void (*handler)(void));
    void (*interrupt_unregister)(uint32_t vector);
    
    // Timer operations
    uint64_t (*timer_get_ticks)(void);
    uint64_t (*timer_get_frequency)(void);
    void (*timer_delay)(uint64_t microseconds);
    
    // Atomic operations
    uint32_t (*atomic_add)(volatile uint32_t* ptr, uint32_t value);
    uint32_t (*atomic_sub)(volatile uint32_t* ptr, uint32_t value);
    uint32_t (*atomic_exchange)(volatile uint32_t* ptr, uint32_t value);
    bool (*atomic_compare_exchange)(volatile uint32_t* ptr, uint32_t expected, uint32_t desired);
    
    // I/O operations
    uint8_t (*io_read_8)(uint16_t port);
    uint16_t (*io_read_16)(uint16_t port);
    uint32_t (*io_read_32)(uint16_t port);
    void (*io_write_8)(uint16_t port, uint8_t value);
    void (*io_write_16)(uint16_t port, uint16_t value);
    void (*io_write_32)(uint16_t port, uint32_t value);
    
    // DMA operations
    int (*dma_allocate_channel)(uint32_t* channel);
    void (*dma_free_channel)(uint32_t channel);
    int (*dma_transfer)(uint32_t channel, void* dest, void* src, size_t size);
    
    // Power management
    void (*power_suspend)(void);
    void (*power_resume)(void);
    void (*power_shutdown)(void);
    void (*power_set_frequency)(uint32_t frequency_mhz);
} hal_operations_t;

// Global HAL state
extern hal_status_t hal_status;
extern hal_system_info_t hal_system_info;
extern hal_statistics_t hal_statistics;
extern hal_performance_counters_t hal_performance;
extern hal_operations_t hal_ops;

// Core HAL functions
int hal_initialize(void);
void hal_shutdown(void);
hal_status_t hal_get_status(void);
const char* hal_get_version(void);
hal_platform_t hal_get_platform(void);
uint32_t hal_get_features(void);

// System information functions
int hal_get_system_info(hal_system_info_t* info);
int hal_get_cpu_info(hal_cpu_info_t* info);
int hal_get_memory_info(hal_memory_info_t* info);
int hal_get_timer_info(hal_timer_info_t* info);
int hal_get_interrupt_info(hal_interrupt_info_t* info);

// Performance and statistics
int hal_get_performance_counters(hal_performance_counters_t* counters);
int hal_get_statistics(hal_statistics_t* stats);
void hal_reset_statistics(void);
void hal_reset_performance_counters(void);

// Hardware detection
int hal_detect_hardware(void);
int hal_enumerate_devices(void);
int hal_probe_capabilities(void);

// Resource management
int hal_allocate_resource(uint32_t type, uint64_t size, void** resource);
void hal_free_resource(void* resource);
int hal_reserve_memory_region(uint64_t base, uint64_t size);
void hal_release_memory_region(uint64_t base, uint64_t size);

// Integration testing
int hal_run_self_test(void);
int hal_run_hardware_test(void);
int hal_run_performance_test(void);
int hal_run_stress_test(uint32_t duration_seconds);

// Diagnostic and debugging
void hal_dump_system_info(void);
void hal_dump_performance_info(void);
void hal_dump_statistics(void);
void hal_dump_memory_map(void);

// Utility functions
const char* hal_platform_to_string(hal_platform_t platform);
const char* hal_status_to_string(hal_status_t status);
const char* hal_feature_to_string(hal_feature_flags_t feature);
bool hal_is_feature_supported(hal_feature_flags_t feature);

// Platform-specific initialization (implemented by each platform)
int hal_platform_init(void);
void hal_platform_shutdown(void);
int hal_platform_detect(hal_platform_t* platform);
int hal_platform_get_operations(hal_operations_t* ops);

/** @} */

#endif /* __HAL_H__ */ 