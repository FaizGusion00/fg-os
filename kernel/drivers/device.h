/**
 * @file device.h
 * @brief Device Driver Framework for FG-OS
 * 
 * This file provides the core device driver framework including device
 * management, driver registration, hardware abstraction, and device
 * communication interfaces.
 * 
 * @author Faiz Nasir
 * @company FGCompany Official
 * @version 1.0.0
 * @date 2024
 * @copyright Copyright (c) 2024 FGCompany Official. All rights reserved.
 */

#ifndef __DEVICE_H__
#define __DEVICE_H__

#include <types.h>
#include "../interrupt/interrupt.h"

/**
 * @brief Device types
 */
typedef enum {
    DEVICE_TYPE_UNKNOWN = 0,        /**< Unknown device type */
    DEVICE_TYPE_STORAGE,            /**< Storage device (HDD, SSD, etc.) */
    DEVICE_TYPE_NETWORK,            /**< Network interface device */
    DEVICE_TYPE_INPUT,              /**< Input device (keyboard, mouse) */
    DEVICE_TYPE_OUTPUT,             /**< Output device (display, printer) */
    DEVICE_TYPE_AUDIO,              /**< Audio device */
    DEVICE_TYPE_USB,                /**< USB device */
    DEVICE_TYPE_PCI,                /**< PCI device */
    DEVICE_TYPE_SERIAL,             /**< Serial communication device */
    DEVICE_TYPE_PARALLEL,           /**< Parallel communication device */
    DEVICE_TYPE_GRAPHICS,           /**< Graphics device */
    DEVICE_TYPE_SENSOR,             /**< Sensor device */
    DEVICE_TYPE_POWER,              /**< Power management device */
    DEVICE_TYPE_THERMAL,            /**< Thermal management device */
    DEVICE_TYPE_VIRTUAL,            /**< Virtual device */
    DEVICE_TYPE_MAX
} device_type_t;

/**
 * @brief Device states
 */
typedef enum {
    DEVICE_STATE_UNKNOWN = 0,       /**< Unknown state */
    DEVICE_STATE_INITIALIZING,      /**< Device initializing */
    DEVICE_STATE_READY,             /**< Device ready for operation */
    DEVICE_STATE_BUSY,              /**< Device busy */
    DEVICE_STATE_ERROR,             /**< Device in error state */
    DEVICE_STATE_SUSPENDED,         /**< Device suspended */
    DEVICE_STATE_REMOVED,           /**< Device removed */
    DEVICE_STATE_MAX
} device_state_t;

/**
 * @brief Device capability flags
 */
#define DEVICE_CAP_READ             (1 << 0)    /**< Device supports read operations */
#define DEVICE_CAP_WRITE            (1 << 1)    /**< Device supports write operations */
#define DEVICE_CAP_INTERRUPT        (1 << 2)    /**< Device supports interrupts */
#define DEVICE_CAP_DMA              (1 << 3)    /**< Device supports DMA */
#define DEVICE_CAP_HOTPLUG          (1 << 4)    /**< Device supports hot-plug */
#define DEVICE_CAP_POWER_MGMT       (1 << 5)    /**< Device supports power management */
#define DEVICE_CAP_SUSPEND          (1 << 6)    /**< Device supports suspend/resume */
#define DEVICE_CAP_RESET            (1 << 7)    /**< Device supports reset */
#define DEVICE_CAP_CONFIG           (1 << 8)    /**< Device supports configuration */
#define DEVICE_CAP_DIAGNOSTIC       (1 << 9)    /**< Device supports diagnostics */

/**
 * @brief Device I/O request types
 */
typedef enum {
    DEVICE_IO_READ = 0,             /**< Read operation */
    DEVICE_IO_WRITE,                /**< Write operation */
    DEVICE_IO_CONTROL,              /**< Control operation */
    DEVICE_IO_CONFIG,               /**< Configuration operation */
    DEVICE_IO_RESET,                /**< Reset operation */
    DEVICE_IO_DIAGNOSTIC,           /**< Diagnostic operation */
    DEVICE_IO_MAX
} device_io_type_t;

/**
 * @brief Device I/O status
 */
typedef enum {
    DEVICE_IO_PENDING = 0,          /**< I/O operation pending */
    DEVICE_IO_SUCCESS,              /**< I/O operation successful */
    DEVICE_IO_ERROR,                /**< I/O operation failed */
    DEVICE_IO_TIMEOUT,              /**< I/O operation timed out */
    DEVICE_IO_CANCELLED,            /**< I/O operation cancelled */
    DEVICE_IO_MAX
} device_io_status_t;

