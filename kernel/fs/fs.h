/**
 * @file fs.h
 * @brief File System Interface and Design for FG-OS
 * 
 * This file defines the core file system architecture, interfaces, and data structures
 * for FG-OS. It provides a unified interface for multiple file system types with
 * support for journaling, caching, and cross-platform compatibility.
 * 
 * @author Faiz Nasir
 * @company FGCompany Official
 * @version 1.0.0
 * @date 2024
 * @copyright © 2024 FGCompany Official. All rights reserved.
 */

#ifndef __FS_H__
#define __FS_H__

#include "../include/types.h"

/**
 * @defgroup filesystem File System
 * @brief File system architecture and implementation
 * @{
 */

// File system version information
#define FS_VERSION_MAJOR    1
#define FS_VERSION_MINOR    0
#define FS_VERSION_PATCH    0
#define FS_VERSION_STRING   "1.0.0"

// File system constants
#define MAX_FILENAME_LENGTH     255
#define MAX_PATH_LENGTH         4096
#define MAX_SYMLINK_DEPTH       8
#define MAX_OPEN_FILES          1024
#define MAX_MOUNTED_FILESYSTEMS 64

// Block and sector sizes
#define SECTOR_SIZE             512
#define DEFAULT_BLOCK_SIZE      4096
#define MAX_BLOCK_SIZE          65536
#define MIN_BLOCK_SIZE          512

// File system types
typedef enum {
    FS_TYPE_UNKNOWN = 0,
    FS_TYPE_FGFS,           /**< Native FG-OS file system */
    FS_TYPE_FAT32,          /**< FAT32 compatibility */
    FS_TYPE_NTFS,           /**< NTFS compatibility */
    FS_TYPE_EXT4,           /**< ext4 compatibility */
    FS_TYPE_EXT3,           /**< ext3 compatibility */
    FS_TYPE_ISO9660,        /**< ISO 9660 (CD-ROM) */
    FS_TYPE_TMPFS,          /**< Temporary file system */
    FS_TYPE_PROCFS,         /**< Process file system */
    FS_TYPE_DEVFS           /**< Device file system */
} fs_type_t;

// File types
typedef enum {
    FILE_TYPE_REGULAR = 0,
    FILE_TYPE_DIRECTORY,
    FILE_TYPE_SYMLINK,
    FILE_TYPE_BLOCK_DEVICE,
    FILE_TYPE_CHAR_DEVICE,
    FILE_TYPE_FIFO,
    FILE_TYPE_SOCKET,
    FILE_TYPE_UNKNOWN
} file_type_t;

// File permissions (POSIX-style)
typedef enum {
    PERM_OTHER_EXECUTE  = 0001,
    PERM_OTHER_WRITE    = 0002,
    PERM_OTHER_READ     = 0004,
    PERM_GROUP_EXECUTE  = 0010,
    PERM_GROUP_WRITE    = 0020,
    PERM_GROUP_READ     = 0040,
    PERM_OWNER_EXECUTE  = 0100,
    PERM_OWNER_WRITE    = 0200,
    PERM_OWNER_READ     = 0400,
    PERM_SETUID         = 04000,
    PERM_SETGID         = 02000,
    PERM_STICKY         = 01000
} file_permissions_t;

// File access modes
typedef enum {
    ACCESS_READ_ONLY = 0,
    ACCESS_WRITE_ONLY,
    ACCESS_READ_WRITE,
    ACCESS_APPEND,
    ACCESS_CREATE,
    ACCESS_TRUNCATE,
    ACCESS_EXCLUSIVE
} file_access_mode_t;

// File system status
typedef enum {
    FS_STATUS_UNINITIALIZED = 0,
    FS_STATUS_INITIALIZING,
    FS_STATUS_READY,
    FS_STATUS_MOUNTED,
    FS_STATUS_UNMOUNTED,
    FS_STATUS_ERROR,
    FS_STATUS_READ_ONLY,
    FS_STATUS_CORRUPTED
} fs_status_t;

// Mount flags
typedef enum {
    MOUNT_READ_ONLY     = (1 << 0),
    MOUNT_NO_EXEC       = (1 << 1),
    MOUNT_NO_SUID       = (1 << 2),
    MOUNT_NO_DEV        = (1 << 3),
    MOUNT_SYNCHRONOUS   = (1 << 4),
    MOUNT_JOURNALING    = (1 << 5),
    MOUNT_CACHE_ENABLED = (1 << 6),
    MOUNT_AUTO_REPAIR   = (1 << 7)
} mount_flags_t;

