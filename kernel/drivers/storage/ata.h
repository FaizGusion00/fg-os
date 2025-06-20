/**
 * @file ata.h
 * @brief ATA/IDE Storage Device Driver for FG-OS
 * 
 * This file provides ATA (Advanced Technology Attachment) storage device
 * support including IDE hard drives and optical drives with standard
 * read/write operations and device identification.
 * 
 * @author Faiz Nasir
 * @company FGCompany Official
 * @version 1.0.0
 * @date 2024
 * @copyright Copyright (c) 2024 FGCompany Official. All rights reserved.
 */

#ifndef __ATA_H__
#define __ATA_H__

#include <types.h>
#include "../device.h"

/**
 * @brief ATA I/O ports (primary controller)
 */
#define ATA_PRIMARY_DATA        0x1F0   /**< Data register */
#define ATA_PRIMARY_ERROR       0x1F1   /**< Error information */
#define ATA_PRIMARY_FEATURES    0x1F1   /**< Features register */
#define ATA_PRIMARY_SECTOR_COUNT 0x1F2  /**< Sector count */
#define ATA_PRIMARY_LBA_LOW     0x1F3   /**< LBA low byte */
#define ATA_PRIMARY_LBA_MID     0x1F4   /**< LBA mid byte */
#define ATA_PRIMARY_LBA_HIGH    0x1F5   /**< LBA high byte */
#define ATA_PRIMARY_DRIVE       0x1F6   /**< Drive select */
#define ATA_PRIMARY_STATUS      0x1F7   /**< Status register */
#define ATA_PRIMARY_COMMAND     0x1F7   /**< Command register */
#define ATA_PRIMARY_CONTROL     0x3F6   /**< Device control */

/**
 * @brief ATA I/O ports (secondary controller)
 */
#define ATA_SECONDARY_DATA      0x170   /**< Data register */
#define ATA_SECONDARY_ERROR     0x171   /**< Error information */
#define ATA_SECONDARY_FEATURES  0x171   /**< Features register */
#define ATA_SECONDARY_SECTOR_COUNT 0x172 /**< Sector count */
#define ATA_SECONDARY_LBA_LOW   0x173   /**< LBA low byte */
#define ATA_SECONDARY_LBA_MID   0x174   /**< LBA mid byte */
#define ATA_SECONDARY_LBA_HIGH  0x175   /**< LBA high byte */
#define ATA_SECONDARY_DRIVE     0x176   /**< Drive select */
#define ATA_SECONDARY_STATUS    0x177   /**< Status register */
#define ATA_SECONDARY_COMMAND   0x177   /**< Command register */
#define ATA_SECONDARY_CONTROL   0x376   /**< Device control */

/**
 * @brief ATA commands
 */
#define ATA_CMD_READ_SECTORS    0x20    /**< Read sectors */
#define ATA_CMD_READ_SECTORS_EXT 0x24   /**< Read sectors extended */
#define ATA_CMD_WRITE_SECTORS   0x30    /**< Write sectors */
#define ATA_CMD_WRITE_SECTORS_EXT 0x34  /**< Write sectors extended */
#define ATA_CMD_IDENTIFY        0xEC    /**< Identify device */
#define ATA_CMD_IDENTIFY_PACKET 0xA1    /**< Identify packet device */
#define ATA_CMD_FLUSH_CACHE     0xE7    /**< Flush cache */
#define ATA_CMD_FLUSH_CACHE_EXT 0xEA    /**< Flush cache extended */

/**
 * @brief ATA status register bits
 */
#define ATA_STATUS_ERR          (1 << 0)    /**< Error */
#define ATA_STATUS_DRQ          (1 << 3)    /**< Data request */
#define ATA_STATUS_SRV          (1 << 4)    /**< Service */
#define ATA_STATUS_DF           (1 << 5)    /**< Drive fault */
#define ATA_STATUS_RDY          (1 << 6)    /**< Ready */
#define ATA_STATUS_BSY          (1 << 7)    /**< Busy */

