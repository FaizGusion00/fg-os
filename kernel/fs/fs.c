/**
 * @file fs.c
 * @brief File System Core Implementation for FG-OS
 * 
 * This file implements the core file system functionality, providing a unified
 * interface for different file system types and managing the global file system
 * state including mounting, caching, and journaling.
 * 
 * @author Faiz Nasir
 * @company FGCompany Official
 * @version 1.0.0
 * @date 2024
 * @copyright © 2024 FGCompany Official. All rights reserved.
 */

#include "fs.h"
#include "fgfs.h"
#include "fat32.h"
#include "ext4.h"
#include "../include/kernel.h"
#include "../mm/heap.h"
#include "../hal/hal.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

// Global file system manager
fs_manager_t fs_manager = {0};

// Registered file systems
static fs_operations_t *registered_fs[FS_TYPE_DEVFS + 1] = {0};

/**
 * @brief Initialize the file system subsystem
 * 
 * Sets up the global file system manager, initializes caches,
 * and registers built-in file systems.
 * 
 * @return 0 on success, negative error code on failure
 */
int fs_init(void) {
    // Initialize manager structure
    memset(&fs_manager, 0, sizeof(fs_manager_t));
    
    // Initialize global cache
    fs_manager.global_cache = kmalloc(sizeof(fs_cache_t));
    if (!fs_manager.global_cache) {
        return -1; // ENOMEM
    }
    
    if (fs_cache_init(fs_manager.global_cache, 1024, DEFAULT_BLOCK_SIZE) != 0) {
        kfree(fs_manager.global_cache);
        return -1;
    }
    
    // Initialize global journal
    fs_manager.global_journal = kmalloc(sizeof(journal_t));
    if (!fs_manager.global_journal) {
        fs_cache_destroy(fs_manager.global_cache);
        kfree(fs_manager.global_cache);
        return -1;
    }
    
    if (fs_journal_init(fs_manager.global_journal, 1000, 100) != 0) {
        kfree(fs_manager.global_journal);
        fs_cache_destroy(fs_manager.global_cache);
        kfree(fs_manager.global_cache);
        return -1;
    }
    
    // Initialize file descriptor management
    fs_manager.next_fd = 3; // Reserve 0, 1, 2 for stdin, stdout, stderr
    fs_manager.mounted_count = 0;
    fs_manager.mount_points = NULL;
    
    // Register built-in file systems
    fgfs_init();
    fat32_init();
    ext4_init();
    
    fs_manager.initialized = true;
    
    return 0;
}

/**
 * @brief Shutdown the file system subsystem
 * 
 * Unmounts all file systems, destroys caches and journals,
 * and cleans up resources.
 */
void fs_shutdown(void) {
    if (!fs_manager.initialized) {
        return;
    }
    
    // Unmount all file systems
    mount_point_t *mount = fs_manager.mount_points;
    while (mount) {
        mount_point_t *next = mount->next;
        fs_unmount(mount->path);
        mount = next;
    }
    
    // Destroy global journal
    if (fs_manager.global_journal) {
        fs_journal_destroy(fs_manager.global_journal);
        kfree(fs_manager.global_journal);
    }
    
    // Destroy global cache
    if (fs_manager.global_cache) {
        fs_cache_destroy(fs_manager.global_cache);
        kfree(fs_manager.global_cache);
    }
    
    // Shutdown file system implementations
    ext4_shutdown();
    fat32_shutdown();
    fgfs_shutdown();
    
    fs_manager.initialized = false;
}

/**
 * @brief Register a file system type
 * 
 * @param type File system type
 * @param ops Operations structure
 * @return 0 on success, negative error code on failure
 */
int fs_register_filesystem(fs_type_t type, fs_operations_t *ops) {
    if (type >= FS_TYPE_DEVFS + 1 || !ops) {
        return -1; // EINVAL
    }
    
    registered_fs[type] = ops;
    return 0;
}

/**
 * @brief Mount a file system
 * 
 * @param device Device path
 * @param mount_point Mount point path
 * @param type File system type
 * @param flags Mount flags
 * @return 0 on success, negative error code on failure
 */
