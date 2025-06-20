/**
 * @file device.c
 * @brief Device Driver Framework Implementation for FG-OS
 * 
 * This file implements the core device driver framework including device
 * management, driver registration, hardware abstraction, and device
 * communication interfaces.
 * 
 * @author Faiz Nasir
 * @company FGCompany Official
 * @version 1.0.0
 * @date 2024
 * @copyright Copyright (c) 2024 FGCompany Official. All rights reserved.
 */

#include "device.h"
#include "../include/kernel.h"
#include "../interrupt/interrupt.h"
#include "../mm/kmalloc.h"
#include "../src/string_stubs.h"

// Global device framework state
static struct {
    device_t*               device_list;           /**< Global device list */
    device_driver_t*        driver_list;           /**< Global driver list */
    device_manager_stats_t  stats;                 /**< Manager statistics */
    uint32_t                next_device_id;        /**< Next device ID */
    uint32_t                next_request_id;       /**< Next request ID */
    bool                    initialized;           /**< Initialization flag */
} device_manager = {0};

// Device type name strings
static const char* device_type_names[DEVICE_TYPE_MAX] = {
    "Unknown", "Storage", "Network", "Input", "Output", "Audio",
    "USB", "PCI", "Serial", "Parallel", "Graphics", "Sensor",
    "Power", "Thermal", "Virtual"
};

// Device state name strings
static const char* device_state_names[DEVICE_STATE_MAX] = {
    "Unknown", "Initializing", "Ready", "Busy", "Error", "Suspended", "Removed"
};

// Internal function declarations
static void update_device_stats(device_t* device, device_io_request_t* request);
static void complete_request(device_io_request_t* request, device_io_status_t status);
static int validate_device(device_t* device);
static int validate_driver(device_driver_t* driver);

/**
 * @brief Initialize the device driver framework
 */
int device_init(void)
{
    if (device_manager.initialized) {
        return 0; // Already initialized
    }

    // Initialize manager state
    memset(&device_manager, 0, sizeof(device_manager));
    device_manager.next_device_id = 1;
    device_manager.next_request_id = 1;
    device_manager.initialized = true;

    kprintf(KERN_INFO "Device framework initialized successfully\n");
    return 0;
}

/**
 * @brief Shutdown the device driver framework
 */
void device_shutdown(void)
{
    if (!device_manager.initialized) {
        return;
    }

    // Shutdown all devices
    device_t* device = device_manager.device_list;
    while (device) {
        device_t* next = device->next;
        device_stop(device);
        device_unregister(device);
        device = next;
    }

    // Unregister all drivers
    device_driver_t* driver = device_manager.driver_list;
    while (driver) {
        device_driver_t* next = driver->next;
        device_driver_unregister(driver);
        driver = next;
    }

    device_manager.initialized = false;
    kprintf(KERN_INFO "Device framework shutdown complete\n");
}

/**
 * @brief Register a device driver
 */
int device_driver_register(device_driver_t* driver)
{
    if (!device_manager.initialized) {
        return -ENOTINIT;
    }

    if (!driver || validate_driver(driver) != 0) {
        return -EINVAL;
    }

    // Check if driver already registered
    if (device_driver_find(driver->name)) {
        return -EEXIST;
    }

    // Add to driver list
    driver->next = device_manager.driver_list;
    device_manager.driver_list = driver;
    driver->loaded = true;
    driver->reference_count = 0;

    device_manager.stats.total_drivers++;
    device_manager.stats.active_drivers++;

    kprintf(KERN_INFO "Driver '%s' registered successfully\n", driver->name);
    return 0;
}

/**
 * @brief Unregister a device driver
 */
int device_driver_unregister(device_driver_t* driver)
{
    if (!device_manager.initialized || !driver) {
        return -EINVAL;
    }

    if (driver->reference_count > 0) {
        return -EBUSY;
    }

    // Remove from driver list
    device_driver_t** current = &device_manager.driver_list;
    while (*current) {
        if (*current == driver) {
            *current = driver->next;
            driver->loaded = false;
            device_manager.stats.active_drivers--;
            kprintf(KERN_INFO "Driver '%s' unregistered\n", driver->name);
            return 0;
        }
        current = &(*current)->next;
    }

    return -ENOENT;
}

