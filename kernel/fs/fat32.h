/**
 * @file fat32.h
 * @brief FAT32 File System Compatibility Layer for FG-OS
 * 
 * This file defines the FAT32 file system compatibility layer, allowing FG-OS
 * to read and write FAT32 file systems for interoperability with Windows and
 * other operating systems.
 * 
 * @author Faiz Nasir
 * @company FGCompany Official
 * @version 1.0.0
 * @date 2024
 * @copyright © 2024 FGCompany Official. All rights reserved.
 */

#ifndef __FAT32_H__
#define __FAT32_H__

#include "fs.h"

/**
 * @defgroup fat32 FAT32 Compatibility Layer
 * @brief FAT32 file system support
 * @{
 */

// FAT32 Constants
#define FAT32_SECTOR_SIZE       512
#define FAT32_MAX_FILENAME      255
#define FAT32_MAX_PATH          260
#define FAT32_CLUSTER_EOF       0x0FFFFFF8
#define FAT32_CLUSTER_BAD       0x0FFFFFF7
#define FAT32_CLUSTER_FREE      0x00000000

// FAT32 Attribute Flags
typedef enum {
    FAT32_ATTR_READ_ONLY    = 0x01,
    FAT32_ATTR_HIDDEN       = 0x02,
    FAT32_ATTR_SYSTEM       = 0x04,
    FAT32_ATTR_VOLUME_ID    = 0x08,
    FAT32_ATTR_DIRECTORY    = 0x10,
    FAT32_ATTR_ARCHIVE      = 0x20,
    FAT32_ATTR_LONG_NAME    = 0x0F
} fat32_attributes_t;

// FAT32 Boot Sector Structure
typedef struct {
    uint8_t     jump_boot[3];           /**< Jump instruction */
    uint8_t     oem_name[8];            /**< OEM name */
    uint16_t    bytes_per_sector;       /**< Bytes per sector */
    uint8_t     sectors_per_cluster;    /**< Sectors per cluster */
    uint16_t    reserved_sector_count;  /**< Reserved sectors */
    uint8_t     num_fats;               /**< Number of FATs */
    uint16_t    root_entry_count;       /**< Root directory entries */
    uint16_t    total_sectors_16;       /**< Total sectors (16-bit) */
    uint8_t     media;                  /**< Media descriptor */
    uint16_t    fat_size_16;            /**< FAT size (16-bit) */
    uint16_t    sectors_per_track;      /**< Sectors per track */
    uint16_t    num_heads;              /**< Number of heads */
    uint32_t    hidden_sectors;         /**< Hidden sectors */
    uint32_t    total_sectors_32;       /**< Total sectors (32-bit) */
    uint32_t    fat_size_32;            /**< FAT size (32-bit) */
    uint16_t    ext_flags;              /**< Extended flags */
    uint16_t    fs_version;             /**< File system version */
    uint32_t    root_cluster;           /**< Root directory cluster */
    uint16_t    fs_info;                /**< FSInfo sector */
    uint16_t    backup_boot_sector;     /**< Backup boot sector */
    uint8_t     reserved[12];           /**< Reserved */
    uint8_t     drive_number;           /**< Drive number */
    uint8_t     reserved1;              /**< Reserved */
    uint8_t     boot_signature;         /**< Boot signature */
    uint32_t    volume_id;              /**< Volume ID */
    uint8_t     volume_label[11];       /**< Volume label */
    uint8_t     fs_type[8];             /**< File system type */
    uint8_t     boot_code[420];         /**< Boot code */
    uint16_t    boot_sector_signature;  /**< Boot sector signature */
} __attribute__((packed)) fat32_boot_sector_t;

// FAT32 FSInfo Structure
typedef struct {
    uint32_t    lead_signature;         /**< Lead signature */
    uint8_t     reserved1[480];         /**< Reserved */
    uint32_t    struct_signature;       /**< Structure signature */
    uint32_t    free_count;             /**< Free cluster count */
    uint32_t    next_free;              /**< Next free cluster */
    uint8_t     reserved2[12];          /**< Reserved */
    uint32_t    trail_signature;        /**< Trail signature */
} __attribute__((packed)) fat32_fsinfo_t;

// FAT32 Directory Entry Structure
typedef struct {
    uint8_t     name[11];               /**< File name (8.3 format) */
    uint8_t     attributes;             /**< File attributes */
    uint8_t     reserved;               /**< Reserved */
    uint8_t     create_time_tenth;      /**< Creation time (tenths) */
    uint16_t    create_time;            /**< Creation time */
    uint16_t    create_date;            /**< Creation date */
    uint16_t    last_access_date;       /**< Last access date */
    uint16_t    first_cluster_high;     /**< First cluster (high) */
    uint16_t    write_time;             /**< Write time */
    uint16_t    write_date;             /**< Write date */
    uint16_t    first_cluster_low;      /**< First cluster (low) */
    uint32_t    file_size;              /**< File size */
} __attribute__((packed)) fat32_dir_entry_t;