int fs_mount(const char *device, const char *mount_point, fs_type_t type, uint32_t flags) {
    if (!device || !mount_point || type >= FS_TYPE_DEVFS + 1) {
        return -1; // EINVAL
    }
    
    if (!fs_manager.initialized) {
        return -1; // ENOSYS
    }
    
    // Check if mount point already exists
    mount_point_t *existing = fs_manager.mount_points;
    while (existing) {
        if (strcmp(existing->path, mount_point) == 0) {
            return -1; // EBUSY
        }
        existing = existing->next;
    }
    
    // Get operations for file system type
    fs_operations_t *ops = registered_fs[type];
    if (!ops) {
        return -1; // ENODEV
    }
    
    // Create filesystem structure
    filesystem_t *fs = kmalloc(sizeof(filesystem_t));
    if (!fs) {
        return -1; // ENOMEM
    }
    
    memset(fs, 0, sizeof(filesystem_t));
    fs->type = type;
    fs->status = FS_STATUS_INITIALIZING;
    fs->ops = ops;
    fs->flags = flags;
    fs->ref_count = 1;
    strncpy(fs->device, device, sizeof(fs->device) - 1);
    strncpy(fs->name, fs_type_to_string(type), sizeof(fs->name) - 1);
    
    // Mount the file system
    int result = ops->mount(fs, device, flags);
    if (result != 0) {
        kfree(fs);
        return result;
    }
    
    // Create mount point structure
    mount_point_t *mount = kmalloc(sizeof(mount_point_t));
    if (!mount) {
        ops->unmount(fs);
        kfree(fs);
        return -1; // ENOMEM
    }
    
    strncpy(mount->path, mount_point, sizeof(mount->path) - 1);
    mount->fs = fs;
    mount->flags = flags;
    mount->mount_time = hal_get_timestamp();
    mount->next = fs_manager.mount_points;
    
    fs_manager.mount_points = mount;
    fs_manager.filesystems[fs_manager.mounted_count] = fs;
    fs_manager.mounted_count++;
    
    fs->status = FS_STATUS_MOUNTED;
    fs->mount_point = mount;
    
    return 0;
}

/**
 * @brief Unmount a file system
 * 
 * @param mount_point Mount point path
 * @return 0 on success, negative error code on failure
 */
int fs_unmount(const char *mount_point) {
    if (!mount_point || !fs_manager.initialized) {
        return -1; // EINVAL
    }
    
    // Find mount point
    mount_point_t *mount = fs_manager.mount_points;
    mount_point_t *prev = NULL;
    
    while (mount) {
        if (strcmp(mount->path, mount_point) == 0) {
            break;
        }
        prev = mount;
        mount = mount->next;
    }
    
    if (!mount) {
        return -1; // ENOENT
    }
    
    filesystem_t *fs = mount->fs;
    
    // Check if file system is busy
    if (fs->ref_count > 1) {
        return -1; // EBUSY
    }
    
    // Unmount the file system
    int result = fs->ops->unmount(fs);
    if (result != 0) {
        return result;
    }
    
    // Remove from mount list
    if (prev) {
        prev->next = mount->next;
    } else {
        fs_manager.mount_points = mount->next;
    }
    
    // Remove from filesystems array
    for (uint32_t i = 0; i < fs_manager.mounted_count; i++) {
        if (fs_manager.filesystems[i] == fs) {
            // Shift remaining entries
            for (uint32_t j = i; j < fs_manager.mounted_count - 1; j++) {
                fs_manager.filesystems[j] = fs_manager.filesystems[j + 1];
            }
            fs_manager.filesystems[fs_manager.mounted_count - 1] = NULL;
            fs_manager.mounted_count--;
            break;
        }
    }
    
    // Clean up
    kfree(fs);
    kfree(mount);
    
    return 0;
}

/**
 * @brief Get file system for a given path
 * 
 * @param path File path
 * @return Filesystem pointer or NULL if not found
 */
filesystem_t* fs_get_filesystem(const char *path) {
    if (!path || !fs_manager.initialized) {
        return NULL;
    }
    
    mount_point_t *best_match = NULL;
    size_t best_match_len = 0;
    
    // Find the longest matching mount point
    mount_point_t *mount = fs_manager.mount_points;
    while (mount) {
        size_t mount_len = strlen(mount->path);
        if (strncmp(path, mount->path, mount_len) == 0 && mount_len > best_match_len) {
            best_match = mount;
            best_match_len = mount_len;
        }
        mount = mount->next;
    }
    
    return best_match ? best_match->fs : NULL;
}

