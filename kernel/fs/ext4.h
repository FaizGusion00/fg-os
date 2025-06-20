/**
 * @file ext4.h
 * @brief ext4 File System Compatibility Layer for FG-OS
 * 
 * This file defines the ext4 file system compatibility layer, allowing FG-OS
 * to read and write ext4 file systems for interoperability with Linux and
 * other Unix-like operating systems.
 * 
 * @author Faiz Nasir
 * @company FGCompany Official
 * @version 1.0.0
 * @date 2024
 * @copyright © 2024 FGCompany Official. All rights reserved.
 */

#ifndef __EXT4_H__
#define __EXT4_H__

#include "fs.h"

/**
 * @defgroup ext4 ext4 Compatibility Layer
 * @brief ext4 file system support
 * @{
 */

// ext4 Constants
#define EXT4_SUPER_MAGIC        0xEF53
#define EXT4_MAX_FILENAME       255
#define EXT4_BLOCK_SIZE_MIN     1024
#define EXT4_BLOCK_SIZE_MAX     65536
#define EXT4_MIN_BLOCK_SIZE     1024
#define EXT4_MAX_BLOCK_SIZE     4096
#define EXT4_INODE_SIZE_MIN     128
#define EXT4_INODE_SIZE_MAX     1024

// ext4 Feature Flags
#define EXT4_FEATURE_COMPAT_DIR_PREALLOC        0x0001
#define EXT4_FEATURE_COMPAT_IMAGIC_INODES       0x0002
#define EXT4_FEATURE_COMPAT_HAS_JOURNAL         0x0004
#define EXT4_FEATURE_COMPAT_EXT_ATTR            0x0008
#define EXT4_FEATURE_COMPAT_RESIZE_INODE        0x0010
#define EXT4_FEATURE_COMPAT_DIR_INDEX           0x0020
#define EXT4_FEATURE_COMPAT_SPARSE_SUPER2       0x0200

#define EXT4_FEATURE_INCOMPAT_COMPRESSION       0x0001
#define EXT4_FEATURE_INCOMPAT_FILETYPE          0x0002
#define EXT4_FEATURE_INCOMPAT_RECOVER           0x0004
#define EXT4_FEATURE_INCOMPAT_JOURNAL_DEV       0x0008
#define EXT4_FEATURE_INCOMPAT_META_BG           0x0010
#define EXT4_FEATURE_INCOMPAT_EXTENTS           0x0040
#define EXT4_FEATURE_INCOMPAT_64BIT             0x0080
#define EXT4_FEATURE_INCOMPAT_MMP               0x0100
#define EXT4_FEATURE_INCOMPAT_FLEX_BG           0x0200
#define EXT4_FEATURE_INCOMPAT_EA_INODE          0x0400
#define EXT4_FEATURE_INCOMPAT_DIRDATA           0x1000
#define EXT4_FEATURE_INCOMPAT_CSUM_SEED         0x2000
#define EXT4_FEATURE_INCOMPAT_LARGEDIR          0x4000
#define EXT4_FEATURE_INCOMPAT_INLINE_DATA       0x8000
#define EXT4_FEATURE_INCOMPAT_ENCRYPT           0x10000

// ext4 File Types
#define EXT4_FT_UNKNOWN         0
#define EXT4_FT_REG_FILE        1
#define EXT4_FT_DIR             2
#define EXT4_FT_CHRDEV          3
#define EXT4_FT_BLKDEV          4
#define EXT4_FT_FIFO            5
#define EXT4_FT_SOCK            6
#define EXT4_FT_SYMLINK         7

