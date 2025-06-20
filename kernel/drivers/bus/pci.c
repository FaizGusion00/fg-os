/**
 * @file pci.c
 * @brief PCI Bus Driver Implementation for FG-OS
 * 
 * This file implements PCI (Peripheral Component Interconnect) bus management
 * including device enumeration, configuration space access, and device
 * identification for the device driver framework.
 * 
 * @author Faiz Nasir
 * @company FGCompany Official
 * @version 1.0.0
 * @date 2024
 * @copyright Copyright (c) 2024 FGCompany Official. All rights reserved.
 */

#include "pci.h"
#include "../../include/kernel.h"
#include "../../arch/x86_64/io.h"
#include "../../mm/kmalloc.h"
#include "../../src/string_stubs.h"

// PCI Configuration Mechanism 1 I/O ports
#define PCI_CONFIG_ADDRESS      0xCF8   /**< Configuration address port */
#define PCI_CONFIG_DATA         0xCFC   /**< Configuration data port */

// PCI Configuration Address Register format
#define PCI_CONFIG_ENABLE       (1 << 31)  /**< Enable bit */

// Global PCI state
static struct {
    pci_device_t*       device_list;       /**< List of PCI devices */
    pci_bus_stats_t     stats;             /**< PCI bus statistics */
    bool                initialized;       /**< Initialization flag */
    uint32_t            enumeration_start; /**< Enumeration start time */
} pci_manager = {0};

// PCI vendor names (partial list)
static const struct {
    uint16_t vendor_id;
    const char* name;
} pci_vendors[] = {
    {0x8086, "Intel Corporation"},
    {0x1022, "Advanced Micro Devices"},
    {0x10DE, "NVIDIA Corporation"},
    {0x1002, "Advanced Micro Devices"},
    {0x15AD, "VMware"},
    {0x1234, "QEMU"},
    {0x1AF4, "Red Hat, Inc."},
    {0x1013, "Cirrus Logic"},
    {0x5853, "XenSource, Inc."},
    {0x80EE, "InnoTek Systemberatung GmbH"},
    {0x0000, "Unknown Vendor"}
};

// PCI class names
static const char* pci_class_names[256] = {
    [0x00] = "Unclassified Device",
    [0x01] = "Mass Storage Controller",
    [0x02] = "Network Controller",
    [0x03] = "Display Controller",
    [0x04] = "Multimedia Controller",
    [0x05] = "Memory Controller",
    [0x06] = "Bridge Device",
    [0x07] = "Communication Controller",
    [0x08] = "System Peripheral",
    [0x09] = "Input Device Controller",
    [0x0A] = "Docking Station",
    [0x0B] = "Processor",
    [0x0C] = "Serial Bus Controller",
    [0x0D] = "Wireless Controller",
    [0x0E] = "Intelligent Controller",
    [0x0F] = "Satellite Controller",
    [0x10] = "Encryption Controller",
    [0x11] = "Signal Processing Controller",
    [0x12] = "Processing Accelerator",
    [0x13] = "Non-Essential Instrumentation",
    [0x40] = "Coprocessor",
    [0xFF] = "Unassigned Class"
};

// Internal function declarations
static uint32_t pci_make_address(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset);
static int pci_probe_device(pci_location_t location);
static pci_device_t* pci_create_device(pci_location_t location);
static void pci_init_device_info(pci_device_t* pci_dev);
static void pci_probe_bars(pci_device_t* pci_dev);
static device_type_t pci_class_to_device_type(uint8_t class_code);

/**
 * @brief Initialize the PCI bus driver
 */
int pci_init(void)
{
    if (pci_manager.initialized) {
        return 0; // Already initialized
    }

    // Initialize PCI manager state
    memset(&pci_manager, 0, sizeof(pci_manager));
    pci_manager.initialized = true;
    pci_manager.enumeration_start = get_system_time_ms();

    kprintf(KERN_INFO "PCI bus driver initialized\n");
    return 0;
}

