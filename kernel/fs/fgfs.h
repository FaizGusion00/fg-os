/**
 * @file fgfs.h
 * @brief Native FG-OS File System (FGFS) Specification
 * 
 * This file defines the native FG-OS file system architecture, data structures,
 * and on-disk layout. FGFS is designed for high performance, reliability, and
 * modern features including advanced journaling, compression, and encryption.
 * 
 * @author Faiz Nasir
 * @company FGCompany Official
 * @version 1.0.0
 * @date 2024
 * @copyright © 2024 FGCompany Official. All rights reserved.
 */

#ifndef __FGFS_H__
#define __FGFS_H__

#include "fs.h"

/**
 * @defgroup fgfs Native FG-OS File System
 * @brief Native file system implementation
 * @{
 */

// FGFS Magic Numbers and Version
#define FGFS_MAGIC              0x46474653  // "FGFS"
#define FGFS_VERSION_MAJOR      1
#define FGFS_VERSION_MINOR      0
#define FGFS_VERSION_PATCH      0
#define FGFS_VERSION            ((FGFS_VERSION_MAJOR << 16) | (FGFS_VERSION_MINOR << 8) | FGFS_VERSION_PATCH)

// FGFS Constants
#define FGFS_BLOCK_SIZE_MIN     512
#define FGFS_BLOCK_SIZE_MAX     65536
#define FGFS_BLOCK_SIZE_DEFAULT 4096
#define FGFS_MAX_FILENAME       255
#define FGFS_MAX_PATH           4096
#define FGFS_MAX_SYMLINK_SIZE   1024

// FGFS Limits
#define FGFS_MAX_FILE_SIZE      (1ULL << 48)  // 256 TB
#define FGFS_MAX_FS_SIZE        (1ULL << 60)  // 1 EB
#define FGFS_MAX_INODES         (1ULL << 32)  // 4 billion
#define FGFS_MAX_EXTENT_LENGTH  (1ULL << 15)  // 32K blocks

// Block Group Configuration
#define FGFS_BLOCKS_PER_GROUP_MIN   1024
#define FGFS_BLOCKS_PER_GROUP_MAX   32768
#define FGFS_BLOCKS_PER_GROUP_DEFAULT 8192
#define FGFS_INODES_PER_GROUP_DEFAULT 2048

// Journal Configuration
#define FGFS_JOURNAL_MIN_SIZE   (1024 * 1024)      // 1 MB
#define FGFS_JOURNAL_MAX_SIZE   (1024 * 1024 * 1024) // 1 GB
#define FGFS_JOURNAL_DEFAULT_SIZE (32 * 1024 * 1024) // 32 MB

// Feature Flags
typedef enum {
    FGFS_FEATURE_SPARSE_SUPER   = (1 << 0),  /**< Sparse superblock copies */
    FGFS_FEATURE_LARGE_FILE     = (1 << 1),  /**< Large file support */
    FGFS_FEATURE_EXTENT         = (1 << 2),  /**< Extent-based allocation */
    FGFS_FEATURE_JOURNAL        = (1 << 3),  /**< Journaling support */
    FGFS_FEATURE_COMPRESSION    = (1 << 4),  /**< Compression support */
    FGFS_FEATURE_ENCRYPTION     = (1 << 5),  /**< Encryption support */
    FGFS_FEATURE_CHECKSUMS      = (1 << 6),  /**< Block checksums */
    FGFS_FEATURE_SNAPSHOTS      = (1 << 7),  /**< Snapshot support */
    FGFS_FEATURE_QUOTA          = (1 << 8),  /**< Quota support */
    FGFS_FEATURE_ACL            = (1 << 9),  /**< Access Control Lists */
    FGFS_FEATURE_XATTR          = (1 << 10), /**< Extended attributes */
    FGFS_FEATURE_DEDUPLICATION  = (1 << 11), /**< Deduplication support */
    FGFS_FEATURE_COPY_ON_WRITE  = (1 << 12), /**< Copy-on-write support */
    FGFS_FEATURE_REFLINK        = (1 << 13), /**< Reflink support */
    FGFS_FEATURE_ONLINE_RESIZE  = (1 << 14), /**< Online resize support */
    FGFS_FEATURE_METADATA_CSUM  = (1 << 15)  /**< Metadata checksums */
} fgfs_feature_flags_t;