/**
 * @brief Forward declarations
 */
struct device;
struct device_driver;
struct device_io_request;

/**
 * @brief Device I/O request structure
 */
typedef struct device_io_request {
    uint32_t                request_id;     /**< Unique request ID */
    device_io_type_t        type;           /**< Request type */
    device_io_status_t      status;         /**< Request status */
    struct device*          device;         /**< Target device */
    uint64_t                offset;         /**< Offset for operation */
    void*                   buffer;         /**< Data buffer */
    size_t                  size;           /**< Buffer size */
    uint32_t                flags;          /**< Request flags */
    uint64_t                timestamp;      /**< Request timestamp */
    void*                   private_data;   /**< Driver private data */
    void (*callback)(struct device_io_request*); /**< Completion callback */
    struct device_io_request* next;        /**< Next request in queue */
} device_io_request_t;

/**
 * @brief Device statistics
 */
typedef struct {
    uint64_t    requests_total;         /**< Total requests processed */
    uint64_t    requests_read;          /**< Read requests */
    uint64_t    requests_write;         /**< Write requests */
    uint64_t    requests_control;       /**< Control requests */
    uint64_t    requests_successful;    /**< Successful requests */
    uint64_t    requests_failed;        /**< Failed requests */
    uint64_t    requests_timeout;       /**< Timed out requests */
    uint64_t    bytes_read;             /**< Total bytes read */
    uint64_t    bytes_written;          /**< Total bytes written */
    uint64_t    interrupts_received;    /**< Interrupts received */
    uint64_t    errors_total;           /**< Total errors */
    uint64_t    uptime_ms;              /**< Device uptime in milliseconds */
    uint32_t    avg_request_time_us;    /**< Average request time (microseconds) */
    uint32_t    max_request_time_us;    /**< Maximum request time (microseconds) */
} device_stats_t;

/**
 * @brief Device information structure
 */
typedef struct {
    char        name[64];               /**< Device name */
    char        description[128];       /**< Device description */
    char        manufacturer[64];       /**< Device manufacturer */
    char        model[64];              /**< Device model */
    char        serial_number[32];      /**< Device serial number */
    char        firmware_version[32];   /**< Firmware version */
    uint32_t    vendor_id;              /**< Vendor ID */
    uint32_t    device_id;              /**< Device ID */
    uint32_t    revision;               /**< Device revision */
    uint32_t    class_code;             /**< Device class code */
    uint32_t    subclass_code;          /**< Device subclass code */
} device_info_t;

/**
 * @brief Device configuration structure
 */
typedef struct {
    uint32_t    base_address;           /**< Base I/O address */
    uint32_t    memory_mapped_base;     /**< Memory-mapped I/O base */
    size_t      memory_size;            /**< Memory region size */
    uint8_t     irq_line;               /**< IRQ line number */
    uint8_t     dma_channel;            /**< DMA channel */
    uint32_t    clock_frequency;        /**< Device clock frequency */
    uint32_t    max_transfer_size;      /**< Maximum transfer size */
    uint32_t    alignment_requirement;  /**< Memory alignment requirement */
    uint32_t    timeout_ms;             /**< Operation timeout */
    uint32_t    retry_count;            /**< Retry count for failed operations */
} device_config_t;

/**
 * @brief Device driver operations structure
 */
typedef struct {
    int (*probe)(struct device* dev);                              /**< Probe device */
    int (*init)(struct device* dev);                               /**< Initialize device */
    int (*start)(struct device* dev);                              /**< Start device */
    int (*stop)(struct device* dev);                               /**< Stop device */
    int (*reset)(struct device* dev);                              /**< Reset device */
    int (*suspend)(struct device* dev);                            /**< Suspend device */
    int (*resume)(struct device* dev);                             /**< Resume device */
    int (*remove)(struct device* dev);                             /**< Remove device */
    int (*read)(struct device* dev, device_io_request_t* req);     /**< Read operation */
    int (*write)(struct device* dev, device_io_request_t* req);    /**< Write operation */
    int (*ioctl)(struct device* dev, uint32_t cmd, void* arg);     /**< I/O control */
    void (*interrupt_handler)(struct device* dev, uint32_t vector); /**< Interrupt handler */
} device_driver_ops_t;

/**
 * @brief Device driver structure
 */