/**
 * @brief Find a device driver by name
 */
device_driver_t* device_driver_find(const char* name)
{
    if (!device_manager.initialized || !name) {
        return NULL;
    }

    device_driver_t* driver = device_manager.driver_list;
    while (driver) {
        if (strcmp(driver->name, name) == 0) {
            return driver;
        }
        driver = driver->next;
    }

    return NULL;
}

/**
 * @brief Register a device
 */
int device_register(device_t* device)
{
    if (!device_manager.initialized) {
        return -ENOTINIT;
    }

    if (!device || validate_device(device) != 0) {
        return -EINVAL;
    }

    if (device->registered) {
        return -EEXIST;
    }

    // Assign device ID
    device->device_id = device_manager.next_device_id++;
    device->created_time = get_system_time_ms();
    device->last_access_time = device->created_time;
    device->state = DEVICE_STATE_INITIALIZING;

    // Reset statistics
    memset(&device->stats, 0, sizeof(device_stats_t));

    // Add to device list
    device->next = device_manager.device_list;
    device_manager.device_list = device;
    device->registered = true;

    // Update manager statistics
    device_manager.stats.total_devices++;
    device_manager.stats.devices_by_type[device->type]++;

    // Initialize device if driver supports it
    if (device->driver && device->driver->ops.init) {
        int result = device->driver->ops.init(device);
        if (result == 0) {
            device->state = DEVICE_STATE_READY;
            device_manager.stats.active_devices++;
        } else {
            device->state = DEVICE_STATE_ERROR;
            device_manager.stats.failed_devices++;
            kprintf(KERN_ERROR "Device %u initialization failed: %d\n", device->device_id, result);
        }
    }

    kprintf(KERN_INFO "Device %u (%s) registered successfully\n", 
            device->device_id, device->info.name);
    return 0;
}

/**
 * @brief Unregister a device
 */
int device_unregister(device_t* device)
{
    if (!device_manager.initialized || !device || !device->registered) {
        return -EINVAL;
    }

    // Stop device if running
    if (device->state == DEVICE_STATE_READY || device->state == DEVICE_STATE_BUSY) {
        device_stop(device);
    }

    // Remove device if driver supports it
    if (device->driver && device->driver->ops.remove) {
        device->driver->ops.remove(device);
        device->driver->reference_count--;
    }

    // Remove from device list
    device_t** current = &device_manager.device_list;
    while (*current) {
        if (*current == device) {
            *current = device->next;
            device->registered = false;
            device->state = DEVICE_STATE_REMOVED;
            
            device_manager.stats.total_devices--;
            device_manager.stats.devices_by_type[device->type]--;
            
            if (device->state == DEVICE_STATE_READY) {
                device_manager.stats.active_devices--;
            } else if (device->state == DEVICE_STATE_ERROR) {
                device_manager.stats.failed_devices--;
            }
            
            kprintf(KERN_INFO "Device %u unregistered\n", device->device_id);
            return 0;
        }
        current = &(*current)->next;
    }

    return -ENOENT;
}

/**
 * @brief Find a device by ID
 */
device_t* device_find_by_id(uint32_t device_id)
{
    if (!device_manager.initialized) {
        return NULL;
    }

    device_t* device = device_manager.device_list;
    while (device) {
        if (device->device_id == device_id) {
            return device;
        }
        device = device->next;
    }

    return NULL;
}

/**
 * @brief Find devices by type
 */
uint32_t device_find_by_type(device_type_t type, device_t** devices, uint32_t max_devices)
{
    if (!device_manager.initialized || !devices || max_devices == 0) {
        return 0;
    }

    uint32_t count = 0;
    device_t* device = device_manager.device_list;
    
    while (device && count < max_devices) {
        if (device->type == type) {
            devices[count++] = device;
        }
        device = device->next;
    }

    return count;
}

/**
 * @brief Start a device
 */