/**
 * @brief ATA error register bits
 */
#define ATA_ERROR_AMNF          (1 << 0)    /**< Address mark not found */
#define ATA_ERROR_T0NF          (1 << 1)    /**< Track 0 not found */
#define ATA_ERROR_ABRT          (1 << 2)    /**< Aborted command */
#define ATA_ERROR_MCR           (1 << 3)    /**< Media change request */
#define ATA_ERROR_IDNF          (1 << 4)    /**< ID not found */
#define ATA_ERROR_MC            (1 << 5)    /**< Media changed */
#define ATA_ERROR_UNC           (1 << 6)    /**< Uncorrectable data */
#define ATA_ERROR_BBK           (1 << 7)    /**< Bad block */

/**
 * @brief ATA drive select bits
 */
#define ATA_DRIVE_MASTER        0xA0        /**< Master drive */
#define ATA_DRIVE_SLAVE         0xB0        /**< Slave drive */
#define ATA_DRIVE_LBA           (1 << 6)    /**< LBA mode */

/**
 * @brief ATA device types
 */
typedef enum {
    ATA_DEVICE_UNKNOWN = 0,     /**< Unknown device */
    ATA_DEVICE_PATA,            /**< Parallel ATA */
    ATA_DEVICE_SATA,            /**< Serial ATA */
    ATA_DEVICE_ATAPI,           /**< ATAPI (CD/DVD) */
    ATA_DEVICE_MAX
} ata_device_type_t;

/**
 * @brief ATA controller structure
 */
typedef struct {
    uint16_t    base_port;          /**< Base I/O port */
    uint16_t    control_port;       /**< Control port */
    uint8_t     irq;                /**< IRQ number */
    bool        enabled;            /**< Controller enabled */
} ata_controller_t;

/**
 * @brief ATA device identification structure
 */
