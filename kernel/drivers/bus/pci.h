/**
 * @file pci.h
 * @brief PCI Bus Driver for FG-OS
 * 
 * This file provides PCI (Peripheral Component Interconnect) bus management
 * including device enumeration, configuration space access, and device
 * identification for the device driver framework.
 * 
 * @author Faiz Nasir
 * @company FGCompany Official
 * @version 1.0.0
 * @date 2024
 * @copyright Copyright (c) 2024 FGCompany Official. All rights reserved.
 */

#ifndef __PCI_H__
#define __PCI_H__

#include <types.h>
#include "../device.h"

/**
 * @brief PCI Configuration Space Registers
 */
#define PCI_CONFIG_VENDOR_ID        0x00    /**< Vendor ID */
#define PCI_CONFIG_DEVICE_ID        0x02    /**< Device ID */
#define PCI_CONFIG_COMMAND          0x04    /**< Command register */
#define PCI_CONFIG_STATUS           0x06    /**< Status register */
#define PCI_CONFIG_REVISION_ID      0x08    /**< Revision ID */
#define PCI_CONFIG_PROG_IF          0x09    /**< Programming Interface */
#define PCI_CONFIG_SUBCLASS         0x0A    /**< Subclass code */
#define PCI_CONFIG_CLASS            0x0B    /**< Class code */
#define PCI_CONFIG_CACHE_LINE_SIZE  0x0C    /**< Cache line size */
#define PCI_CONFIG_LATENCY_TIMER    0x0D    /**< Latency timer */
#define PCI_CONFIG_HEADER_TYPE      0x0E    /**< Header type */
#define PCI_CONFIG_BIST             0x0F    /**< Built-in self test */
#define PCI_CONFIG_BAR0             0x10    /**< Base Address Register 0 */
#define PCI_CONFIG_BAR1             0x14    /**< Base Address Register 1 */
#define PCI_CONFIG_BAR2             0x18    /**< Base Address Register 2 */
#define PCI_CONFIG_BAR3             0x1C    /**< Base Address Register 3 */
#define PCI_CONFIG_BAR4             0x20    /**< Base Address Register 4 */
#define PCI_CONFIG_BAR5             0x24    /**< Base Address Register 5 */
#define PCI_CONFIG_CARDBUS_CIS      0x28    /**< CardBus CIS pointer */
#define PCI_CONFIG_SUBSYSTEM_VID    0x2C    /**< Subsystem vendor ID */
#define PCI_CONFIG_SUBSYSTEM_ID     0x2E    /**< Subsystem ID */
#define PCI_CONFIG_ROM_ADDRESS      0x30    /**< Expansion ROM base address */
#define PCI_CONFIG_CAPABILITIES     0x34    /**< Capabilities pointer */
#define PCI_CONFIG_INTERRUPT_LINE   0x3C    /**< Interrupt line */
#define PCI_CONFIG_INTERRUPT_PIN    0x3D    /**< Interrupt pin */
#define PCI_CONFIG_MIN_GRANT        0x3E    /**< Min_Gnt */
#define PCI_CONFIG_MAX_LATENCY      0x3F    /**< Max_Lat */

/**
 * @brief PCI Command Register Bits
 */
#define PCI_COMMAND_IO              (1 << 0)    /**< I/O Space Enable */
#define PCI_COMMAND_MEMORY          (1 << 1)    /**< Memory Space Enable */
#define PCI_COMMAND_MASTER          (1 << 2)    /**< Bus Master Enable */
#define PCI_COMMAND_SPECIAL         (1 << 3)    /**< Special Cycles Enable */
#define PCI_COMMAND_INVALIDATE      (1 << 4)    /**< Memory Write and Invalidate */
#define PCI_COMMAND_VGA_PALETTE     (1 << 5)    /**< VGA Palette Snoop */
#define PCI_COMMAND_PARITY          (1 << 6)    /**< Parity Error Response */
#define PCI_COMMAND_WAIT            (1 << 7)    /**< Address/Data Stepping */
#define PCI_COMMAND_SERR            (1 << 8)    /**< SERR# Enable */
#define PCI_COMMAND_FAST_BACK       (1 << 9)    /**< Fast Back-to-Back Enable */
#define PCI_COMMAND_INTX_DISABLE    (1 << 10)   /**< INTx Emulation Disable */

/**
 * @brief PCI Status Register Bits
 */