/**
 * @brief Shutdown the PCI bus driver
 */
void pci_shutdown(void)
{
    if (!pci_manager.initialized) {
        return;
    }

    // Free all PCI devices
    pci_device_t* device = pci_manager.device_list;
    while (device) {
        pci_device_t* next = device->next;
        kfree(device);
        device = next;
    }

    pci_manager.initialized = false;
    kprintf(KERN_INFO "PCI bus driver shutdown complete\n");
}

/**
 * @brief Enumerate all PCI devices
 */
uint32_t pci_enumerate_devices(void)
{
    if (!pci_manager.initialized) {
        return 0;
    }

    uint32_t device_count = 0;
    uint32_t start_time = get_system_time_ms();

    kprintf(KERN_INFO "Starting PCI device enumeration...\n");

    // Scan all buses (0-255)
    for (uint16_t bus = 0; bus < 256; bus++) {
        pci_manager.stats.total_buses++;
        
        // Scan all devices on this bus (0-31)
        for (uint8_t device = 0; device < 32; device++) {
            // Check function 0 first
            pci_location_t location = {(uint8_t)bus, device, 0};
            
            uint16_t vendor_id = pci_config_read16(location, PCI_CONFIG_VENDOR_ID);
            if (vendor_id == 0xFFFF || vendor_id == 0x0000) {
                continue; // No device present
            }

            // Device found at function 0
            if (pci_probe_device(location) == 0) {
                device_count++;
            }

            // Check if this is a multi-function device
            uint8_t header_type = pci_config_read8(location, PCI_CONFIG_HEADER_TYPE);
            if (header_type & PCI_HEADER_TYPE_MULTIFUNCTION) {
                // Scan functions 1-7
                for (uint8_t function = 1; function < 8; function++) {
                    location.function = function;
                    
                    vendor_id = pci_config_read16(location, PCI_CONFIG_VENDOR_ID);
                    if (vendor_id != 0xFFFF && vendor_id != 0x0000) {
                        if (pci_probe_device(location) == 0) {
                            device_count++;
                        }
                    }
                }
            }
        }
    }

    uint32_t enumeration_time = get_system_time_ms() - start_time;
    pci_manager.stats.enumeration_time_ms = enumeration_time;

    kprintf(KERN_INFO "PCI enumeration complete: %u devices found in %u ms\n", 
            device_count, enumeration_time);

    return device_count;
}

/**
 * @brief Read from PCI configuration space
 */
uint32_t pci_config_read(pci_location_t location, uint8_t offset, uint8_t size)
{
    if (!pci_manager.initialized || size > 4 || (size & (size - 1)) != 0) {
        return 0xFFFFFFFF;
    }

    // Align offset to size boundary
    offset &= ~(size - 1);

    uint32_t address = pci_make_address(location.bus, location.device, location.function, offset);
    
    // Write address to configuration address port
    outl(PCI_CONFIG_ADDRESS, address);
    
    // Read data from configuration data port
    uint32_t data;
    switch (size) {
        case 1:
            data = inb(PCI_CONFIG_DATA + (offset & 3));
            break;
        case 2:
            data = inw(PCI_CONFIG_DATA + (offset & 2));
            break;
        case 4:
            data = inl(PCI_CONFIG_DATA);
            break;
        default:
            data = 0xFFFFFFFF;
            break;
    }

    pci_manager.stats.config_reads++;
    return data;
}

/**
 * @brief Write to PCI configuration space
 */