// ext4 Superblock Structure
typedef struct {
    uint32_t    s_inodes_count;         /**< Total inodes */
    uint32_t    s_blocks_count_lo;      /**< Total blocks (low) */
    uint32_t    s_r_blocks_count_lo;    /**< Reserved blocks (low) */
    uint32_t    s_free_blocks_count_lo; /**< Free blocks (low) */
    uint32_t    s_free_inodes_count;    /**< Free inodes */
    uint32_t    s_first_data_block;     /**< First data block */
    uint32_t    s_log_block_size;       /**< Log block size */
    uint32_t    s_log_cluster_size;     /**< Log cluster size */
    uint32_t    s_blocks_per_group;     /**< Blocks per group */
    uint32_t    s_clusters_per_group;   /**< Clusters per group */
    uint32_t    s_inodes_per_group;     /**< Inodes per group */
    uint32_t    s_mtime;                /**< Mount time */
    uint32_t    s_wtime;                /**< Write time */
    uint16_t    s_mnt_count;            /**< Mount count */
    uint16_t    s_max_mnt_count;        /**< Maximum mount count */
    uint16_t    s_magic;                /**< Magic signature */
    uint16_t    s_state;                /**< File system state */
    uint16_t    s_errors;               /**< Error behavior */
    uint16_t    s_minor_rev_level;      /**< Minor revision level */
    uint32_t    s_lastcheck;            /**< Last check time */
    uint32_t    s_checkinterval;        /**< Check interval */
    uint32_t    s_creator_os;           /**< Creator OS */
    uint32_t    s_rev_level;            /**< Revision level */
    uint16_t    s_def_resuid;           /**< Default reserved UID */
    uint16_t    s_def_resgid;           /**< Default reserved GID */
    uint32_t    s_first_ino;            /**< First non-reserved inode */
    uint16_t    s_inode_size;           /**< Inode size */
    uint16_t    s_block_group_nr;       /**< Block group number */
    uint32_t    s_feature_compat;       /**< Compatible features */
    uint32_t    s_feature_incompat;     /**< Incompatible features */
    uint32_t    s_feature_ro_compat;    /**< Read-only compatible features */
    uint8_t     s_uuid[16];             /**< UUID */
    uint8_t     s_volume_name[16];      /**< Volume name */
    uint8_t     s_last_mounted[64];     /**< Last mounted path */
    uint32_t    s_algorithm_usage_bitmap; /**< Compression algorithms */
    uint8_t     s_prealloc_blocks;      /**< Preallocate blocks */
    uint8_t     s_prealloc_dir_blocks;  /**< Preallocate dir blocks */
    uint16_t    s_reserved_gdt_blocks;  /**< Reserved GDT blocks */
    uint8_t     s_journal_uuid[16];     /**< Journal UUID */
    uint32_t    s_journal_inum;         /**< Journal inode */
    uint32_t    s_journal_dev;          /**< Journal device */
    uint32_t    s_last_orphan;          /**< Last orphan */
    uint32_t    s_hash_seed[4];         /**< Hash seed */
    uint8_t     s_def_hash_version;     /**< Default hash version */
    uint8_t     s_jnl_backup_type;      /**< Journal backup type */
    uint16_t    s_desc_size;            /**< Group descriptor size */
    uint32_t    s_default_mount_opts;   /**< Default mount options */
    uint32_t    s_first_meta_bg;        /**< First meta block group */
    uint32_t    s_mkfs_time;            /**< Filesystem creation time */
    uint32_t    s_jnl_blocks[17];       /**< Journal backup */
    uint32_t    s_blocks_count_hi;      /**< Total blocks (high) */
    uint32_t    s_r_blocks_count_hi;    /**< Reserved blocks (high) */
    uint32_t    s_free_blocks_count_hi; /**< Free blocks (high) */
    uint16_t    s_min_extra_isize;      /**< Minimum extra inode size */
    uint16_t    s_want_extra_isize;     /**< Wanted extra inode size */
    uint32_t    s_flags;                /**< Miscellaneous flags */
    uint16_t    s_raid_stride;          /**< RAID stride */
    uint16_t    s_mmp_update_interval;  /**< MMP update interval */
    uint64_t    s_mmp_block;            /**< MMP block */
    uint32_t    s_raid_stripe_width;    /**< RAID stripe width */
    uint8_t     s_log_groups_per_flex;  /**< Log groups per flex */
    uint8_t     s_checksum_type;        /**< Checksum type */
    uint16_t    s_reserved_pad;         /**< Reserved padding */
    uint64_t    s_kbytes_written;       /**< KB written */
    uint32_t    s_snapshot_inum;        /**< Snapshot inode */
    uint32_t    s_snapshot_id;          /**< Snapshot ID */
    uint64_t    s_snapshot_r_blocks_count; /**< Snapshot reserved blocks */
    uint32_t    s_snapshot_list;        /**< Snapshot list */
    uint32_t    s_error_count;          /**< Error count */
    uint32_t    s_first_error_time;     /**< First error time */
    uint32_t    s_first_error_ino;      /**< First error inode */
    uint64_t    s_first_error_block;    /**< First error block */
    uint8_t     s_first_error_func[32]; /**< First error function */
    uint32_t    s_first_error_line;     /**< First error line */
    uint32_t    s_last_error_time;      /**< Last error time */
    uint32_t    s_last_error_ino;       /**< Last error inode */
    uint32_t    s_last_error_line;      /**< Last error line */
    uint64_t    s_last_error_block;     /**< Last error block */
    uint8_t     s_last_error_func[32];  /**< Last error function */
    uint8_t     s_mount_opts[64];       /**< Mount options */
    uint32_t    s_usr_quota_inum;       /**< User quota inode */
    uint32_t    s_grp_quota_inum;       /**< Group quota inode */
    uint32_t    s_overhead_clusters;    /**< Overhead clusters */
    uint32_t    s_backup_bgs[2];        /**< Backup block groups */
    uint8_t     s_encrypt_algos[4];     /**< Encryption algorithms */
    uint8_t     s_encrypt_pw_salt[16];  /**< Encryption password salt */
    uint32_t    s_lpf_ino;              /**< Lost+found inode */
    uint32_t    s_prj_quota_inum;       /**< Project quota inode */
    uint32_t    s_checksum_seed;        /**< Checksum seed */
    uint32_t    s_reserved[98];         /**< Reserved */
    uint32_t    s_checksum;             /**< Checksum */
} __attribute__((packed)) ext4_superblock_t;