// Inode Flags
typedef enum {
    FGFS_INODE_SECRM        = (1 << 0),  /**< Secure deletion */
    FGFS_INODE_UNRM         = (1 << 1),  /**< Undelete */
    FGFS_INODE_COMPR        = (1 << 2),  /**< Compress file */
    FGFS_INODE_SYNC         = (1 << 3),  /**< Synchronous updates */
    FGFS_INODE_IMMUTABLE    = (1 << 4),  /**< Immutable file */
    FGFS_INODE_APPEND       = (1 << 5),  /**< Append-only */
    FGFS_INODE_NODUMP       = (1 << 6),  /**< No dump */
    FGFS_INODE_NOATIME      = (1 << 7),  /**< No access time update */
    FGFS_INODE_DIRTY        = (1 << 8),  /**< Dirty */
    FGFS_INODE_COMPRBLKS    = (1 << 9),  /**< Compressed blocks */
    FGFS_INODE_NOCOMPR      = (1 << 10), /**< Don't compress */
    FGFS_INODE_ENCRYPT      = (1 << 11), /**< Encrypted inode */
    FGFS_INODE_BTREE_FL     = (1 << 12), /**< B-tree format directory */
    FGFS_INODE_INDEX_FL     = (1 << 13), /**< Hash indexed directory */
    FGFS_INODE_IMAGIC_FL    = (1 << 14), /**< AFS directory */
    FGFS_INODE_JOURNAL_DATA = (1 << 15), /**< Journal file data */
    FGFS_INODE_NOTAIL       = (1 << 16), /**< File tail not merged */
    FGFS_INODE_DIRSYNC      = (1 << 17), /**< Synchronous directory */
    FGFS_INODE_TOPDIR       = (1 << 18), /**< Top directory */
    FGFS_INODE_HUGE_FILE    = (1 << 19), /**< Huge file */
    FGFS_INODE_EXTENTS      = (1 << 20), /**< Uses extents */
    FGFS_INODE_VERITY       = (1 << 21), /**< Verity protected */
    FGFS_INODE_EA_INODE     = (1 << 22)  /**< Extended attribute inode */
} fgfs_inode_flags_t;

// Compression Algorithms
typedef enum {
    FGFS_COMPRESS_NONE = 0,
    FGFS_COMPRESS_LZ4,
    FGFS_COMPRESS_ZSTD,
    FGFS_COMPRESS_GZIP,
    FGFS_COMPRESS_BZIP2,
    FGFS_COMPRESS_XZ
} fgfs_compression_t;

// Encryption Algorithms
typedef enum {
    FGFS_ENCRYPT_NONE = 0,
    FGFS_ENCRYPT_AES_128_CBC,
    FGFS_ENCRYPT_AES_256_CBC,
    FGFS_ENCRYPT_AES_128_GCM,
    FGFS_ENCRYPT_AES_256_GCM,
    FGFS_ENCRYPT_CHACHA20_POLY1305
} fgfs_encryption_t;

// Checksum Algorithms
typedef enum {
    FGFS_CHECKSUM_NONE = 0,
    FGFS_CHECKSUM_CRC32,
    FGFS_CHECKSUM_CRC32C,
    FGFS_CHECKSUM_SHA256,
    FGFS_CHECKSUM_BLAKE2B
} fgfs_checksum_t;