typedef struct device_driver {
    char                    name[64];           /**< Driver name */
    char                    version[16];        /**< Driver version */
    char                    author[64];         /**< Driver author */
    device_type_t           supported_types[8]; /**< Supported device types */
    uint32_t                flags;              /**< Driver flags */
    device_driver_ops_t     ops;                /**< Driver operations */
    struct device_driver*   next;               /**< Next driver in list */
    void*                   private_data;       /**< Driver private data */
    uint32_t                reference_count;    /**< Reference count */
    bool                    loaded;             /**< Driver loaded status */
} device_driver_t;

/**
 * @brief Device structure
 */
typedef struct device {
    uint32_t                device_id;          /**< Unique device ID */
    device_type_t           type;               /**< Device type */
    device_state_t          state;              /**< Current device state */
    uint32_t                capabilities;       /**< Device capabilities */
    device_info_t           info;               /**< Device information */
    device_config_t         config;            /**< Device configuration */
    device_stats_t          stats;              /**< Device statistics */
    device_driver_t*        driver;             /**< Associated driver */
    struct device*          parent;             /**< Parent device */
    struct device*          children;           /**< Child devices */
    struct device*          next_sibling;       /**< Next sibling device */
    struct device*          next;               /**< Next device in global list */
    device_io_request_t*    request_queue;      /**< I/O request queue */
    void*                   driver_data;        /**< Driver-specific data */
    uint64_t                created_time;       /**< Device creation timestamp */
    uint64_t                last_access_time;   /**< Last access timestamp */
    bool                    registered;         /**< Device registration status */
} device_t;

/**
 * @brief Device manager statistics
 */
typedef struct {
    uint32_t    total_devices;          /**< Total devices registered */
    uint32_t    active_devices;         /**< Active devices */
    uint32_t    failed_devices;         /**< Failed devices */
    uint32_t    suspended_devices;      /**< Suspended devices */
    uint32_t    total_drivers;          /**< Total drivers registered */
    uint32_t    active_drivers;         /**< Active drivers */
    uint64_t    total_requests;         /**< Total I/O requests */
    uint64_t    pending_requests;       /**< Pending I/O requests */
    uint64_t    completed_requests;     /**< Completed I/O requests */
    uint64_t    failed_requests;        /**< Failed I/O requests */
    uint32_t    devices_by_type[DEVICE_TYPE_MAX]; /**< Device count by type */
} device_manager_stats_t;

// Function declarations

/**
 * @brief Initialize the device driver framework
 * 
 * @return 0 on success, negative error code on failure
 */
int device_init(void);

/**
 * @brief Shutdown the device driver framework
 */
void device_shutdown(void);

/**
 * @brief Register a device driver
 * 
 * @param driver Pointer to device driver structure
 * @return 0 on success, negative error code on failure
 */
int device_driver_register(device_driver_t* driver);

/**
 * @brief Unregister a device driver
 * 
 * @param driver Pointer to device driver structure
 * @return 0 on success, negative error code on failure
 */
int device_driver_unregister(device_driver_t* driver);

/**
 * @brief Find a device driver by name
 * 
 * @param name Driver name
 * @return Pointer to driver structure, NULL if not found
 */
device_driver_t* device_driver_find(const char* name);

/**
 * @brief Register a device
 * 
 * @param device Pointer to device structure
 * @return 0 on success, negative error code on failure
 */
int device_register(device_t* device);

/**
 * @brief Unregister a device
 * 
 * @param device Pointer to device structure
 * @return 0 on success, negative error code on failure
 */
int device_unregister(device_t* device);

/**
 * @brief Find a device by ID
 * 
 * @param device_id Device ID
 * @return Pointer to device structure, NULL if not found
 */
device_t* device_find_by_id(uint32_t device_id);

/**
 * @brief Find devices by type
 * 
 * @param type Device type
 * @param devices Array to store device pointers
 * @param max_devices Maximum number of devices to return
 * @return Number of devices found
 */
uint32_t device_find_by_type(device_type_t type, device_t** devices, uint32_t max_devices);

/**
 * @brief Start a device
 * 
 * @param device Pointer to device structure
 * @return 0 on success, negative error code on failure
 */
int device_start(device_t* device);

/**
 * @brief Stop a device
 * 
 * @param device Pointer to device structure
 * @return 0 on success, negative error code on failure
 */
int device_stop(device_t* device);

/**
 * @brief Reset a device
 * 
 * @param device Pointer to device structure
 * @return 0 on success, negative error code on failure
 */