void pci_config_write(pci_location_t location, uint8_t offset, uint32_t value, uint8_t size)
{
    if (!pci_manager.initialized || size > 4 || (size & (size - 1)) != 0) {
        return;
    }

    // Align offset to size boundary
    offset &= ~(size - 1);

    uint32_t address = pci_make_address(location.bus, location.device, location.function, offset);
    
    // Write address to configuration address port
    outl(PCI_CONFIG_ADDRESS, address);
    
    // Write data to configuration data port
    switch (size) {
        case 1:
            outb(PCI_CONFIG_DATA + (offset & 3), (uint8_t)value);
            break;
        case 2:
            outw(PCI_CONFIG_DATA + (offset & 2), (uint16_t)value);
            break;
        case 4:
            outl(PCI_CONFIG_DATA, value);
            break;
    }

    pci_manager.stats.config_writes++;
}

/**
 * @brief Read 8-bit value from PCI configuration space
 */
uint8_t pci_config_read8(pci_location_t location, uint8_t offset)
{
    return (uint8_t)pci_config_read(location, offset, 1);
}

/**
 * @brief Read 16-bit value from PCI configuration space
 */
uint16_t pci_config_read16(pci_location_t location, uint8_t offset)
{
    return (uint16_t)pci_config_read(location, offset, 2);
}

/**
 * @brief Read 32-bit value from PCI configuration space
 */
uint32_t pci_config_read32(pci_location_t location, uint8_t offset)
{
    return pci_config_read(location, offset, 4);
}

/**
 * @brief Write 8-bit value to PCI configuration space
 */
void pci_config_write8(pci_location_t location, uint8_t offset, uint8_t value)
{
    pci_config_write(location, offset, value, 1);
}

/**
 * @brief Write 16-bit value to PCI configuration space
 */
void pci_config_write16(pci_location_t location, uint8_t offset, uint16_t value)
{
    pci_config_write(location, offset, value, 2);
}

/**
 * @brief Write 32-bit value to PCI configuration space
 */
void pci_config_write32(pci_location_t location, uint8_t offset, uint32_t value)
{
    pci_config_write(location, offset, value, 4);
}

/**
 * @brief Find PCI device by vendor and device ID
 */
pci_device_t* pci_find_device(uint16_t vendor_id, uint16_t device_id)
{
    if (!pci_manager.initialized) {
        return NULL;
    }

    pci_device_t* device = pci_manager.device_list;
    while (device) {
        if (device->config.vendor_id == vendor_id && device->config.device_id == device_id) {
            return device;
        }
        device = device->next;
    }

    return NULL;
}

/**
 * @brief Find PCI devices by class code
 */
uint32_t pci_find_devices_by_class(uint8_t class_code, pci_device_t** devices, uint32_t max_devices)
{
    if (!pci_manager.initialized || !devices || max_devices == 0) {
        return 0;
    }

    uint32_t count = 0;
    pci_device_t* device = pci_manager.device_list;
    
    while (device && count < max_devices) {
        if (device->config.class_code == class_code) {
            devices[count++] = device;
        }
        device = device->next;
    }

    return count;
}

/**
 * @brief Enable PCI device
 */
int pci_enable_device(pci_device_t* device)
{
    if (!device) {
        return -EINVAL;
    }

    uint16_t command = device->config.command;
    command |= PCI_COMMAND_IO | PCI_COMMAND_MEMORY;
    
    pci_config_write16(device->location, PCI_CONFIG_COMMAND, command);
    device->config.command = command;

    kprintf(KERN_INFO "PCI device %02x:%02x.%x enabled\n", 
            device->location.bus, device->location.device, device->location.function);
    
    return 0;
}

/**
 * @brief Disable PCI device
 */
int pci_disable_device(pci_device_t* device)
{
    if (!device) {
        return -EINVAL;
    }

    uint16_t command = device->config.command;
    command &= ~(PCI_COMMAND_IO | PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER);
    
    pci_config_write16(device->location, PCI_CONFIG_COMMAND, command);
    device->config.command = command;

    kprintf(KERN_INFO "PCI device %02x:%02x.%x disabled\n", 
            device->location.bus, device->location.device, device->location.function);
    
    return 0;
}