#define PCI_STATUS_INTERRUPT        (1 << 3)    /**< Interrupt Status */
#define PCI_STATUS_CAP_LIST         (1 << 4)    /**< Capabilities List */
#define PCI_STATUS_66MHZ            (1 << 5)    /**< 66 MHz Capable */
#define PCI_STATUS_UDF              (1 << 6)    /**< User Definable Features */
#define PCI_STATUS_FAST_BACK        (1 << 7)    /**< Fast Back-to-Back Capable */
#define PCI_STATUS_PARITY           (1 << 8)    /**< Data Parity Detected */
#define PCI_STATUS_DEVSEL_MASK      0x0600      /**< DEVSEL Timing */
#define PCI_STATUS_DEVSEL_FAST      0x0000      /**< Fast DEVSEL */
#define PCI_STATUS_DEVSEL_MEDIUM    0x0200      /**< Medium DEVSEL */
#define PCI_STATUS_DEVSEL_SLOW      0x0400      /**< Slow DEVSEL */
#define PCI_STATUS_SIG_TARGET_ABORT (1 << 11)   /**< Signaled Target Abort */
#define PCI_STATUS_REC_TARGET_ABORT (1 << 12)   /**< Received Target Abort */
#define PCI_STATUS_REC_MASTER_ABORT (1 << 13)   /**< Received Master Abort */
#define PCI_STATUS_SIG_SYSTEM_ERROR (1 << 14)   /**< Signaled System Error */
#define PCI_STATUS_DETECTED_PARITY  (1 << 15)   /**< Detected Parity Error */

/**
 * @brief PCI Header Types
 */
#define PCI_HEADER_TYPE_NORMAL      0x00    /**< Normal device */
#define PCI_HEADER_TYPE_BRIDGE      0x01    /**< PCI-to-PCI bridge */
#define PCI_HEADER_TYPE_CARDBUS     0x02    /**< CardBus bridge */
#define PCI_HEADER_TYPE_MULTIFUNCTION 0x80  /**< Multi-function device */

/**
 * @brief PCI Class Codes (Major Classes)
 */
#define PCI_CLASS_NOT_DEFINED       0x00    /**< Device built before class codes */
#define PCI_CLASS_MASS_STORAGE       0x01    /**< Mass storage controller */
#define PCI_CLASS_NETWORK            0x02    /**< Network controller */
#define PCI_CLASS_DISPLAY            0x03    /**< Display controller */
#define PCI_CLASS_MULTIMEDIA         0x04    /**< Multimedia controller */
#define PCI_CLASS_MEMORY             0x05    /**< Memory controller */
#define PCI_CLASS_BRIDGE             0x06    /**< Bridge device */
#define PCI_CLASS_COMMUNICATION      0x07    /**< Communication controller */
#define PCI_CLASS_SYSTEM             0x08    /**< Generic system peripheral */
#define PCI_CLASS_INPUT              0x09    /**< Input device controller */
#define PCI_CLASS_DOCKING            0x0A    /**< Docking station */
#define PCI_CLASS_PROCESSOR          0x0B    /**< Processor */
#define PCI_CLASS_SERIAL             0x0C    /**< Serial bus controller */
#define PCI_CLASS_WIRELESS           0x0D    /**< Wireless controller */
#define PCI_CLASS_INTELLIGENT        0x0E    /**< Intelligent controller */
#define PCI_CLASS_SATELLITE          0x0F    /**< Satellite communications */
#define PCI_CLASS_ENCRYPTION         0x10    /**< Encryption controller */
#define PCI_CLASS_SIGNAL             0x11    /**< Signal processing controller */
#define PCI_CLASS_PROCESSING         0x12    /**< Processing accelerators */
#define PCI_CLASS_NON_ESSENTIAL      0x13    /**< Non-Essential Instrumentation */
#define PCI_CLASS_COPROCESSOR        0x40    /**< Coprocessor */
#define PCI_CLASS_UNKNOWN            0xFF    /**< Unassigned class */

/**
 * @brief PCI Base Address Register (BAR) types
 */
#define PCI_BAR_TYPE_MEMORY         0x00    /**< Memory BAR */
#define PCI_BAR_TYPE_IO             0x01    /**< I/O BAR */
#define PCI_BAR_MEMORY_TYPE_32      0x00    /**< 32-bit memory */
#define PCI_BAR_MEMORY_TYPE_64      0x04    /**< 64-bit memory */
#define PCI_BAR_MEMORY_PREFETCHABLE 0x08    /**< Prefetchable memory */

/**
 * @brief PCI device location structure
 */