// ext4 Group Descriptor
typedef struct {
    uint32_t    bg_block_bitmap_lo;     /**< Block bitmap (low) */
    uint32_t    bg_inode_bitmap_lo;     /**< Inode bitmap (low) */
    uint32_t    bg_inode_table_lo;      /**< Inode table (low) */
    uint16_t    bg_free_blocks_count_lo; /**< Free blocks (low) */
    uint16_t    bg_free_inodes_count_lo; /**< Free inodes (low) */
    uint16_t    bg_used_dirs_count_lo;  /**< Used directories (low) */
    uint16_t    bg_flags;               /**< Flags */
    uint32_t    bg_exclude_bitmap_lo;   /**< Exclude bitmap (low) */
    uint16_t    bg_block_bitmap_csum_lo; /**< Block bitmap checksum (low) */
    uint16_t    bg_inode_bitmap_csum_lo; /**< Inode bitmap checksum (low) */
    uint16_t    bg_itable_unused_lo;    /**< Unused inodes (low) */
    uint16_t    bg_checksum;            /**< Group checksum */
    uint32_t    bg_block_bitmap_hi;     /**< Block bitmap (high) */
    uint32_t    bg_inode_bitmap_hi;     /**< Inode bitmap (high) */
    uint32_t    bg_inode_table_hi;      /**< Inode table (high) */
    uint16_t    bg_free_blocks_count_hi; /**< Free blocks (high) */
    uint16_t    bg_free_inodes_count_hi; /**< Free inodes (high) */
    uint16_t    bg_used_dirs_count_hi;  /**< Used directories (high) */
    uint16_t    bg_itable_unused_hi;    /**< Unused inodes (high) */
    uint32_t    bg_exclude_bitmap_hi;   /**< Exclude bitmap (high) */
    uint16_t    bg_block_bitmap_csum_hi; /**< Block bitmap checksum (high) */
    uint16_t    bg_inode_bitmap_csum_hi; /**< Inode bitmap checksum (high) */
    uint32_t    bg_reserved;            /**< Reserved */
} __attribute__((packed)) ext4_group_desc_t;

// ext4 Inode Structure
typedef struct {
    uint16_t    i_mode;                 /**< File mode */
    uint16_t    i_uid;                  /**< Owner UID (low) */
    uint32_t    i_size_lo;              /**< File size (low) */
    uint32_t    i_atime;                /**< Access time */
    uint32_t    i_ctime;                /**< Change time */
    uint32_t    i_mtime;                /**< Modification time */
    uint32_t    i_dtime;                /**< Deletion time */
    uint16_t    i_gid;                  /**< Group GID (low) */
    uint16_t    i_links_count;          /**< Hard links count */
    uint32_t    i_blocks_lo;            /**< Block count (low) */
    uint32_t    i_flags;                /**< File flags */
    uint32_t    i_version;              /**< Version */
    uint32_t    i_block[15];            /**< Block pointers */
    uint32_t    i_generation;           /**< File version */
    uint32_t    i_file_acl_lo;          /**< File ACL (low) */
    uint32_t    i_size_high;            /**< File size (high) */
    uint32_t    i_obso_faddr;           /**< Obsolete fragment address */
    uint16_t    i_blocks_high;          /**< Block count (high) */
    uint16_t    i_file_acl_high;        /**< File ACL (high) */
    uint16_t    i_uid_high;             /**< Owner UID (high) */
    uint16_t    i_gid_high;             /**< Group GID (high) */
    uint16_t    i_checksum_lo;          /**< Checksum (low) */
    uint16_t    i_reserved;             /**< Reserved */
    uint16_t    i_extra_isize;          /**< Extra inode size */
    uint16_t    i_checksum_hi;          /**< Checksum (high) */
    uint32_t    i_ctime_extra;          /**< Change time (extra) */
    uint32_t    i_mtime_extra;          /**< Modification time (extra) */
    uint32_t    i_atime_extra;          /**< Access time (extra) */
    uint32_t    i_crtime;               /**< Creation time */
    uint32_t    i_crtime_extra;         /**< Creation time (extra) */
    uint32_t    i_version_hi;           /**< Version (high) */
    uint32_t    i_projid;               /**< Project ID */
} __attribute__((packed)) ext4_inode_t;