// Seek origins
typedef enum {
    SEEK_SET = 0,       /**< Seek from beginning */
    SEEK_CUR,          /**< Seek from current position */
    SEEK_END           /**< Seek from end */
} seek_origin_t;

// Forward declarations
struct file;
struct directory;
struct filesystem;
struct mount_point;
struct superblock;

// File system statistics
typedef struct {
    uint64_t total_size;        /**< Total file system size */
    uint64_t free_size;         /**< Free space available */
    uint64_t used_size;         /**< Used space */
    uint64_t total_inodes;      /**< Total number of inodes */
    uint64_t free_inodes;       /**< Free inodes available */
    uint32_t block_size;        /**< Block size in bytes */
    uint32_t fragment_size;     /**< Fragment size in bytes */
    uint32_t max_filename_len;  /**< Maximum filename length */
    uint32_t files_count;       /**< Total number of files */
    uint32_t directories_count; /**< Total number of directories */
} fs_stats_t;

// File metadata structure
typedef struct {
    uint64_t    inode;          /**< Inode number */
    file_type_t type;           /**< File type */
    uint64_t    size;           /**< File size in bytes */
    uint32_t    uid;            /**< Owner user ID */
    uint32_t    gid;            /**< Owner group ID */
    uint16_t    permissions;    /**< File permissions */
    uint64_t    created_time;   /**< Creation timestamp */
    uint64_t    modified_time;  /**< Last modification timestamp */
    uint64_t    accessed_time;  /**< Last access timestamp */
    uint32_t    hard_links;     /**< Number of hard links */
    uint32_t    block_count;    /**< Number of blocks allocated */
    uint32_t    flags;          /**< File flags */
    char        name[MAX_FILENAME_LENGTH]; /**< File name */
} file_metadata_t;

// Directory entry structure
typedef struct {
    uint64_t    inode;          /**< Inode number */
    file_type_t type;           /**< Entry type */
    uint16_t    name_length;    /**< Name length */
    char        name[MAX_FILENAME_LENGTH]; /**< Entry name */
} directory_entry_t;

// File handle structure
typedef struct file {
    uint32_t            fd;             /**< File descriptor */
    uint64_t            inode;          /**< Inode number */
    file_access_mode_t  mode;           /**< Access mode */
    uint64_t            position;       /**< Current file position */
    uint64_t            size;           /**< File size */
    uint32_t            flags;          /**< File flags */
    struct filesystem   *fs;            /**< Associated file system */
    struct mount_point  *mount;         /**< Mount point */
    void                *private_data;  /**< File system specific data */
    uint32_t            ref_count;      /**< Reference count */
    bool                modified;       /**< Modification flag */
} file_t;

// Directory handle structure
typedef struct directory {
    uint32_t            fd;             /**< Directory descriptor */
    uint64_t            inode;          /**< Directory inode */
    uint64_t            position;       /**< Current position */
    struct filesystem   *fs;            /**< Associated file system */
    struct mount_point  *mount;         /**< Mount point */
    void                *private_data;  /**< File system specific data */
    uint32_t            ref_count;      /**< Reference count */
} directory_t;

// Superblock structure (generic)
typedef struct superblock {
    uint32_t    magic;              /**< Magic number */
    fs_type_t   type;               /**< File system type */
    uint32_t    version;            /**< File system version */
    uint64_t    total_size;         /**< Total size in bytes */
    uint64_t    free_size;          /**< Free space in bytes */
    uint32_t    block_size;         /**< Block size */
    uint32_t    blocks_per_group;   /**< Blocks per group */
    uint64_t    total_blocks;       /**< Total number of blocks */
    uint64_t    free_blocks;        /**< Free blocks */
    uint64_t    total_inodes;       /**< Total inodes */
    uint64_t    free_inodes;        /**< Free inodes */
    uint64_t    root_inode;         /**< Root directory inode */
    uint32_t    mount_count;        /**< Mount count */
    uint32_t    max_mount_count;    /**< Maximum mount count */
    uint64_t    last_check;         /**< Last check timestamp */
    uint64_t    check_interval;     /**< Check interval */
    uint32_t    flags;              /**< File system flags */
    uint8_t     label[16];          /**< Volume label */
    uint8_t     uuid[16];           /**< UUID */
} superblock_t;