// FGFS Superblock Structure
typedef struct {
    uint32_t    magic;                  /**< Magic number (FGFS_MAGIC) */
    uint32_t    version;                /**< File system version */
    uint64_t    block_count;            /**< Total blocks */
    uint64_t    free_blocks;            /**< Free blocks */
    uint64_t    inode_count;            /**< Total inodes */
    uint64_t    free_inodes;            /**< Free inodes */
    uint32_t    block_size;             /**< Block size in bytes */
    uint32_t    fragment_size;          /**< Fragment size in bytes */
    uint32_t    blocks_per_group;       /**< Blocks per block group */
    uint32_t    inodes_per_group;       /**< Inodes per block group */
    uint32_t    group_count;            /**< Number of block groups */
    uint64_t    root_inode;             /**< Root directory inode */
    uint64_t    lost_found_inode;       /**< Lost+found directory inode */
    uint64_t    journal_inode;          /**< Journal inode */
    uint64_t    first_data_block;       /**< First data block */
    uint32_t    max_mount_count;        /**< Maximum mount count */
    uint32_t    mount_count;            /**< Current mount count */
    uint64_t    last_mounted;           /**< Last mount time */
    uint64_t    last_written;           /**< Last write time */
    uint64_t    last_checked;           /**< Last check time */
    uint64_t    check_interval;         /**< Check interval */
    uint32_t    state;                  /**< File system state */
    uint32_t    errors;                 /**< Error behavior */
    uint32_t    feature_compat;         /**< Compatible features */
    uint32_t    feature_incompat;       /**< Incompatible features */
    uint32_t    feature_ro_compat;      /**< Read-only compatible features */
    uint8_t     uuid[16];               /**< File system UUID */
    uint8_t     volume_name[16];        /**< Volume name */
    uint8_t     last_mounted_on[64];    /**< Last mount path */
    uint32_t    algorithm_usage_bitmap; /**< Compression algorithm bitmap */
    uint32_t    default_mount_opts;     /**< Default mount options */
    uint32_t    first_meta_bg;          /**< First metablock group */
    uint64_t    mkfs_time;              /**< Creation time */
    uint32_t    reserved[25];           /**< Reserved for future use */
    uint32_t    checksum;               /**< Superblock checksum */
} __attribute__((packed)) fgfs_superblock_t;

// FGFS Block Group Descriptor
typedef struct {
    uint64_t    block_bitmap;           /**< Block bitmap block */
    uint64_t    inode_bitmap;           /**< Inode bitmap block */
    uint64_t    inode_table;            /**< Inode table block */
    uint32_t    free_blocks_count;      /**< Free blocks count */
    uint32_t    free_inodes_count;      /**< Free inodes count */
    uint32_t    used_dirs_count;        /**< Used directories count */
    uint16_t    flags;                  /**< Flags */
    uint16_t    exclude_bitmap_lo;      /**< Exclude bitmap low */
    uint32_t    block_bitmap_csum_lo;   /**< Block bitmap checksum low */
    uint32_t    inode_bitmap_csum_lo;   /**< Inode bitmap checksum low */
    uint32_t    itable_unused_lo;       /**< Unused inodes count low */
    uint16_t    checksum;               /**< Group descriptor checksum */
    uint32_t    exclude_bitmap_hi;      /**< Exclude bitmap high */
    uint32_t    block_bitmap_csum_hi;   /**< Block bitmap checksum high */
    uint32_t    inode_bitmap_csum_hi;   /**< Inode bitmap checksum high */
    uint32_t    itable_unused_hi;       /**< Unused inodes count high */
    uint32_t    reserved;               /**< Reserved */
} __attribute__((packed)) fgfs_group_desc_t;

// FGFS Extent Structure
typedef struct {
    uint32_t    logical_block;          /**< Logical block number */
    uint16_t    length;                 /**< Length in blocks */
    uint16_t    start_hi;               /**< Physical block high */
    uint32_t    start_lo;               /**< Physical block low */
} __attribute__((packed)) fgfs_extent_t;

// FGFS Extent Index
typedef struct {
    uint32_t    logical_block;          /**< Logical block number */
    uint32_t    leaf_lo;                /**< Leaf node low */
    uint16_t    leaf_hi;                /**< Leaf node high */
    uint16_t    unused;                 /**< Unused */
} __attribute__((packed)) fgfs_extent_idx_t;

// FGFS Extent Header
typedef struct {
    uint16_t    magic;                  /**< Magic number */
    uint16_t    entries;                /**< Number of entries */
    uint16_t    max;                    /**< Maximum entries */
    uint16_t    depth;                  /**< Tree depth */
    uint32_t    generation;             /**< Generation */
} __attribute__((packed)) fgfs_extent_header_t;