/**
 * @brief Set PCI device as bus master
 */
int pci_set_master(pci_device_t* device, bool enable)
{
    if (!device) {
        return -EINVAL;
    }

    uint16_t command = device->config.command;
    
    if (enable) {
        command |= PCI_COMMAND_MASTER;
    } else {
        command &= ~PCI_COMMAND_MASTER;
    }
    
    pci_config_write16(device->location, PCI_CONFIG_COMMAND, command);
    device->config.command = command;

    kprintf(KERN_INFO "PCI device %02x:%02x.%x bus master %s\n", 
            device->location.bus, device->location.device, device->location.function,
            enable ? "enabled" : "disabled");
    
    return 0;
}

/**
 * @brief Get PCI device BAR address
 */
uint64_t pci_get_bar_address(pci_device_t* device, uint8_t bar_index)
{
    if (!device || bar_index >= 6) {
        return 0;
    }

    uint32_t bar = device->config.bar[bar_index];
    
    if (device->bar_is_io[bar_index]) {
        // I/O space BAR
        return bar & ~0x3UL;
    } else {
        // Memory space BAR
        if (device->bar_is_64bit[bar_index] && bar_index < 5) {
            // 64-bit BAR (uses two consecutive BARs)
            uint64_t address = bar & ~0xFUL;
            address |= ((uint64_t)device->config.bar[bar_index + 1]) << 32;
            return address;
        } else {
            // 32-bit BAR
            return bar & ~0xFUL;
        }
    }
}

/**
 * @brief Get PCI device BAR size
 */
uint32_t pci_get_bar_size(pci_device_t* device, uint8_t bar_index)
{
    if (!device || bar_index >= 6) {
        return 0;
    }

    return device->bar_sizes[bar_index];
}

/**
 * @brief Check if PCI device BAR is I/O space
 */
bool pci_bar_is_io(pci_device_t* device, uint8_t bar_index)
{
    if (!device || bar_index >= 6) {
        return false;
    }

    return device->bar_is_io[bar_index];
}

/**
 * @brief Check if PCI device BAR is 64-bit
 */
bool pci_bar_is_64bit(pci_device_t* device, uint8_t bar_index)
{
    if (!device || bar_index >= 6) {
        return false;
    }

    return device->bar_is_64bit[bar_index];
}

/**
 * @brief Allocate PCI device resources
 */
int pci_allocate_resources(pci_device_t* device)
{
    if (!device) {
        return -EINVAL;
    }

    // Enable device I/O and memory access
    return pci_enable_device(device);
}

/**
 * @brief Free PCI device resources
 */
void pci_free_resources(pci_device_t* device)
{
    if (!device) {
        return;
    }

    // Disable device
    pci_disable_device(device);
}

/**
 * @brief Get PCI bus statistics
 */
const pci_bus_stats_t* pci_get_stats(void)
{
    if (!pci_manager.initialized) {
        return NULL;
    }

    return &pci_manager.stats;
}

/**
 * @brief Reset PCI bus statistics
 */
void pci_reset_stats(void)
{
    if (!pci_manager.initialized) {
        return;
    }

    memset(&pci_manager.stats, 0, sizeof(pci_bus_stats_t));
    kprintf(KERN_INFO "PCI bus statistics reset\n");
}

/**
 * @brief Get PCI class name
 */
const char* pci_get_class_name(uint8_t class_code)
{
    const char* name = pci_class_names[class_code];
    return name ? name : "Unknown Class";
}

/**
 * @brief Get PCI vendor name
 */
const char* pci_get_vendor_name(uint16_t vendor_id)
{
    for (size_t i = 0; i < sizeof(pci_vendors) / sizeof(pci_vendors[0]); i++) {
        if (pci_vendors[i].vendor_id == vendor_id) {
            return pci_vendors[i].name;
        }
    }
    
    return "Unknown Vendor";
}

/**
 * @brief Dump PCI device information
 */