int device_start(device_t* device)
{
    if (!device || !device->registered) {
        return -EINVAL;
    }

    if (device->state == DEVICE_STATE_READY) {
        return 0; // Already started
    }

    if (device->state != DEVICE_STATE_READY && device->state != DEVICE_STATE_SUSPENDED) {
        return -EINVAL;
    }

    if (device->driver && device->driver->ops.start) {
        int result = device->driver->ops.start(device);
        if (result == 0) {
            device->state = DEVICE_STATE_READY;
            device_manager.stats.active_devices++;
            kprintf(KERN_INFO "Device %u started\n", device->device_id);
        } else {
            device->state = DEVICE_STATE_ERROR;
            device_manager.stats.failed_devices++;
            kprintf(KERN_ERROR "Device %u start failed: %d\n", device->device_id, result);
        }
        return result;
    }

    return -ENOSYS;
}

/**
 * @brief Stop a device
 */
int device_stop(device_t* device)
{
    if (!device || !device->registered) {
        return -EINVAL;
    }

    if (device->state != DEVICE_STATE_READY && device->state != DEVICE_STATE_BUSY) {
        return 0; // Already stopped
    }

    if (device->driver && device->driver->ops.stop) {
        int result = device->driver->ops.stop(device);
        if (result == 0) {
            device->state = DEVICE_STATE_SUSPENDED;
            if (device_manager.stats.active_devices > 0) {
                device_manager.stats.active_devices--;
            }
            device_manager.stats.suspended_devices++;
            kprintf(KERN_INFO "Device %u stopped\n", device->device_id);
        }
        return result;
    }

    return -ENOSYS;
}

/**
 * @brief Reset a device
 */
int device_reset(device_t* device)
{
    if (!device || !device->registered) {
        return -EINVAL;
    }

    if (device->driver && device->driver->ops.reset) {
        int result = device->driver->ops.reset(device);
        if (result == 0) {
            device->state = DEVICE_STATE_READY;
            kprintf(KERN_INFO "Device %u reset\n", device->device_id);
        } else {
            device->state = DEVICE_STATE_ERROR;
            kprintf(KERN_ERROR "Device %u reset failed: %d\n", device->device_id, result);
        }
        return result;
    }

    return -ENOSYS;
}

/**
 * @brief Suspend a device
 */
int device_suspend(device_t* device)
{
    if (!device || !device->registered) {
        return -EINVAL;
    }

    if (device->state == DEVICE_STATE_SUSPENDED) {
        return 0; // Already suspended
    }

    if (!(device->capabilities & DEVICE_CAP_SUSPEND)) {
        return -ENOSYS;
    }

    if (device->driver && device->driver->ops.suspend) {
        int result = device->driver->ops.suspend(device);
        if (result == 0) {
            device->state = DEVICE_STATE_SUSPENDED;
            if (device_manager.stats.active_devices > 0) {
                device_manager.stats.active_devices--;
            }
            device_manager.stats.suspended_devices++;
            kprintf(KERN_INFO "Device %u suspended\n", device->device_id);
        }
        return result;
    }

    return -ENOSYS;
}

/**
 * @brief Resume a device
 */
int device_resume(device_t* device)
{
    if (!device || !device->registered) {
        return -EINVAL;
    }

    if (device->state != DEVICE_STATE_SUSPENDED) {
        return -EINVAL;
    }

    if (device->driver && device->driver->ops.resume) {
        int result = device->driver->ops.resume(device);
        if (result == 0) {
            device->state = DEVICE_STATE_READY;
            device_manager.stats.active_devices++;
            if (device_manager.stats.suspended_devices > 0) {
                device_manager.stats.suspended_devices--;
            }
            kprintf(KERN_INFO "Device %u resumed\n", device->device_id);
        }
        return result;
    }

    return -ENOSYS;
}

/**
 * @brief Submit an I/O request to a device
 */
int device_submit_request(device_t* device, device_io_request_t* request)
{
    if (!device || !request || !device->registered) {
        return -EINVAL;
    }

    if (device->state != DEVICE_STATE_READY) {
        return -ENODEV;
    }

    // Set request properties
    request->request_id = device_manager.next_request_id++;
    request->device = device;
    request->status = DEVICE_IO_PENDING;
    request->timestamp = get_system_time_ms();

    // Add to device request queue
    request->next = device->request_queue;
    device->request_queue = request;

    // Update statistics
    device_manager.stats.total_requests++;
    device_manager.stats.pending_requests++;

    // Process request based on type
    int result = 0;
    switch (request->type) {
        case DEVICE_IO_READ:
            if (device->driver && device->driver->ops.read) {
                result = device->driver->ops.read(device, request);
            } else {
                result = -ENOSYS;
            }
            break;

        case DEVICE_IO_WRITE:
            if (device->driver && device->driver->ops.write) {
                result = device->driver->ops.write(device, request);
            } else {
                result = -ENOSYS;
            }
            break;

        case DEVICE_IO_CONTROL:
            if (device->driver && device->driver->ops.ioctl) {
                result = device->driver->ops.ioctl(device, (uint32_t)(uintptr_t)request->buffer, request->private_data);
            } else {
                result = -ENOSYS;
            }
            break;

        default:
            result = -EINVAL;
            break;
    }

    if (result != 0) {
        complete_request(request, DEVICE_IO_ERROR);
    }

    return result;
}