// FGFS Inode Structure
typedef struct {
    uint16_t    mode;                   /**< File mode */
    uint16_t    uid;                    /**< Owner UID low */
    uint64_t    size;                   /**< File size in bytes */
    uint64_t    atime;                  /**< Access time */
    uint64_t    ctime;                  /**< Change time */
    uint64_t    mtime;                  /**< Modification time */
    uint64_t    dtime;                  /**< Deletion time */
    uint16_t    gid;                    /**< Group GID low */
    uint16_t    links_count;            /**< Hard links count */
    uint64_t    blocks;                 /**< Block count */
    uint32_t    flags;                  /**< File flags */
    uint32_t    version;                /**< Version */
    union {
        struct {
            uint32_t    block[15];      /**< Direct/indirect blocks */
        } blocks;
        struct {
            fgfs_extent_header_t header; /**< Extent header */
            fgfs_extent_t extent[4];    /**< Extent entries */
        } extents;
    } data;
    uint32_t    generation;             /**< File version */
    uint32_t    file_acl;               /**< File ACL */
    uint32_t    size_high;              /**< File size high */
    uint32_t    fragment_addr;          /**< Fragment address */
    uint16_t    uid_high;               /**< Owner UID high */
    uint16_t    gid_high;               /**< Group GID high */
    uint32_t    checksum_lo;            /**< Checksum low */
    uint16_t    extra_isize;            /**< Extra inode size */
    uint16_t    checksum_hi;            /**< Checksum high */
    uint64_t    crtime;                 /**< Creation time */
    uint32_t    crtime_extra;           /**< Creation time extra */
    uint32_t    version_hi;             /**< Version high */
    uint32_t    projid;                 /**< Project ID */
} __attribute__((packed)) fgfs_inode_t;

// FGFS Directory Entry Structure
typedef struct {
    uint64_t    inode;                  /**< Inode number */
    uint16_t    rec_len;                /**< Record length */
    uint8_t     name_len;               /**< Name length */
    uint8_t     file_type;              /**< File type */
    char        name[];                 /**< File name */
} __attribute__((packed)) fgfs_dir_entry_t;

// FGFS Extended Attribute Header
typedef struct {
    uint32_t    magic;                  /**< Magic number */
    uint32_t    refcount;               /**< Reference count */
    uint32_t    blocks;                 /**< Number of blocks */
    uint32_t    hash;                   /**< Hash value */
    uint32_t    checksum;               /**< Checksum */
    uint32_t    reserved[3];            /**< Reserved */
} __attribute__((packed)) fgfs_xattr_header_t;

// FGFS Extended Attribute Entry
typedef struct {
    uint8_t     name_len;               /**< Name length */
    uint8_t     name_index;             /**< Name index */
    uint16_t    value_offs;             /**< Value offset */
    uint32_t    value_ino;              /**< Value inode */
    uint32_t    value_size;             /**< Value size */
    uint32_t    hash;                   /**< Hash value */
    char        name[];                 /**< Attribute name */
} __attribute__((packed)) fgfs_xattr_entry_t;

// FGFS Journal Superblock
typedef struct {
    uint32_t    magic;                  /**< Magic number */
    uint32_t    blocktype;              /**< Block type */
    uint32_t    sequence;               /**< Sequence number */
    uint32_t    blocksize;              /**< Block size */
    uint32_t    maxlen;                 /**< Maximum length */
    uint32_t    first;                  /**< First block */
    uint32_t    start;                  /**< Start of log */
    uint32_t    errno;                  /**< Error number */
    uint32_t    feature_compat;         /**< Compatible features */
    uint32_t    feature_incompat;       /**< Incompatible features */
    uint32_t    feature_ro_compat;      /**< Read-only compatible features */
    uint8_t     uuid[16];               /**< Journal UUID */
    uint32_t    nr_users;               /**< Number of users */
    uint32_t    dynsuper;               /**< Dynamic superblock */
    uint32_t    max_transaction;        /**< Maximum transaction */
    uint32_t    max_trans_data;         /**< Maximum transaction data */
    uint32_t    checksum_type;          /**< Checksum type */
    uint32_t    checksum_size;          /**< Checksum size */
    uint32_t    reserved[42];           /**< Reserved */
    uint32_t    checksum;               /**< Checksum */
    uint8_t     users[16*48];           /**< User array */
} __attribute__((packed)) fgfs_journal_superblock_t;