/**
 * @brief Open a file
 * 
 * @param path File path
 * @param mode Access mode
 * @param file Output file pointer
 * @return 0 on success, negative error code on failure
 */
int fs_open(const char *path, file_access_mode_t mode, file_t **file) {
    if (!path || !file) {
        return -1; // EINVAL
    }
    
    filesystem_t *fs = fs_get_filesystem(path);
    if (!fs) {
        return -1; // ENOENT
    }
    
    return fs->ops->open(fs, path, mode, file);
}

/**
 * @brief Close a file
 * 
 * @param file File pointer
 * @return 0 on success, negative error code on failure
 */
int fs_close(file_t *file) {
    if (!file || !file->fs) {
        return -1; // EINVAL
    }
    
    return file->fs->ops->close(file);
}

/**
 * @brief Read from a file
 * 
 * @param file File pointer
 * @param buffer Buffer to read into
 * @param size Number of bytes to read
 * @return Number of bytes read, or negative error code
 */
ssize_t fs_read(file_t *file, void *buffer, size_t size) {
    if (!file || !buffer || !file->fs) {
        return -1; // EINVAL
    }
    
    return file->fs->ops->read(file, buffer, size);
}

/**
 * @brief Write to a file
 * 
 * @param file File pointer
 * @param buffer Buffer to write from
 * @param size Number of bytes to write
 * @return Number of bytes written, or negative error code
 */
ssize_t fs_write(file_t *file, const void *buffer, size_t size) {
    if (!file || !buffer || !file->fs) {
        return -1; // EINVAL
    }
    
    return file->fs->ops->write(file, buffer, size);
}

/**
 * @brief Seek in a file
 * 
 * @param file File pointer
 * @param offset Offset to seek to
 * @param origin Seek origin
 * @return New file position, or negative error code
 */
int64_t fs_seek(file_t *file, int64_t offset, seek_origin_t origin) {
    if (!file || !file->fs) {
        return -1; // EINVAL
    }
    
    return file->fs->ops->seek(file, offset, origin);
}

/**
 * @brief Create a file
 * 
 * @param path File path
 * @param permissions File permissions
 * @return 0 on success, negative error code on failure
 */
int fs_create(const char *path, uint16_t permissions) {
    if (!path) {
        return -1; // EINVAL
    }
    
    filesystem_t *fs = fs_get_filesystem(path);
    if (!fs) {
        return -1; // ENOENT
    }
    
    return fs->ops->create(fs, path, permissions);
}

/**
 * @brief Create a directory
 * 
 * @param path Directory path
 * @param permissions Directory permissions
 * @return 0 on success, negative error code on failure
 */
int fs_mkdir(const char *path, uint16_t permissions) {
    if (!path) {
        return -1; // EINVAL
    }
    
    filesystem_t *fs = fs_get_filesystem(path);
    if (!fs) {
        return -1; // ENOENT
    }
    
    return fs->ops->mkdir(fs, path, permissions);
}

/**
 * @brief Get file statistics
 * 
 * @param path File path
 * @param metadata Output metadata structure
 * @return 0 on success, negative error code on failure
 */
int fs_stat(const char *path, file_metadata_t *metadata) {
    if (!path || !metadata) {
        return -1; // EINVAL
    }
    
    filesystem_t *fs = fs_get_filesystem(path);
    if (!fs) {
        return -1; // ENOENT
    }
    
    return fs->ops->stat(fs, path, metadata);
}

/**
 * @brief Convert file system type to string
 * 
 * @param type File system type
 * @return String representation
 */
const char* fs_type_to_string(fs_type_t type) {
    switch (type) {
        case FS_TYPE_FGFS:      return "FGFS";
        case FS_TYPE_FAT32:     return "FAT32";
        case FS_TYPE_NTFS:      return "NTFS";
        case FS_TYPE_EXT4:      return "ext4";
        case FS_TYPE_EXT3:      return "ext3";
        case FS_TYPE_ISO9660:   return "ISO9660";
        case FS_TYPE_TMPFS:     return "tmpfs";
        case FS_TYPE_PROCFS:    return "procfs";
        case FS_TYPE_DEVFS:     return "devfs";
        default:                return "unknown";
    }
}