/**
 * @brief Read data from a device
 */
ssize_t device_read(device_t* device, uint64_t offset, void* buffer, size_t size)
{
    if (!device || !buffer || size == 0) {
        return -EINVAL;
    }

    device_io_request_t* request = device_create_request(DEVICE_IO_READ, offset, buffer, size, NULL);
    if (!request) {
        return -ENOMEM;
    }

    int result = device_submit_request(device, request);
    if (result != 0) {
        device_free_request(request);
        return result;
    }

    // For synchronous operation, wait for completion
    // In a real implementation, this would be a proper wait mechanism
    while (request->status == DEVICE_IO_PENDING) {
        // Yield CPU or sleep briefly
    }

    ssize_t bytes_read = (request->status == DEVICE_IO_SUCCESS) ? (ssize_t)size : -EIO;
    device_free_request(request);
    return bytes_read;
}

/**
 * @brief Write data to a device
 */
ssize_t device_write(device_t* device, uint64_t offset, const void* buffer, size_t size)
{
    if (!device || !buffer || size == 0) {
        return -EINVAL;
    }

    device_io_request_t* request = device_create_request(DEVICE_IO_WRITE, offset, (void*)buffer, size, NULL);
    if (!request) {
        return -ENOMEM;
    }

    int result = device_submit_request(device, request);
    if (result != 0) {
        device_free_request(request);
        return result;
    }

    // For synchronous operation, wait for completion
    while (request->status == DEVICE_IO_PENDING) {
        // Yield CPU or sleep briefly
    }

    ssize_t bytes_written = (request->status == DEVICE_IO_SUCCESS) ? (ssize_t)size : -EIO;
    device_free_request(request);
    return bytes_written;
}

/**
 * @brief Perform device I/O control operation
 */
int device_ioctl(device_t* device, uint32_t cmd, void* arg)
{
    if (!device || !device->registered) {
        return -EINVAL;
    }

    if (device->state != DEVICE_STATE_READY) {
        return -ENODEV;
    }

    if (device->driver && device->driver->ops.ioctl) {
        return device->driver->ops.ioctl(device, cmd, arg);
    }

    return -ENOSYS;
}

/**
 * @brief Get device information
 */
const device_info_t* device_get_info(device_t* device)
{
    if (!device) {
        return NULL;
    }

    device->last_access_time = get_system_time_ms();
    return &device->info;
}

/**
 * @brief Get device statistics
 */
const device_stats_t* device_get_stats(device_t* device)
{
    if (!device) {
        return NULL;
    }

    // Update uptime
    device->stats.uptime_ms = get_system_time_ms() - device->created_time;
    device->last_access_time = get_system_time_ms();
    
    return &device->stats;
}

/**
 * @brief Get device manager statistics
 */
const device_manager_stats_t* device_get_manager_stats(void)
{
    if (!device_manager.initialized) {
        return NULL;
    }

    return &device_manager.stats;
}

/**
 * @brief Reset device statistics
 */
void device_reset_stats(device_t* device)
{
    if (!device) {
        return;
    }

    memset(&device->stats, 0, sizeof(device_stats_t));
    kprintf(KERN_INFO "Device %u statistics reset\n", device->device_id);
}

/**
 * @brief Set device state
 */