typedef struct {
    uint16_t    config;             /**< General configuration */
    uint16_t    cylinders;          /**< Number of logical cylinders */
    uint16_t    reserved1;          /**< Reserved */
    uint16_t    heads;              /**< Number of logical heads */
    uint16_t    bytes_per_track;    /**< Bytes per track */
    uint16_t    bytes_per_sector;   /**< Bytes per sector */
    uint16_t    sectors_per_track;  /**< Sectors per track */
    uint16_t    vendor_unique1[3];  /**< Vendor unique */
    char        serial_number[20];  /**< Serial number */
    uint16_t    buffer_type;        /**< Buffer type */
    uint16_t    buffer_size;        /**< Buffer size in 512-byte increments */
    uint16_t    ecc_bytes;          /**< ECC bytes */
    char        firmware_revision[8]; /**< Firmware revision */
    char        model_number[40];   /**< Model number */
    uint16_t    max_sectors_per_int; /**< Maximum sectors per interrupt */
    uint16_t    trusted_computing;  /**< Trusted computing */
    uint16_t    capabilities;       /**< Capabilities */
    uint16_t    reserved2;          /**< Reserved */
    uint16_t    pio_timing;         /**< PIO data transfer cycle timing */
    uint16_t    dma_timing;         /**< DMA data transfer cycle timing */
    uint16_t    field_validity;     /**< Field validity */
    uint16_t    current_cylinders;  /**< Current logical cylinders */
    uint16_t    current_heads;      /**< Current logical heads */
    uint16_t    current_sectors;    /**< Current logical sectors per track */
    uint32_t    current_capacity;   /**< Current capacity in sectors */
    uint16_t    multi_sector;       /**< Multiple sector setting */
    uint32_t    lba_sectors;        /**< Total sectors in LBA mode */
    uint16_t    single_dma;         /**< Single word DMA transfer */
    uint16_t    multi_dma;          /**< Multiple word DMA transfer */
    uint16_t    advanced_pio;       /**< Advanced PIO transfer modes */
    uint16_t    min_dma_cycle;      /**< Minimum multiword DMA cycle time */
    uint16_t    rec_dma_cycle;      /**< Recommended multiword DMA cycle time */
    uint16_t    min_pio_cycle;      /**< Minimum PIO cycle time without flow control */
    uint16_t    min_pio_iordy;      /**< Minimum PIO cycle time with IORDY */
    uint16_t    reserved3[6];       /**< Reserved */
    uint16_t    queue_depth;        /**< Queue depth */
    uint16_t    reserved4[4];       /**< Reserved */
    uint16_t    major_version;      /**< Major version number */
    uint16_t    minor_version;      /**< Minor version number */
    uint16_t    command_sets1;      /**< Command sets supported */
    uint16_t    command_sets2;      /**< Command sets supported */
    uint16_t    command_sets3;      /**< Command sets supported */
    uint16_t    features1;          /**< Command sets/features enabled */
    uint16_t    features2;          /**< Command sets/features enabled */
    uint16_t    features3;          /**< Command sets/features enabled */
    uint16_t    ultra_dma;          /**< Ultra DMA transfer modes */
    uint16_t    erase_time;         /**< Time for security erase */
    uint16_t    enhanced_erase_time; /**< Time for enhanced security erase */
    uint16_t    current_apm;        /**< Current advanced power management */
    uint16_t    master_password;    /**< Master password revision code */
    uint16_t    hw_reset_result;    /**< Hardware reset result */
    uint16_t    acoustic_value;     /**< Acoustic management value */
    uint16_t    stream_min_size;    /**< Stream minimum request granularity */
    uint16_t    stream_transfer_time; /**< Stream transfer time for DMA */
    uint16_t    stream_access_latency; /**< Stream access latency for DMA and PIO */
    uint32_t    stream_granularity; /**< Stream performance granularity */
    uint64_t    lba48_sectors;      /**< Total sectors in LBA48 mode */
    uint16_t    stream_transfer_size; /**< Stream transfer size for PIO */
    uint16_t    reserved5[23];      /**< Reserved */
    uint16_t    removable_status;   /**< Removable media status notification */
    uint16_t    security_status;    /**< Security status */
    uint16_t    vendor_unique2[31]; /**< Vendor unique */
    uint16_t    cfa_power_mode;     /**< CFA power mode */
    uint16_t    reserved6[15];      /**< Reserved */
    uint16_t    media_serial[30];   /**< Current media serial number */
    uint16_t    reserved7[49];      /**< Reserved */
    uint16_t    integrity_word;     /**< Integrity word */
} __attribute__((packed)) ata_identify_t;

/**
 * @brief ATA device structure
 */
typedef struct ata_device {
    device_t            device;         /**< Base device structure */
    ata_device_type_t   type;           /**< ATA device type */
    ata_controller_t*   controller;     /**< ATA controller */
    uint8_t             drive_num;      /**< Drive number (0=master, 1=slave) */
    ata_identify_t      identify;       /**< Device identification */
    uint64_t            total_sectors;  /**< Total number of sectors */
    uint32_t            sector_size;    /**< Sector size in bytes */
    bool                lba48_support;  /**< LBA48 support */
    bool                dma_support;    /**< DMA support */
    struct ata_device*  next;           /**< Next ATA device */
} ata_device_t;

/**
 * @brief ATA statistics
 */
typedef struct {
    uint32_t    controllers_found;      /**< Controllers found */
    uint32_t    devices_found;          /**< Devices found */
    uint32_t    read_operations;        /**< Read operations */
    uint32_t    write_operations;       /**< Write operations */
    uint64_t    sectors_read;           /**< Sectors read */
    uint64_t    sectors_written;        /**< Sectors written */
    uint32_t    errors_total;           /**< Total errors */
    uint32_t    timeouts;               /**< Operation timeouts */
} ata_stats_t;

// Function declarations