// FAT32 Long File Name Entry
typedef struct {
    uint8_t     order;                  /**< Order in sequence */
    uint16_t    name1[5];               /**< First 5 characters */
    uint8_t     attributes;             /**< Attributes (always 0x0F) */
    uint8_t     type;                   /**< Type (always 0) */
    uint8_t     checksum;               /**< Checksum */
    uint16_t    name2[6];               /**< Next 6 characters */
    uint16_t    first_cluster_low;      /**< First cluster (always 0) */
    uint16_t    name3[2];               /**< Last 2 characters */
} __attribute__((packed)) fat32_lfn_entry_t;

// FAT32 Private Data Structure
typedef struct {
    fat32_boot_sector_t *boot_sector;   /**< Boot sector */
    fat32_fsinfo_t      *fsinfo;        /**< FSInfo structure */
    uint32_t            *fat_table;     /**< FAT table */
    uint32_t            fat_start_sector; /**< FAT start sector */
    uint32_t            fat_size;        /**< FAT size in sectors */
    uint32_t            data_start_sector; /**< Data start sector */
    uint32_t            root_cluster;    /**< Root directory cluster */
    uint32_t            cluster_size;    /**< Cluster size in bytes */
    uint32_t            total_clusters;  /**< Total clusters */
    uint32_t            free_clusters;   /**< Free clusters */
    uint32_t            next_free_cluster; /**< Next free cluster */
    fs_cache_t          *cache;          /**< Cache */
} fat32_private_t;

// FAT32 File Handle
typedef struct {
    uint32_t            first_cluster;   /**< First cluster */
    uint32_t            current_cluster; /**< Current cluster */
    uint32_t            cluster_offset;  /**< Offset in cluster */
    fat32_dir_entry_t   dir_entry;      /**< Directory entry */
    bool                is_directory;    /**< Is directory flag */
    char                long_name[FAT32_MAX_FILENAME]; /**< Long file name */
} fat32_file_handle_t;

// FAT32 Function Declarations
int fat32_init(void);
void fat32_shutdown(void);
int fat32_mount(filesystem_t *fs, const char *device, uint32_t flags);
int fat32_unmount(filesystem_t *fs);

// File operations
int fat32_open(filesystem_t *fs, const char *path, file_access_mode_t mode, file_t **file);
int fat32_close(file_t *file);
ssize_t fat32_read(file_t *file, void *buffer, size_t size);
ssize_t fat32_write(file_t *file, const void *buffer, size_t size);
int64_t fat32_seek(file_t *file, int64_t offset, seek_origin_t origin);

// Directory operations
int fat32_opendir(filesystem_t *fs, const char *path, directory_t **dir);
int fat32_readdir(directory_t *dir, directory_entry_t *entry);
int fat32_mkdir(filesystem_t *fs, const char *path, uint16_t permissions);

// File management
int fat32_create(filesystem_t *fs, const char *path, uint16_t permissions);
int fat32_unlink(filesystem_t *fs, const char *path);
int fat32_stat(filesystem_t *fs, const char *path, file_metadata_t *metadata);

// Internal functions
uint32_t fat32_get_next_cluster(fat32_private_t *priv, uint32_t cluster);
int fat32_set_cluster(fat32_private_t *priv, uint32_t cluster, uint32_t value);
uint32_t fat32_alloc_cluster(fat32_private_t *priv);
void fat32_free_cluster_chain(fat32_private_t *priv, uint32_t start_cluster);
int fat32_read_cluster(fat32_private_t *priv, uint32_t cluster, void *buffer);
int fat32_write_cluster(fat32_private_t *priv, uint32_t cluster, const void *buffer);

// Utility functions
uint32_t fat32_cluster_to_sector(fat32_private_t *priv, uint32_t cluster);
void fat32_parse_8_3_name(const uint8_t *raw_name, char *name);
void fat32_create_8_3_name(const char *name, uint8_t *raw_name);
uint8_t fat32_lfn_checksum(const uint8_t *short_name);
int fat32_parse_long_name(const fat32_lfn_entry_t *lfn_entries, int count, char *name);
uint16_t fat32_date_to_fat(uint64_t timestamp);
uint16_t fat32_time_to_fat(uint64_t timestamp);
uint64_t fat32_fat_to_timestamp(uint16_t date, uint16_t time);

/** @} */

#endif /* __FAT32_H__ */ 