void pci_dump_device_info(pci_device_t* device)
{
    if (!pci_manager.initialized) {
        return;
    }

    if (device) {
        // Dump single device
        kprintf("PCI Device %02x:%02x.%x:\n", 
                device->location.bus, device->location.device, device->location.function);
        kprintf("  Vendor: %s (0x%04X)\n", 
                pci_get_vendor_name(device->config.vendor_id), device->config.vendor_id);
        kprintf("  Device ID: 0x%04X\n", device->config.device_id);
        kprintf("  Class: %s (0x%02X)\n", 
                pci_get_class_name(device->config.class_code), device->config.class_code);
        kprintf("  Subclass: 0x%02X\n", device->config.subclass);
        kprintf("  Revision: 0x%02X\n", device->config.revision_id);
        kprintf("  Command: 0x%04X\n", device->config.command);
        kprintf("  Status: 0x%04X\n", device->config.status);
        kprintf("  IRQ Line: %u\n", device->config.interrupt_line);
        
        // Dump BARs
        for (int i = 0; i < 6; i++) {
            if (device->config.bar[i] != 0) {
                uint64_t address = pci_get_bar_address(device, i);
                kprintf("  BAR%d: 0x%016llX (%s, %u bytes)\n", 
                        i, address, 
                        device->bar_is_io[i] ? "I/O" : "Memory",
                        device->bar_sizes[i]);
            }
        }
    } else {
        // Dump all devices
        kprintf("PCI Bus Statistics:\n");
        kprintf("  Total buses: %u\n", pci_manager.stats.total_buses);
        kprintf("  Total devices: %u\n", pci_manager.stats.total_devices);
        kprintf("  Total functions: %u\n", pci_manager.stats.total_functions);
        kprintf("  Config reads: %u\n", pci_manager.stats.config_reads);
        kprintf("  Config writes: %u\n", pci_manager.stats.config_writes);
        kprintf("  Enumeration time: %u ms\n", pci_manager.stats.enumeration_time_ms);
        
        pci_device_t* dev = pci_manager.device_list;
        while (dev) {
            pci_dump_device_info(dev);
            dev = dev->next;
        }
    }
}

/**
 * @brief Enumerate PCI devices with callback
 */
void pci_enumerate(void (*callback)(pci_device_t* device, void* user_data), void* user_data)
{
    if (!callback || !pci_manager.initialized) {
        return;
    }

    pci_device_t* device = pci_manager.device_list;
    while (device) {
        callback(device, user_data);
        device = device->next;
    }
}

// Internal functions

/**
 * @brief Create PCI configuration address
 */
static uint32_t pci_make_address(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset)
{
    return PCI_CONFIG_ENABLE |
           ((uint32_t)bus << 16) |
           ((uint32_t)(device & 0x1F) << 11) |
           ((uint32_t)(function & 0x07) << 8) |
           (offset & 0xFC);
}

/**
 * @brief Probe a PCI device at the given location
 */
static int pci_probe_device(pci_location_t location)
{
    // Read vendor ID to verify device exists
    uint16_t vendor_id = pci_config_read16(location, PCI_CONFIG_VENDOR_ID);
    if (vendor_id == 0xFFFF || vendor_id == 0x0000) {
        return -ENODEV;
    }

    // Create PCI device structure
    pci_device_t* pci_dev = pci_create_device(location);
    if (!pci_dev) {
        return -ENOMEM;
    }

    // Initialize device information
    pci_init_device_info(pci_dev);

    // Probe BARs
    pci_probe_bars(pci_dev);

    // Add to device list
    pci_dev->next = pci_manager.device_list;
    pci_manager.device_list = pci_dev;

    // Update statistics
    pci_manager.stats.total_devices++;
    pci_manager.stats.total_functions++;
    pci_manager.stats.devices_by_class[pci_dev->config.class_code]++;

    // Register with device framework
    device_register(&pci_dev->device);

    kprintf(KERN_DEBUG "PCI device found: %02x:%02x.%x [%04x:%04x] %s\n",
            location.bus, location.device, location.function,
            pci_dev->config.vendor_id, pci_dev->config.device_id,
            pci_get_vendor_name(pci_dev->config.vendor_id));

    return 0;
}