void device_set_state(device_t* device, device_state_t state)
{
    if (!device || state >= DEVICE_STATE_MAX) {
        return;
    }

    device_state_t old_state = device->state;
    device->state = state;
    device->last_access_time = get_system_time_ms();

    // Update manager statistics
    if (old_state == DEVICE_STATE_READY && state != DEVICE_STATE_READY) {
        if (device_manager.stats.active_devices > 0) {
            device_manager.stats.active_devices--;
        }
    } else if (old_state != DEVICE_STATE_READY && state == DEVICE_STATE_READY) {
        device_manager.stats.active_devices++;
    }

    if (state == DEVICE_STATE_ERROR) {
        device_manager.stats.failed_devices++;
    } else if (old_state == DEVICE_STATE_ERROR && state != DEVICE_STATE_ERROR) {
        if (device_manager.stats.failed_devices > 0) {
            device_manager.stats.failed_devices--;
        }
    }

    if (state == DEVICE_STATE_SUSPENDED) {
        device_manager.stats.suspended_devices++;
    } else if (old_state == DEVICE_STATE_SUSPENDED && state != DEVICE_STATE_SUSPENDED) {
        if (device_manager.stats.suspended_devices > 0) {
            device_manager.stats.suspended_devices--;
        }
    }
}

/**
 * @brief Get device state
 */
device_state_t device_get_state(device_t* device)
{
    if (!device) {
        return DEVICE_STATE_UNKNOWN;
    }

    device->last_access_time = get_system_time_ms();
    return device->state;
}

/**
 * @brief Check if device supports capability
 */
bool device_has_capability(device_t* device, uint32_t capability)
{
    if (!device) {
        return false;
    }

    return (device->capabilities & capability) != 0;
}

/**
 * @brief Create a new I/O request
 */
device_io_request_t* device_create_request(device_io_type_t type, uint64_t offset, 
                                          void* buffer, size_t size, 
                                          void (*callback)(device_io_request_t*))
{
    device_io_request_t* request = kmalloc(sizeof(device_io_request_t));
    if (!request) {
        return NULL;
    }

    memset(request, 0, sizeof(device_io_request_t));
    request->type = type;
    request->offset = offset;
    request->buffer = buffer;
    request->size = size;
    request->callback = callback;
    request->status = DEVICE_IO_PENDING;

    return request;
}

/**
 * @brief Free an I/O request
 */
void device_free_request(device_io_request_t* request)
{
    if (request) {
        kfree(request);
    }
}

/**
 * @brief Process pending I/O requests for all devices
 */
void device_process_requests(void)
{
    device_t* device = device_manager.device_list;
    
    while (device) {
        device_io_request_t** current = &device->request_queue;
        
        while (*current) {
            device_io_request_t* request = *current;
            
            // Check for timeout
            uint64_t current_time = get_system_time_ms();
            if (current_time - request->timestamp > device->config.timeout_ms) {
                complete_request(request, DEVICE_IO_TIMEOUT);
                *current = request->next;
                continue;
            }
            
            current = &request->next;
        }
        
        device = device->next;
    }
}

/**
 * @brief Handle device interrupt
 */
void device_handle_interrupt(device_t* device, uint32_t vector)
{
    if (!device || !device->registered) {
        return;
    }

    device->stats.interrupts_received++;
    device->last_access_time = get_system_time_ms();

    if (device->driver && device->driver->ops.interrupt_handler) {
        device->driver->ops.interrupt_handler(device, vector);
    }
}

/**
 * @brief Enumerate all devices
 */
void device_enumerate(void (*callback)(device_t* device, void* user_data), void* user_data)
{
    if (!callback || !device_manager.initialized) {
        return;
    }

    device_t* device = device_manager.device_list;
    while (device) {
        callback(device, user_data);
        device = device->next;
    }
}

/**
 * @brief Dump device information for debugging
 */