int device_reset(device_t* device);

/**
 * @brief Suspend a device
 * 
 * @param device Pointer to device structure
 * @return 0 on success, negative error code on failure
 */
int device_suspend(device_t* device);

/**
 * @brief Resume a device
 * 
 * @param device Pointer to device structure
 * @return 0 on success, negative error code on failure
 */
int device_resume(device_t* device);

/**
 * @brief Submit an I/O request to a device
 * 
 * @param device Pointer to device structure
 * @param request Pointer to I/O request structure
 * @return 0 on success, negative error code on failure
 */
int device_submit_request(device_t* device, device_io_request_t* request);

/**
 * @brief Read data from a device
 * 
 * @param device Pointer to device structure
 * @param offset Offset to read from
 * @param buffer Buffer to store data
 * @param size Number of bytes to read
 * @return Number of bytes read, negative error code on failure
 */
ssize_t device_read(device_t* device, uint64_t offset, void* buffer, size_t size);

/**
 * @brief Write data to a device
 * 
 * @param device Pointer to device structure
 * @param offset Offset to write to
 * @param buffer Buffer containing data to write
 * @param size Number of bytes to write
 * @return Number of bytes written, negative error code on failure
 */
ssize_t device_write(device_t* device, uint64_t offset, const void* buffer, size_t size);

/**
 * @brief Perform device I/O control operation
 * 
 * @param device Pointer to device structure
 * @param cmd Control command
 * @param arg Command argument
 * @return 0 on success, negative error code on failure
 */
int device_ioctl(device_t* device, uint32_t cmd, void* arg);

/**
 * @brief Get device information
 * 
 * @param device Pointer to device structure
 * @return Pointer to device information structure
 */
const device_info_t* device_get_info(device_t* device);

/**
 * @brief Get device statistics
 * 
 * @param device Pointer to device structure
 * @return Pointer to device statistics structure
 */
const device_stats_t* device_get_stats(device_t* device);

/**
 * @brief Get device manager statistics
 * 
 * @return Pointer to device manager statistics structure
 */
const device_manager_stats_t* device_get_manager_stats(void);

/**
 * @brief Reset device statistics
 * 
 * @param device Pointer to device structure
 */
void device_reset_stats(device_t* device);

/**
 * @brief Set device state
 * 
 * @param device Pointer to device structure
 * @param state New device state
 */
void device_set_state(device_t* device, device_state_t state);

/**
 * @brief Get device state
 * 
 * @param device Pointer to device structure
 * @return Current device state
 */
device_state_t device_get_state(device_t* device);

/**
 * @brief Check if device supports capability
 * 
 * @param device Pointer to device structure
 * @param capability Capability flag to check
 * @return true if supported, false otherwise
 */
bool device_has_capability(device_t* device, uint32_t capability);

/**
 * @brief Create a new I/O request
 * 
 * @param type Request type
 * @param offset Offset for operation
 * @param buffer Data buffer
 * @param size Buffer size
 * @param callback Completion callback
 * @return Pointer to I/O request structure, NULL on failure
 */
device_io_request_t* device_create_request(device_io_type_t type, uint64_t offset, 
                                          void* buffer, size_t size, 
                                          void (*callback)(device_io_request_t*));

/**
 * @brief Free an I/O request
 * 
 * @param request Pointer to I/O request structure
 */
void device_free_request(device_io_request_t* request);

/**
 * @brief Process pending I/O requests for all devices
 * 
 * This function should be called periodically to process pending requests
 */
void device_process_requests(void);

/**
 * @brief Handle device interrupt
 * 
 * @param device Pointer to device structure
 * @param vector Interrupt vector
 */
void device_handle_interrupt(device_t* device, uint32_t vector);

/**
 * @brief Enumerate all devices
 * 
 * @param callback Callback function called for each device
 * @param user_data User data passed to callback
 */
void device_enumerate(void (*callback)(device_t* device, void* user_data), void* user_data);

/**
 * @brief Dump device information for debugging
 * 
 * @param device Pointer to device structure (NULL for all devices)
 */
void device_dump_info(device_t* device);

/**
 * @brief Get device type name string
 * 
 * @param type Device type
 * @return Device type name string
 */
const char* device_type_to_string(device_type_t type);

/**
 * @brief Get device state name string
 * 
 * @param state Device state
 * @return Device state name string
 */
const char* device_state_to_string(device_state_t state);

#endif /* __DEVICE_H__ */ 