// File system operations structure
typedef struct fs_operations {
    // Mount/unmount operations
    int (*mount)(struct filesystem *fs, const char *device, uint32_t flags);
    int (*unmount)(struct filesystem *fs);
    int (*remount)(struct filesystem *fs, uint32_t flags);
    
    // File operations
    int (*open)(struct filesystem *fs, const char *path, file_access_mode_t mode, file_t **file);
    int (*close)(file_t *file);
    ssize_t (*read)(file_t *file, void *buffer, size_t size);
    ssize_t (*write)(file_t *file, const void *buffer, size_t size);
    int64_t (*seek)(file_t *file, int64_t offset, seek_origin_t origin);
    int (*flush)(file_t *file);
    int (*truncate)(file_t *file, uint64_t size);
    
    // Directory operations
    int (*opendir)(struct filesystem *fs, const char *path, directory_t **dir);
    int (*closedir)(directory_t *dir);
    int (*readdir)(directory_t *dir, directory_entry_t *entry);
    int (*mkdir)(struct filesystem *fs, const char *path, uint16_t permissions);
    int (*rmdir)(struct filesystem *fs, const char *path);
    
    // File management operations
    int (*create)(struct filesystem *fs, const char *path, uint16_t permissions);
    int (*unlink)(struct filesystem *fs, const char *path);
    int (*rename)(struct filesystem *fs, const char *old_path, const char *new_path);
    int (*link)(struct filesystem *fs, const char *old_path, const char *new_path);
    int (*symlink)(struct filesystem *fs, const char *target, const char *link_path);
    
    // Metadata operations
    int (*stat)(struct filesystem *fs, const char *path, file_metadata_t *metadata);
    int (*chmod)(struct filesystem *fs, const char *path, uint16_t permissions);
    int (*chown)(struct filesystem *fs, const char *path, uint32_t uid, uint32_t gid);
    int (*utime)(struct filesystem *fs, const char *path, uint64_t atime, uint64_t mtime);
    
    // File system management
    int (*sync)(struct filesystem *fs);
    int (*get_stats)(struct filesystem *fs, fs_stats_t *stats);
    int (*check)(struct filesystem *fs, bool repair);
    int (*resize)(struct filesystem *fs, uint64_t new_size);
} fs_operations_t;

// File system structure
typedef struct filesystem {
    fs_type_t           type;           /**< File system type */
    fs_status_t         status;         /**< Current status */
    char                name[32];       /**< File system name */
    char                device[64];     /**< Device path */
    superblock_t        *superblock;    /**< Superblock */
    fs_operations_t     *ops;           /**< Operations table */
    void                *private_data;  /**< File system specific data */
    uint32_t            flags;          /**< Mount flags */
    uint32_t            ref_count;      /**< Reference count */
    struct mount_point  *mount_point;   /**< Associated mount point */
} filesystem_t;

// Mount point structure
typedef struct mount_point {
    char            path[MAX_PATH_LENGTH];  /**< Mount path */
    filesystem_t    *fs;                    /**< File system */
    uint32_t        flags;                  /**< Mount flags */
    uint64_t        mount_time;             /**< Mount timestamp */
    struct mount_point *next;               /**< Next mount point */
} mount_point_t;

// File system cache entry
typedef struct fs_cache_entry {
    uint64_t                block_number;   /**< Block number */
    void                    *data;          /**< Block data */
    uint32_t                size;           /**< Block size */
    bool                    dirty;          /**< Dirty flag */
    uint64_t                access_time;    /**< Last access time */
    uint32_t                ref_count;      /**< Reference count */
    struct fs_cache_entry   *next;          /**< Next cache entry */
    struct fs_cache_entry   *prev;          /**< Previous cache entry */
} fs_cache_entry_t;

// File system cache
typedef struct fs_cache {
    fs_cache_entry_t    *entries;           /**< Cache entries */
    uint32_t            size;               /**< Cache size */
    uint32_t            max_size;           /**< Maximum cache size */
    uint32_t            block_size;         /**< Block size */
    uint64_t            hits;               /**< Cache hits */
    uint64_t            misses;             /**< Cache misses */
    uint32_t            dirty_blocks;       /**< Number of dirty blocks */
} fs_cache_t;

// Journal entry structure
typedef struct journal_entry {
    uint32_t    magic;              /**< Magic number */
    uint32_t    sequence;           /**< Sequence number */
    uint32_t    type;               /**< Entry type */
    uint64_t    timestamp;          /**< Timestamp */
    uint64_t    block_number;       /**< Block number */
    uint32_t    data_size;          /**< Data size */
    uint32_t    checksum;           /**< Checksum */
    uint8_t     data[];             /**< Entry data */
} journal_entry_t;

