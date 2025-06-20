/**
 * @file types.h
 * @brief Basic type definitions for FG-OS kernel
 * 
 * This file contains fundamental type definitions used throughout the kernel.
 * It provides portable integer types, boolean values, and commonly used structures.
 * 
 * @author Faiz Nasir
 * @company FGCompany Official
 * @version 1.0.0
 * @date 2024
 * @copyright © 2024 FGCompany Official. All rights reserved.
 */

#ifndef __TYPES_H__
#define __TYPES_H__

// Standard integer types
typedef unsigned char       uint8_t;    /**< 8-bit unsigned integer */
typedef signed char         int8_t;     /**< 8-bit signed integer */
typedef unsigned short      uint16_t;   /**< 16-bit unsigned integer */
typedef signed short        int16_t;    /**< 16-bit signed integer */
typedef unsigned int        uint32_t;   /**< 32-bit unsigned integer */
typedef signed int          int32_t;    /**< 32-bit signed integer */
typedef unsigned long long  uint64_t;   /**< 64-bit unsigned integer */
typedef signed long long    int64_t;    /**< 64-bit signed integer */

// Size and pointer types
typedef uint64_t            size_t;     /**< Size type for memory operations */
typedef int64_t             ssize_t;    /**< Signed size type */
typedef uint64_t            uintptr_t;  /**< Unsigned integer type for pointers */
typedef int64_t             intptr_t;   /**< Signed integer type for pointers */
typedef uint64_t            addr_t;     /**< Address type */
typedef uint64_t            phys_addr_t; /**< Physical address type */
typedef uint64_t            virt_addr_t; /**< Virtual address type */

// Process and thread types
typedef int32_t             pid_t;      /**< Process ID type */
typedef int32_t             tid_t;      /**< Thread ID type */
typedef uint32_t            uid_t;      /**< User ID type */
typedef uint32_t            gid_t;      /**< Group ID type */

// Time types
typedef uint64_t            ktime_t;    /**< Kernel time type in seconds */
typedef uint64_t            usec_t;     /**< Time type in microseconds */
typedef uint64_t            nsec_t;     /**< Time type in nanoseconds */

// File system types
typedef uint64_t            ino_t;      /**< Inode number type */
typedef uint32_t            mode_t;     /**< File mode type */
typedef uint64_t            koff_t;     /**< Kernel file offset type */
typedef uint32_t            blksize_t;  /**< Block size type */
typedef uint64_t            blkcnt_t;   /**< Block count type */

// Boolean type
typedef enum {
    false = 0,                          /**< Boolean false value */
    true  = 1                           /**< Boolean true value */
} bool;

// NULL pointer definition
#ifndef NULL
    #define NULL ((void *)0)
#endif

// Endianness macros
#define LITTLE_ENDIAN   1234
#define BIG_ENDIAN      4321
#define BYTE_ORDER      LITTLE_ENDIAN

// Byte swapping macros
#define SWAP16(x) ((uint16_t)(((x) << 8) | ((x) >> 8)))
#define SWAP32(x) ((uint32_t)(((x) << 24) | (((x) << 8) & 0xFF0000) | \
                             (((x) >> 8) & 0xFF00) | ((x) >> 24)))
#define SWAP64(x) ((uint64_t)(((uint64_t)SWAP32(x) << 32) | SWAP32((x) >> 32)))

// Endian conversion macros
#if BYTE_ORDER == LITTLE_ENDIAN
    #define le16toh(x)  (x)
    #define le32toh(x)  (x)
    #define le64toh(x)  (x)
    #define htole16(x)  (x)
    #define htole32(x)  (x)
    #define htole64(x)  (x)
    #define be16toh(x)  SWAP16(x)
    #define be32toh(x)  SWAP32(x)
    #define be64toh(x)  SWAP64(x)
    #define htobe16(x)  SWAP16(x)
    #define htobe32(x)  SWAP32(x)
    #define htobe64(x)  SWAP64(x)
#else
    #define le16toh(x)  SWAP16(x)
    #define le32toh(x)  SWAP32(x)
    #define le64toh(x)  SWAP64(x)
    #define htole16(x)  SWAP16(x)
    #define htole32(x)  SWAP32(x)
    #define htole64(x)  SWAP64(x)
    #define be16toh(x)  (x)
    #define be32toh(x)  (x)
    #define be64toh(x)  (x)
    #define htobe16(x)  (x)
    #define htobe32(x)  (x)
    #define htobe64(x)  (x)
#endif

// Common constants
#define BITS_PER_BYTE   8
#define BITS_PER_LONG   64
#define BYTES_PER_LONG  8