/**
 * @brief Create a new PCI device structure
 */
static pci_device_t* pci_create_device(pci_location_t location)
{
    pci_device_t* pci_dev = kmalloc(sizeof(pci_device_t));
    if (!pci_dev) {
        return NULL;
    }

    memset(pci_dev, 0, sizeof(pci_device_t));
    pci_dev->location = location;

    // Initialize base device structure
    pci_dev->device.type = DEVICE_TYPE_PCI;
    pci_dev->device.state = DEVICE_STATE_INITIALIZING;
    pci_dev->device.capabilities = DEVICE_CAP_READ | DEVICE_CAP_WRITE | DEVICE_CAP_CONFIG;

    return pci_dev;
}

/**
 * @brief Initialize PCI device information
 */
static void pci_init_device_info(pci_device_t* pci_dev)
{
    pci_location_t loc = pci_dev->location;

    // Read configuration space
    pci_dev->config.vendor_id = pci_config_read16(loc, PCI_CONFIG_VENDOR_ID);
    pci_dev->config.device_id = pci_config_read16(loc, PCI_CONFIG_DEVICE_ID);
    pci_dev->config.command = pci_config_read16(loc, PCI_CONFIG_COMMAND);
    pci_dev->config.status = pci_config_read16(loc, PCI_CONFIG_STATUS);
    pci_dev->config.revision_id = pci_config_read8(loc, PCI_CONFIG_REVISION_ID);
    pci_dev->config.prog_if = pci_config_read8(loc, PCI_CONFIG_PROG_IF);
    pci_dev->config.subclass = pci_config_read8(loc, PCI_CONFIG_SUBCLASS);
    pci_dev->config.class_code = pci_config_read8(loc, PCI_CONFIG_CLASS);
    pci_dev->config.cache_line_size = pci_config_read8(loc, PCI_CONFIG_CACHE_LINE_SIZE);
    pci_dev->config.latency_timer = pci_config_read8(loc, PCI_CONFIG_LATENCY_TIMER);
    pci_dev->config.header_type = pci_config_read8(loc, PCI_CONFIG_HEADER_TYPE);
    pci_dev->config.bist = pci_config_read8(loc, PCI_CONFIG_BIST);
    pci_dev->config.cardbus_cis = pci_config_read32(loc, PCI_CONFIG_CARDBUS_CIS);
    pci_dev->config.subsystem_vendor_id = pci_config_read16(loc, PCI_CONFIG_SUBSYSTEM_VID);
    pci_dev->config.subsystem_id = pci_config_read16(loc, PCI_CONFIG_SUBSYSTEM_ID);
    pci_dev->config.rom_address = pci_config_read32(loc, PCI_CONFIG_ROM_ADDRESS);
    pci_dev->config.capabilities_ptr = pci_config_read8(loc, PCI_CONFIG_CAPABILITIES);
    pci_dev->config.interrupt_line = pci_config_read8(loc, PCI_CONFIG_INTERRUPT_LINE);
    pci_dev->config.interrupt_pin = pci_config_read8(loc, PCI_CONFIG_INTERRUPT_PIN);
    pci_dev->config.min_grant = pci_config_read8(loc, PCI_CONFIG_MIN_GRANT);
    pci_dev->config.max_latency = pci_config_read8(loc, PCI_CONFIG_MAX_LATENCY);

    // Initialize device info
    snprintf(pci_dev->device.info.name, sizeof(pci_dev->device.info.name),
             "PCI_%02x:%02x.%x", loc.bus, loc.device, loc.function);
    
    snprintf(pci_dev->device.info.description, sizeof(pci_dev->device.info.description),
             "%s", pci_get_class_name(pci_dev->config.class_code));
    
    strncpy(pci_dev->device.info.manufacturer, pci_get_vendor_name(pci_dev->config.vendor_id),
            sizeof(pci_dev->device.info.manufacturer) - 1);
    
    snprintf(pci_dev->device.info.model, sizeof(pci_dev->device.info.model),
             "Device_%04X", pci_dev->config.device_id);

    pci_dev->device.info.vendor_id = pci_dev->config.vendor_id;
    pci_dev->device.info.device_id = pci_dev->config.device_id;
    pci_dev->device.info.revision = pci_dev->config.revision_id;
    pci_dev->device.info.class_code = pci_dev->config.class_code;
    pci_dev->device.info.subclass_code = pci_dev->config.subclass;

    // Set device type based on class
    pci_dev->device.type = pci_class_to_device_type(pci_dev->config.class_code);

    // Set configuration
    pci_dev->device.config.base_address = (uint32_t)pci_get_bar_address(pci_dev, 0);
    pci_dev->device.config.irq_line = pci_dev->config.interrupt_line;
}