// FGFS Journal Header
typedef struct {
    uint32_t    magic;                  /**< Magic number */
    uint32_t    blocktype;              /**< Block type */
    uint32_t    sequence;               /**< Sequence number */
} __attribute__((packed)) fgfs_journal_header_t;

// FGFS Private Data Structure
typedef struct {
    fgfs_superblock_t   *superblock;    /**< Superblock */
    fgfs_group_desc_t   *group_descs;   /**< Group descriptors */
    uint32_t            group_desc_count; /**< Group descriptor count */
    uint32_t            block_size;      /**< Block size */
    uint32_t            inode_size;      /**< Inode size */
    uint32_t            inodes_per_block; /**< Inodes per block */
    uint32_t            blocks_per_group; /**< Blocks per group */
    uint32_t            inodes_per_group; /**< Inodes per group */
    uint64_t            next_generation; /**< Next generation number */
    bool                has_journal;     /**< Has journal */
    uint64_t            journal_inode;   /**< Journal inode */
    fs_cache_t          *cache;          /**< File system cache */
    journal_t           *journal;        /**< Journal */
} fgfs_private_t;

// FGFS Function Declarations
int fgfs_init(void);
void fgfs_shutdown(void);
int fgfs_format(const char *device, uint64_t size, uint32_t block_size, const char *label);
int fgfs_mount(filesystem_t *fs, const char *device, uint32_t flags);
int fgfs_unmount(filesystem_t *fs);

// File operations
int fgfs_open(filesystem_t *fs, const char *path, file_access_mode_t mode, file_t **file);
int fgfs_close(file_t *file);
ssize_t fgfs_read(file_t *file, void *buffer, size_t size);
ssize_t fgfs_write(file_t *file, const void *buffer, size_t size);
int64_t fgfs_seek(file_t *file, int64_t offset, seek_origin_t origin);

// Directory operations
int fgfs_opendir(filesystem_t *fs, const char *path, directory_t **dir);
int fgfs_readdir(directory_t *dir, directory_entry_t *entry);
int fgfs_mkdir(filesystem_t *fs, const char *path, uint16_t permissions);

// File management
int fgfs_create(filesystem_t *fs, const char *path, uint16_t permissions);
int fgfs_unlink(filesystem_t *fs, const char *path);
int fgfs_stat(filesystem_t *fs, const char *path, file_metadata_t *metadata);

// Internal functions
uint64_t fgfs_alloc_inode(fgfs_private_t *priv);
void fgfs_free_inode(fgfs_private_t *priv, uint64_t inode);
uint64_t fgfs_alloc_block(fgfs_private_t *priv);
void fgfs_free_block(fgfs_private_t *priv, uint64_t block);
int fgfs_read_inode(fgfs_private_t *priv, uint64_t inode_num, fgfs_inode_t *inode);
int fgfs_write_inode(fgfs_private_t *priv, uint64_t inode_num, const fgfs_inode_t *inode);
int fgfs_read_block(fgfs_private_t *priv, uint64_t block_num, void *buffer);
int fgfs_write_block(fgfs_private_t *priv, uint64_t block_num, const void *buffer);

// Utility functions
uint32_t fgfs_calculate_checksum(const void *data, size_t size, fgfs_checksum_t type);
bool fgfs_verify_checksum(const void *data, size_t size, uint32_t checksum, fgfs_checksum_t type);
int fgfs_compress_block(const void *input, size_t input_size, void *output, size_t *output_size, fgfs_compression_t type);
int fgfs_decompress_block(const void *input, size_t input_size, void *output, size_t *output_size, fgfs_compression_t type);

/** @} */

#endif /* __FGFS_H__ */ 