/**
 * @brief Convert file system status to string
 * 
 * @param status File system status
 * @return String representation
 */
const char* fs_status_to_string(fs_status_t status) {
    switch (status) {
        case FS_STATUS_UNINITIALIZED:  return "uninitialized";
        case FS_STATUS_INITIALIZING:   return "initializing";
        case FS_STATUS_READY:          return "ready";
        case FS_STATUS_MOUNTED:        return "mounted";
        case FS_STATUS_UNMOUNTED:      return "unmounted";
        case FS_STATUS_ERROR:          return "error";
        case FS_STATUS_READ_ONLY:      return "read-only";
        case FS_STATUS_CORRUPTED:      return "corrupted";
        default:                       return "unknown";
    }
}

/**
 * @brief Convert file type to string
 * 
 * @param type File type
 * @return String representation
 */
const char* file_type_to_string(file_type_t type) {
    switch (type) {
        case FILE_TYPE_REGULAR:     return "regular";
        case FILE_TYPE_DIRECTORY:   return "directory";
        case FILE_TYPE_SYMLINK:     return "symlink";
        case FILE_TYPE_BLOCK_DEVICE: return "block device";
        case FILE_TYPE_CHAR_DEVICE: return "character device";
        case FILE_TYPE_FIFO:        return "fifo";
        case FILE_TYPE_SOCKET:      return "socket";
        default:                    return "unknown";
    }
}

// Additional utility functions would be implemented here...
// Cache management, journal management, path manipulation, etc.

/**
 * @brief Initialize file system cache
 * 
 * @param cache Cache structure
 * @param max_size Maximum cache size
 * @param block_size Block size
 * @return 0 on success, negative error code on failure
 */
int fs_cache_init(fs_cache_t *cache, uint32_t max_size, uint32_t block_size) {
    if (!cache) {
        return -1;
    }
    
    memset(cache, 0, sizeof(fs_cache_t));
    cache->max_size = max_size;
    cache->block_size = block_size;
    cache->entries = NULL;
    cache->size = 0;
    cache->hits = 0;
    cache->misses = 0;
    cache->dirty_blocks = 0;
    
    return 0;
}

/**
 * @brief Destroy file system cache
 * 
 * @param cache Cache structure
 */
void fs_cache_destroy(fs_cache_t *cache) {
    if (!cache) {
        return;
    }
    
    // Free all cache entries
    fs_cache_entry_t *entry = cache->entries;
    while (entry) {
        fs_cache_entry_t *next = entry->next;
        if (entry->data) {
            kfree(entry->data);
        }
        kfree(entry);
        entry = next;
    }
    
    memset(cache, 0, sizeof(fs_cache_t));
}

/**
 * @brief Initialize journal
 * 
 * @param journal Journal structure
 * @param start_block Starting block
 * @param size Journal size in blocks
 * @return 0 on success, negative error code on failure
 */
int fs_journal_init(journal_t *journal, uint64_t start_block, uint64_t size) {
    if (!journal) {
        return -1;
    }
    
    memset(journal, 0, sizeof(journal_t));
    journal->start_block = start_block;
    journal->size = size;
    journal->sequence = 1;
    journal->block_size = DEFAULT_BLOCK_SIZE;
    journal->enabled = true;
    
    return 0;
}

/**
 * @brief Destroy journal
 * 
 * @param journal Journal structure
 */
void fs_journal_destroy(journal_t *journal) {
    if (!journal) {
        return;
    }
    
    // Commit any pending transactions
    if (journal->enabled) {
        fs_journal_commit(journal);
    }
    
    if (journal->cache) {
        fs_cache_destroy(journal->cache);
        kfree(journal->cache);
    }
    
    memset(journal, 0, sizeof(journal_t));
}

/**
 * @brief Commit journal transactions
 * 
 * @param journal Journal structure
 * @return 0 on success, negative error code on failure
 */
int fs_journal_commit(journal_t *journal) {
    if (!journal || !journal->enabled) {
        return -1;
    }
    
    // Implementation would commit pending transactions
    // For now, just increment sequence
    journal->sequence++;
    
    return 0;
} 