void device_dump_info(device_t* device)
{
    if (!device_manager.initialized) {
        return;
    }

    if (device) {
        // Dump single device
        kprintf("Device %u:\n", device->device_id);
        kprintf("  Name: %s\n", device->info.name);
        kprintf("  Type: %s\n", device_type_to_string(device->type));
        kprintf("  State: %s\n", device_state_to_string(device->state));
        kprintf("  Driver: %s\n", device->driver ? device->driver->name : "None");
        kprintf("  Capabilities: 0x%08X\n", device->capabilities);
        kprintf("  Requests: %llu total, %llu successful\n", 
                device->stats.requests_total, device->stats.requests_successful);
    } else {
        // Dump all devices
        kprintf("Device Manager Statistics:\n");
        kprintf("  Total devices: %u\n", device_manager.stats.total_devices);
        kprintf("  Active devices: %u\n", device_manager.stats.active_devices);
        kprintf("  Failed devices: %u\n", device_manager.stats.failed_devices);
        kprintf("  Total drivers: %u\n", device_manager.stats.total_drivers);
        kprintf("  Total requests: %llu\n", device_manager.stats.total_requests);
        
        device_t* dev = device_manager.device_list;
        while (dev) {
            device_dump_info(dev);
            dev = dev->next;
        }
    }
}

/**
 * @brief Get device type name string
 */
const char* device_type_to_string(device_type_t type)
{
    if (type >= DEVICE_TYPE_MAX) {
        return "Invalid";
    }
    
    return device_type_names[type];
}

/**
 * @brief Get device state name string
 */
const char* device_state_to_string(device_state_t state)
{
    if (state >= DEVICE_STATE_MAX) {
        return "Invalid";
    }
    
    return device_state_names[state];
}

// Internal functions

/**
 * @brief Update device statistics after request completion
 */
static void update_device_stats(device_t* device, device_io_request_t* request)
{
    if (!device || !request) {
        return;
    }

    device->stats.requests_total++;
    
    switch (request->type) {
        case DEVICE_IO_READ:
            device->stats.requests_read++;
            if (request->status == DEVICE_IO_SUCCESS) {
                device->stats.bytes_read += request->size;
            }
            break;
        case DEVICE_IO_WRITE:
            device->stats.requests_write++;
            if (request->status == DEVICE_IO_SUCCESS) {
                device->stats.bytes_written += request->size;
            }
            break;
        case DEVICE_IO_CONTROL:
            device->stats.requests_control++;
            break;
        default:
            break;
    }

    if (request->status == DEVICE_IO_SUCCESS) {
        device->stats.requests_successful++;
    } else {
        device->stats.requests_failed++;
        device->stats.errors_total++;
    }

    if (request->status == DEVICE_IO_TIMEOUT) {
        device->stats.requests_timeout++;
    }

    // Calculate request time
    uint64_t request_time = get_system_time_ms() - request->timestamp;
    uint32_t request_time_us = (uint32_t)(request_time * 1000);
    
    if (request_time_us > device->stats.max_request_time_us) {
        device->stats.max_request_time_us = request_time_us;
    }

    // Update average (simple moving average)
    if (device->stats.requests_total == 1) {
        device->stats.avg_request_time_us = request_time_us;
    } else {
        device->stats.avg_request_time_us = 
            (device->stats.avg_request_time_us + request_time_us) / 2;
    }
}

/**
 * @brief Complete an I/O request
 */
static void complete_request(device_io_request_t* request, device_io_status_t status)
{
    if (!request) {
        return;
    }

    request->status = status;
    
    // Update statistics
    if (request->device) {
        update_device_stats(request->device, request);
    }

    // Update manager statistics
    if (device_manager.stats.pending_requests > 0) {
        device_manager.stats.pending_requests--;
    }
    
    device_manager.stats.completed_requests++;
    
    if (status != DEVICE_IO_SUCCESS) {
        device_manager.stats.failed_requests++;
    }

    // Call completion callback
    if (request->callback) {
        request->callback(request);
    }
}

/**
 * @brief Validate device structure
 */
static int validate_device(device_t* device)
{
    if (!device) {
        return -EINVAL;
    }

    if (device->type >= DEVICE_TYPE_MAX) {
        return -EINVAL;
    }

    if (strlen(device->info.name) == 0) {
        return -EINVAL;
    }

    return 0;
}

/**
 * @brief Validate driver structure
 */
static int validate_driver(device_driver_t* driver)
{
    if (!driver) {
        return -EINVAL;
    }

    if (strlen(driver->name) == 0) {
        return -EINVAL;
    }

    if (strlen(driver->version) == 0) {
        return -EINVAL;
    }

    // At least one operation must be supported
    if (!driver->ops.probe && !driver->ops.init && !driver->ops.read && 
        !driver->ops.write && !driver->ops.ioctl) {
        return -EINVAL;
    }

    return 0;
} 