// ext4 Directory Entry
typedef struct {
    uint32_t    inode;                  /**< Inode number */
    uint16_t    rec_len;                /**< Record length */
    uint8_t     name_len;               /**< Name length */
    uint8_t     file_type;              /**< File type */
    char        name[];                 /**< File name */
} __attribute__((packed)) ext4_dir_entry_t;

// ext4 Extent Header
typedef struct {
    uint16_t    eh_magic;               /**< Magic number */
    uint16_t    eh_entries;             /**< Number of entries */
    uint16_t    eh_max;                 /**< Maximum entries */
    uint16_t    eh_depth;               /**< Tree depth */
    uint32_t    eh_generation;          /**< Generation */
} __attribute__((packed)) ext4_extent_header_t;

// ext4 Extent
typedef struct {
    uint32_t    ee_block;               /**< Logical block */
    uint16_t    ee_len;                 /**< Length */
    uint16_t    ee_start_hi;            /**< Physical block (high) */
    uint32_t    ee_start_lo;            /**< Physical block (low) */
} __attribute__((packed)) ext4_extent_t;

// ext4 Private Data Structure
typedef struct {
    ext4_superblock_t   *superblock;    /**< Superblock */
    ext4_group_desc_t   *group_descs;   /**< Group descriptors */
    uint32_t            group_desc_count; /**< Group descriptor count */
    uint32_t            block_size;      /**< Block size */
    uint32_t            inode_size;      /**< Inode size */
    uint32_t            inodes_per_block; /**< Inodes per block */
    uint32_t            blocks_per_group; /**< Blocks per group */
    uint32_t            inodes_per_group; /**< Inodes per group */
    bool                has_64bit;       /**< 64-bit support */
    bool                has_extents;     /**< Extent support */
    fs_cache_t          *cache;          /**< Cache */
} ext4_private_t;

// ext4 Function Declarations
int ext4_init(void);
void ext4_shutdown(void);
int ext4_mount(filesystem_t *fs, const char *device, uint32_t flags);
int ext4_unmount(filesystem_t *fs);

// File operations
int ext4_open(filesystem_t *fs, const char *path, file_access_mode_t mode, file_t **file);
int ext4_close(file_t *file);
ssize_t ext4_read(file_t *file, void *buffer, size_t size);
ssize_t ext4_write(file_t *file, const void *buffer, size_t size);
int64_t ext4_seek(file_t *file, int64_t offset, seek_origin_t origin);

// Directory operations
int ext4_opendir(filesystem_t *fs, const char *path, directory_t **dir);
int ext4_readdir(directory_t *dir, directory_entry_t *entry);
int ext4_mkdir(filesystem_t *fs, const char *path, uint16_t permissions);

// File management
int ext4_create(filesystem_t *fs, const char *path, uint16_t permissions);
int ext4_unlink(filesystem_t *fs, const char *path);
int ext4_stat(filesystem_t *fs, const char *path, file_metadata_t *metadata);

// Internal functions
int ext4_read_inode(ext4_private_t *priv, uint32_t inode_num, ext4_inode_t *inode);
int ext4_write_inode(ext4_private_t *priv, uint32_t inode_num, const ext4_inode_t *inode);
int ext4_read_block(ext4_private_t *priv, uint64_t block_num, void *buffer);
int ext4_write_block(ext4_private_t *priv, uint64_t block_num, const void *buffer);
uint64_t ext4_get_inode_block(ext4_private_t *priv, const ext4_inode_t *inode, uint32_t logical_block);

// Utility functions
uint32_t ext4_calculate_checksum(const void *data, size_t size);
bool ext4_verify_checksum(const void *data, size_t size, uint32_t checksum);
uint64_t ext4_get_block_count(const ext4_superblock_t *sb);
uint64_t ext4_get_inode_size(const ext4_inode_t *inode);

/** @} */

#endif /* __EXT4_H__ */ 