/**
 * @brief Probe PCI device BARs
 */
static void pci_probe_bars(pci_device_t* pci_dev)
{
    pci_location_t loc = pci_dev->location;

    for (int i = 0; i < 6; i++) {
        // Read current BAR value
        uint8_t bar_offset = PCI_CONFIG_BAR0 + (i * 4);
        uint32_t bar = pci_config_read32(loc, bar_offset);
        pci_dev->config.bar[i] = bar;

        if (bar == 0) {
            continue; // BAR not implemented
        }

        // Determine BAR type
        if (bar & PCI_BAR_TYPE_IO) {
            pci_dev->bar_is_io[i] = true;
        } else {
            pci_dev->bar_is_io[i] = false;
            
            // Check if 64-bit BAR
            uint8_t mem_type = (bar >> 1) & 0x3;
            if (mem_type == 2) { // 64-bit
                pci_dev->bar_is_64bit[i] = true;
            }
        }

        // Determine BAR size by writing all 1s
        pci_config_write32(loc, bar_offset, 0xFFFFFFFF);
        uint32_t size_mask = pci_config_read32(loc, bar_offset);
        pci_config_write32(loc, bar_offset, bar); // Restore original value

        if (pci_dev->bar_is_io[i]) {
            // I/O space BAR
            size_mask &= ~0x3UL;
            pci_dev->bar_sizes[i] = (~size_mask) + 1;
        } else {
            // Memory space BAR
            size_mask &= ~0xFUL;
            pci_dev->bar_sizes[i] = (~size_mask) + 1;
            
            if (pci_dev->bar_is_64bit[i] && i < 5) {
                // Handle 64-bit BAR (skip next BAR as it's part of this one)
                i++;
                pci_dev->config.bar[i] = pci_config_read32(loc, PCI_CONFIG_BAR0 + (i * 4));
            }
        }
    }
}

/**
 * @brief Convert PCI class code to device type
 */
static device_type_t pci_class_to_device_type(uint8_t class_code)
{
    switch (class_code) {
        case PCI_CLASS_MASS_STORAGE:
            return DEVICE_TYPE_STORAGE;
        case PCI_CLASS_NETWORK:
            return DEVICE_TYPE_NETWORK;
        case PCI_CLASS_DISPLAY:
            return DEVICE_TYPE_GRAPHICS;
        case PCI_CLASS_MULTIMEDIA:
            return DEVICE_TYPE_AUDIO;
        case PCI_CLASS_INPUT:
            return DEVICE_TYPE_INPUT;
        case PCI_CLASS_SERIAL:
            return DEVICE_TYPE_SERIAL;
        default:
            return DEVICE_TYPE_PCI;
    }
} 