typedef struct {
    uint8_t bus;                    /**< PCI bus number */
    uint8_t device;                 /**< PCI device number */
    uint8_t function;               /**< PCI function number */
} pci_location_t;

/**
 * @brief PCI device configuration structure
 */
typedef struct {
    uint16_t vendor_id;             /**< Vendor ID */
    uint16_t device_id;             /**< Device ID */
    uint16_t command;               /**< Command register */
    uint16_t status;                /**< Status register */
    uint8_t  revision_id;           /**< Revision ID */
    uint8_t  prog_if;               /**< Programming Interface */
    uint8_t  subclass;              /**< Subclass code */
    uint8_t  class_code;            /**< Class code */
    uint8_t  cache_line_size;       /**< Cache line size */
    uint8_t  latency_timer;         /**< Latency timer */
    uint8_t  header_type;           /**< Header type */
    uint8_t  bist;                  /**< Built-in self test */
    uint32_t bar[6];                /**< Base Address Registers */
    uint32_t cardbus_cis;           /**< CardBus CIS pointer */
    uint16_t subsystem_vendor_id;   /**< Subsystem vendor ID */
    uint16_t subsystem_id;          /**< Subsystem ID */
    uint32_t rom_address;           /**< Expansion ROM base address */
    uint8_t  capabilities_ptr;      /**< Capabilities pointer */
    uint8_t  interrupt_line;        /**< Interrupt line */
    uint8_t  interrupt_pin;         /**< Interrupt pin */
    uint8_t  min_grant;             /**< Min_Gnt */
    uint8_t  max_latency;           /**< Max_Lat */
} pci_config_t;

/**
 * @brief PCI device structure
 */
typedef struct pci_device {
    device_t            device;         /**< Base device structure */
    pci_location_t      location;       /**< PCI location */
    pci_config_t        config;         /**< PCI configuration */
    uint32_t            bar_sizes[6];   /**< BAR sizes */
    bool                bar_is_io[6];   /**< BAR type (I/O or memory) */
    bool                bar_is_64bit[6]; /**< 64-bit BAR flag */
    struct pci_device*  next;           /**< Next PCI device */
} pci_device_t;

/**
 * @brief PCI bus statistics
 */
typedef struct {
    uint32_t total_buses;           /**< Total number of buses */
    uint32_t total_devices;         /**< Total number of devices */
    uint32_t total_functions;       /**< Total number of functions */
    uint32_t devices_by_class[256]; /**< Device count by class */
    uint32_t config_reads;          /**< Configuration reads */
    uint32_t config_writes;         /**< Configuration writes */
    uint32_t enumeration_time_ms;   /**< Enumeration time in milliseconds */
} pci_bus_stats_t;

// Function declarations

/**
 * @brief Initialize the PCI bus driver
 * 
 * @return 0 on success, negative error code on failure
 */
int pci_init(void);

/**
 * @brief Shutdown the PCI bus driver
 */
void pci_shutdown(void);

/**
 * @brief Enumerate all PCI devices
 * 
 * Scans all PCI buses and devices, registering them with the device framework
 * 
 * @return Number of devices found
 */
uint32_t pci_enumerate_devices(void);

/**
 * @brief Read from PCI configuration space
 * 
 * @param location PCI device location
 * @param offset Configuration space offset
 * @param size Read size (1, 2, or 4 bytes)
 * @return Configuration space value
 */
uint32_t pci_config_read(pci_location_t location, uint8_t offset, uint8_t size);

/**
 * @brief Write to PCI configuration space
 * 
 * @param location PCI device location
 * @param offset Configuration space offset
 * @param value Value to write
 * @param size Write size (1, 2, or 4 bytes)
 */
void pci_config_write(pci_location_t location, uint8_t offset, uint32_t value, uint8_t size);

/**
 * @brief Read 8-bit value from PCI configuration space
 * 
 * @param location PCI device location
 * @param offset Configuration space offset
 * @return 8-bit value
 */
uint8_t pci_config_read8(pci_location_t location, uint8_t offset);

/**
 * @brief Read 16-bit value from PCI configuration space
 * 
 * @param location PCI device location
 * @param offset Configuration space offset
 * @return 16-bit value
 */
uint16_t pci_config_read16(pci_location_t location, uint8_t offset);

/**
 * @brief Read 32-bit value from PCI configuration space
 * 
 * @param location PCI device location
 * @param offset Configuration space offset
 * @return 32-bit value
 */
uint32_t pci_config_read32(pci_location_t location, uint8_t offset);