// Journal structure
typedef struct journal {
    uint64_t            start_block;        /**< Journal start block */
    uint64_t            size;               /**< Journal size in blocks */
    uint32_t            sequence;           /**< Current sequence number */
    uint32_t            block_size;         /**< Block size */
    bool                enabled;            /**< Journal enabled flag */
    fs_cache_t          *cache;             /**< Journal cache */
    journal_entry_t     *current_entry;     /**< Current entry */
} journal_t;

// Global file system manager
typedef struct fs_manager {
    filesystem_t        *filesystems[MAX_MOUNTED_FILESYSTEMS];
    mount_point_t       *mount_points;
    file_t              *open_files[MAX_OPEN_FILES];
    uint32_t            next_fd;
    fs_cache_t          *global_cache;
    journal_t           *global_journal;
    uint32_t            mounted_count;
    bool                initialized;
} fs_manager_t;

// Global file system manager instance
extern fs_manager_t fs_manager;

// Core file system functions
int fs_init(void);
void fs_shutdown(void);
int fs_register_filesystem(fs_type_t type, fs_operations_t *ops);
int fs_mount(const char *device, const char *mount_point, fs_type_t type, uint32_t flags);
int fs_unmount(const char *mount_point);
filesystem_t* fs_get_filesystem(const char *path);

// File operations
int fs_open(const char *path, file_access_mode_t mode, file_t **file);
int fs_close(file_t *file);
ssize_t fs_read(file_t *file, void *buffer, size_t size);
ssize_t fs_write(file_t *file, const void *buffer, size_t size);
int64_t fs_seek(file_t *file, int64_t offset, seek_origin_t origin);
int fs_flush(file_t *file);
int fs_truncate(file_t *file, uint64_t size);

// Directory operations
int fs_opendir(const char *path, directory_t **dir);
int fs_closedir(directory_t *dir);
int fs_readdir(directory_t *dir, directory_entry_t *entry);
int fs_mkdir(const char *path, uint16_t permissions);
int fs_rmdir(const char *path);

// File management operations
int fs_create(const char *path, uint16_t permissions);
int fs_unlink(const char *path);
int fs_rename(const char *old_path, const char *new_path);
int fs_link(const char *old_path, const char *new_path);
int fs_symlink(const char *target, const char *link_path);

// Metadata operations
int fs_stat(const char *path, file_metadata_t *metadata);
int fs_chmod(const char *path, uint16_t permissions);
int fs_chown(const char *path, uint32_t uid, uint32_t gid);
int fs_utime(const char *path, uint64_t atime, uint64_t mtime);

// File system management
int fs_sync(const char *mount_point);
int fs_get_stats(const char *mount_point, fs_stats_t *stats);
int fs_check(const char *mount_point, bool repair);

// Cache management
int fs_cache_init(fs_cache_t *cache, uint32_t max_size, uint32_t block_size);
void fs_cache_destroy(fs_cache_t *cache);
fs_cache_entry_t* fs_cache_get(fs_cache_t *cache, uint64_t block_number);
int fs_cache_put(fs_cache_t *cache, uint64_t block_number, void *data, uint32_t size);
int fs_cache_flush(fs_cache_t *cache);
void fs_cache_invalidate(fs_cache_t *cache, uint64_t block_number);

// Journal management
int fs_journal_init(journal_t *journal, uint64_t start_block, uint64_t size);
void fs_journal_destroy(journal_t *journal);
int fs_journal_write_entry(journal_t *journal, uint32_t type, uint64_t block, const void *data, uint32_t size);
int fs_journal_replay(journal_t *journal);
int fs_journal_commit(journal_t *journal);

// Utility functions
const char* fs_type_to_string(fs_type_t type);
const char* fs_status_to_string(fs_status_t status);
const char* file_type_to_string(file_type_t type);
bool fs_is_valid_filename(const char *name);
int fs_normalize_path(const char *path, char *normalized, size_t size);
uint32_t fs_calculate_checksum(const void *data, size_t size);

// Path manipulation functions
int fs_dirname(const char *path, char *dirname, size_t size);
int fs_basename(const char *path, char *basename, size_t size);
int fs_join_path(const char *dir, const char *name, char *result, size_t size);
bool fs_is_absolute_path(const char *path);

// Permission checking functions
bool fs_check_permission(file_metadata_t *metadata, uint32_t uid, uint32_t gid, file_permissions_t perm);
uint16_t fs_apply_umask(uint16_t permissions, uint16_t umask);

/** @} */

#endif /* __FS_H__ */ 