/**
 * @brief Initialize the ATA driver
 * 
 * @return 0 on success, negative error code on failure
 */
int ata_init(void);

/**
 * @brief Shutdown the ATA driver
 */
void ata_shutdown(void);

/**
 * @brief Detect and enumerate ATA devices
 * 
 * @return Number of devices found
 */
uint32_t ata_detect_devices(void);

/**
 * @brief Read sectors from ATA device
 * 
 * @param device Pointer to ATA device
 * @param lba Logical block address
 * @param count Number of sectors to read
 * @param buffer Buffer to store data
 * @return 0 on success, negative error code on failure
 */
int ata_read_sectors(ata_device_t* device, uint64_t lba, uint32_t count, void* buffer);

/**
 * @brief Write sectors to ATA device
 * 
 * @param device Pointer to ATA device
 * @param lba Logical block address
 * @param count Number of sectors to write
 * @param buffer Buffer containing data
 * @return 0 on success, negative error code on failure
 */
int ata_write_sectors(ata_device_t* device, uint64_t lba, uint32_t count, const void* buffer);

/**
 * @brief Flush cache to ATA device
 * 
 * @param device Pointer to ATA device
 * @return 0 on success, negative error code on failure
 */
int ata_flush_cache(ata_device_t* device);

/**
 * @brief Identify ATA device
 * 
 * @param controller Pointer to ATA controller
 * @param drive_num Drive number (0=master, 1=slave)
 * @param identify Pointer to identification structure
 * @return 0 on success, negative error code on failure
 */
int ata_identify_device(ata_controller_t* controller, uint8_t drive_num, ata_identify_t* identify);

/**
 * @brief Wait for ATA device to be ready
 * 
 * @param controller Pointer to ATA controller
 * @param timeout_ms Timeout in milliseconds
 * @return 0 on success, negative error code on failure
 */
int ata_wait_ready(ata_controller_t* controller, uint32_t timeout_ms);

/**
 * @brief Wait for ATA device data request
 * 
 * @param controller Pointer to ATA controller
 * @param timeout_ms Timeout in milliseconds
 * @return 0 on success, negative error code on failure
 */
int ata_wait_drq(ata_controller_t* controller, uint32_t timeout_ms);

/**
 * @brief Select ATA drive
 * 
 * @param controller Pointer to ATA controller
 * @param drive_num Drive number (0=master, 1=slave)
 * @param lba LBA mode flag
 */
void ata_select_drive(ata_controller_t* controller, uint8_t drive_num, bool lba);

/**
 * @brief Reset ATA controller
 * 
 * @param controller Pointer to ATA controller
 * @return 0 on success, negative error code on failure
 */
int ata_reset_controller(ata_controller_t* controller);

/**
 * @brief Get ATA device by ID
 * 
 * @param device_id Device ID
 * @return Pointer to ATA device, NULL if not found
 */
ata_device_t* ata_get_device(uint32_t device_id);

/**
 * @brief Get ATA statistics
 * 
 * @return Pointer to ATA statistics structure
 */
const ata_stats_t* ata_get_stats(void);

/**
 * @brief Reset ATA statistics
 */
void ata_reset_stats(void);

/**
 * @brief Get ATA device type name
 * 
 * @param type Device type
 * @return Device type name string
 */
const char* ata_device_type_to_string(ata_device_type_t type);

/**
 * @brief Dump ATA device information
 * 
 * @param device Pointer to ATA device (NULL for all devices)
 */
void ata_dump_device_info(ata_device_t* device);

/**
 * @brief Handle ATA interrupt
 * 
 * @param irq IRQ number
 */
void ata_interrupt_handler(uint32_t irq);

/**
 * @brief Enable ATA controller interrupts
 * 
 * @param controller Pointer to ATA controller
 * @param enable Enable or disable interrupts
 */
void ata_enable_interrupts(ata_controller_t* controller, bool enable);

#endif /* __ATA_H__ */ 