/**
 * @brief Write 8-bit value to PCI configuration space
 * 
 * @param location PCI device location
 * @param offset Configuration space offset
 * @param value Value to write
 */
void pci_config_write8(pci_location_t location, uint8_t offset, uint8_t value);

/**
 * @brief Write 16-bit value to PCI configuration space
 * 
 * @param location PCI device location
 * @param offset Configuration space offset
 * @param value Value to write
 */
void pci_config_write16(pci_location_t location, uint8_t offset, uint16_t value);

/**
 * @brief Write 32-bit value to PCI configuration space
 * 
 * @param location PCI device location
 * @param offset Configuration space offset
 * @param value Value to write
 */
void pci_config_write32(pci_location_t location, uint8_t offset, uint32_t value);

/**
 * @brief Find PCI device by vendor and device ID
 * 
 * @param vendor_id Vendor ID
 * @param device_id Device ID
 * @return Pointer to PCI device structure, NULL if not found
 */
pci_device_t* pci_find_device(uint16_t vendor_id, uint16_t device_id);

/**
 * @brief Find PCI devices by class code
 * 
 * @param class_code Class code to search for
 * @param devices Array to store device pointers
 * @param max_devices Maximum number of devices to return
 * @return Number of devices found
 */
uint32_t pci_find_devices_by_class(uint8_t class_code, pci_device_t** devices, uint32_t max_devices);

/**
 * @brief Enable PCI device
 * 
 * @param device Pointer to PCI device
 * @return 0 on success, negative error code on failure
 */
int pci_enable_device(pci_device_t* device);

/**
 * @brief Disable PCI device
 * 
 * @param device Pointer to PCI device
 * @return 0 on success, negative error code on failure
 */
int pci_disable_device(pci_device_t* device);

/**
 * @brief Set PCI device as bus master
 * 
 * @param device Pointer to PCI device
 * @param enable Enable or disable bus master
 * @return 0 on success, negative error code on failure
 */
int pci_set_master(pci_device_t* device, bool enable);

/**
 * @brief Get PCI device BAR address
 * 
 * @param device Pointer to PCI device
 * @param bar_index BAR index (0-5)
 * @return BAR address, 0 if invalid
 */
uint64_t pci_get_bar_address(pci_device_t* device, uint8_t bar_index);

/**
 * @brief Get PCI device BAR size
 * 
 * @param device Pointer to PCI device
 * @param bar_index BAR index (0-5)
 * @return BAR size, 0 if invalid
 */
uint32_t pci_get_bar_size(pci_device_t* device, uint8_t bar_index);

/**
 * @brief Check if PCI device BAR is I/O space
 * 
 * @param device Pointer to PCI device
 * @param bar_index BAR index (0-5)
 * @return true if I/O space, false if memory space
 */
bool pci_bar_is_io(pci_device_t* device, uint8_t bar_index);

/**
 * @brief Check if PCI device BAR is 64-bit
 * 
 * @param device Pointer to PCI device
 * @param bar_index BAR index (0-5)
 * @return true if 64-bit, false if 32-bit
 */
bool pci_bar_is_64bit(pci_device_t* device, uint8_t bar_index);

/**
 * @brief Allocate PCI device resources
 * 
 * @param device Pointer to PCI device
 * @return 0 on success, negative error code on failure
 */
int pci_allocate_resources(pci_device_t* device);

/**
 * @brief Free PCI device resources
 * 
 * @param device Pointer to PCI device
 */
void pci_free_resources(pci_device_t* device);

/**
 * @brief Get PCI bus statistics
 * 
 * @return Pointer to PCI bus statistics structure
 */
const pci_bus_stats_t* pci_get_stats(void);

/**
 * @brief Reset PCI bus statistics
 */
void pci_reset_stats(void);

/**
 * @brief Get PCI class name
 * 
 * @param class_code PCI class code
 * @return Class name string
 */
const char* pci_get_class_name(uint8_t class_code);

/**
 * @brief Get PCI vendor name
 * 
 * @param vendor_id Vendor ID
 * @return Vendor name string
 */
const char* pci_get_vendor_name(uint16_t vendor_id);

/**
 * @brief Dump PCI device information
 * 
 * @param device Pointer to PCI device (NULL for all devices)
 */
void pci_dump_device_info(pci_device_t* device);

/**
 * @brief Enumerate PCI devices with callback
 * 
 * @param callback Callback function called for each device
 * @param user_data User data passed to callback
 */
void pci_enumerate(void (*callback)(pci_device_t* device, void* user_data), void* user_data);

#endif /* __PCI_H__ */ 