// Page size constants
#define PAGE_SHIFT      12
#define PAGE_SIZE       (1UL << PAGE_SHIFT)
#define PAGE_MASK       (~(PAGE_SIZE - 1))

// Maximum values
#define UINT8_MAX       0xFF
#define UINT16_MAX      0xFFFF
#define UINT32_MAX      0xFFFFFFFF
#define UINT64_MAX      0xFFFFFFFFFFFFFFFFULL
#define INT8_MAX        0x7F
#define INT16_MAX       0x7FFF
#define INT32_MAX       0x7FFFFFFF
#define INT64_MAX       0x7FFFFFFFFFFFFFFFLL

// Minimum values
#define INT8_MIN        (-INT8_MAX - 1)
#define INT16_MIN       (-INT16_MAX - 1)
#define INT32_MIN       (-INT32_MAX - 1)
#define INT64_MIN       (-INT64_MAX - 1)

// Size constants
#define SIZE_MAX        UINT64_MAX
#define SSIZE_MAX       INT64_MAX

// Common bit manipulation macros
#define BIT(n)          (1UL << (n))
#define BIT_MASK(nr)    (1UL << ((nr) % BITS_PER_LONG))
#define BIT_WORD(nr)    ((nr) / BITS_PER_LONG)
#define BITS_TO_LONGS(nr) DIV_ROUND_UP(nr, BITS_PER_LONG)

// Division with round up
#define DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))

// Alignment macros
#define IS_ALIGNED(x, a)    (((x) & ((typeof(x))(a) - 1)) == 0)
#define ALIGN_MASK(x, mask) (((x) + (mask)) & ~(mask))

// Container of macro - get structure from member pointer
#define container_of(ptr, type, member) \
    ((type *)((char *)(ptr) - offsetof(type, member)))

// Offset of macro - get offset of member in structure
#define offsetof(type, member) \
    ((size_t)&((type *)0)->member)

// Result type for kernel functions
typedef enum {
    KERN_SUCCESS    = 0,        /**< Operation successful */
    KERN_ERROR      = -1,       /**< Generic error */
    KERN_NOMEM      = -2,       /**< Out of memory */
    KERN_INVALID    = -3,       /**< Invalid parameter */
    KERN_NOTFOUND   = -4,       /**< Resource not found */
    KERN_EXISTS     = -5,       /**< Resource already exists */
    KERN_BUSY       = -6,       /**< Resource busy */
    KERN_TIMEOUT    = -7,       /**< Operation timed out */
    KERN_IO         = -8,       /**< I/O error */
    KERN_PERM       = -9,       /**< Permission denied */
    KERN_INTR       = -10       /**< Operation interrupted */
} kern_result_t;

// Generic linked list structure
struct list_head {
    struct list_head *next;     /**< Next element in list */
    struct list_head *prev;     /**< Previous element in list */
};

// Hash table entry structure
struct hash_node {
    struct hash_node *next;     /**< Next node in hash bucket */
    uint32_t          hash;     /**< Hash value */
};

// Reference counting structure
struct kref {
    uint32_t count;             /**< Reference count */
};

// Atomic operations structure
typedef struct {
    volatile int32_t counter;   /**< Atomic counter value */
} atomic_t;

typedef struct {
    volatile int64_t counter;   /**< Atomic 64-bit counter value */
} atomic64_t;

// Spinlock structure
typedef struct {
    volatile uint32_t lock;     /**< Lock value */
    uint32_t          cpu;      /**< CPU holding the lock */
} spinlock_t;

// Mutex structure
typedef struct {
    volatile uint32_t lock;     /**< Lock value */
    pid_t             owner;    /**< Process owning the mutex */
    struct list_head  waiters; /**< List of waiting processes */
} mutex_t;

// Memory region structure
struct memory_region {
    uint64_t    start;          /**< Start address */
    uint64_t    end;            /**< End address */
    size_t      size;           /**< Size of the region */
    uint32_t    type;           /**< Memory type */
    uint32_t    flags;          /**< Region flags */
    const char *name;           /**< Region name */
};

// Color structure for gradient UI
struct color {
    uint8_t r;                  /**< Red component (0-255) */
    uint8_t g;                  /**< Green component (0-255) */
    uint8_t b;                  /**< Blue component (0-255) */
    uint8_t a;                  /**< Alpha component (0-255) */
};

// Point structure for UI coordinates
struct point {
    int32_t x;                  /**< X coordinate */
    int32_t y;                  /**< Y coordinate */
};

// Rectangle structure for UI geometry
struct rect {
    int32_t x;                  /**< X coordinate */
    int32_t y;                  /**< Y coordinate */
    int32_t width;              /**< Width */
    int32_t height;             /**< Height */
};

#endif /* __TYPES_H__ */ 