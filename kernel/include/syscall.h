/*
 * FG-OS System Call Interface
 * Phase 4: Kernel Architecture Design
 * 
 * Developed by: Faiz Nasir
 * Company: FGCompany Official
 * 
 * System call definitions and interface architecture.
 */

#ifndef SYSCALL_H
#define SYSCALL_H

#include <types.h>

// System Call Numbers
#define SYS_EXIT            0   // Process termination
#define SYS_FORK            1   // Create child process
#define SYS_READ            2   // Read from file descriptor
#define SYS_WRITE           3   // Write to file descriptor
#define SYS_OPEN            4   // Open file
#define SYS_CLOSE           5   // Close file descriptor
#define SYS_WAIT            6   // Wait for child process
#define SYS_EXEC            7   // Execute program
#define SYS_GETPID          8   // Get process ID
#define SYS_GETPPID         9   // Get parent process ID
#define SYS_SBRK            10  // Change data segment size
#define SYS_SLEEP           11  // Sleep for specified time
#define SYS_YIELD           12  // Yield CPU to other processes

// Memory Management System Calls
#define SYS_MMAP            20  // Memory map
#define SYS_MUNMAP          21  // Memory unmap
#define SYS_MPROTECT        22  // Change memory protection
#define SYS_MALLOC          23  // Allocate memory
#define SYS_FREE            24  // Free memory

// File System System Calls
#define SYS_MKDIR           30  // Create directory
#define SYS_RMDIR           31  // Remove directory
#define SYS_UNLINK          32  // Remove file
#define SYS_LINK            33  // Create hard link
#define SYS_SYMLINK         34  // Create symbolic link
#define SYS_READLINK        35  // Read symbolic link
#define SYS_STAT            36  // Get file status
#define SYS_FSTAT           37  // Get file status by descriptor
#define SYS_LSEEK           38  // Change file position
#define SYS_SYNC            39  // Synchronize file system

// Process Control System Calls
#define SYS_KILL            40  // Send signal to process
#define SYS_SIGNAL          41  // Set signal handler
#define SYS_ALARM           42  // Set alarm
#define SYS_PAUSE           43  // Wait for signal
#define SYS_SETPRIORITY     44  // Set process priority
#define SYS_GETPRIORITY     45  // Get process priority

// Thread Management System Calls
#define SYS_THREAD_CREATE   50  // Create thread
#define SYS_THREAD_EXIT     51  // Exit thread
#define SYS_THREAD_JOIN     52  // Wait for thread
#define SYS_THREAD_DETACH   53  // Detach thread
#define SYS_MUTEX_INIT      54  // Initialize mutex
#define SYS_MUTEX_LOCK      55  // Lock mutex
#define SYS_MUTEX_UNLOCK    56  // Unlock mutex
#define SYS_MUTEX_DESTROY   57  // Destroy mutex

// Inter-Process Communication
#define SYS_PIPE            60  // Create pipe
#define SYS_SHM_GET         61  // Get shared memory
#define SYS_SHM_ATTACH      62  // Attach shared memory
#define SYS_SHM_DETACH      63  // Detach shared memory
#define SYS_SEM_GET         64  // Get semaphore
#define SYS_SEM_OP          65  // Semaphore operation
#define SYS_MSG_GET         66  // Get message queue
#define SYS_MSG_SEND        67  // Send message
#define SYS_MSG_RECV        68  // Receive message

// System Information
#define SYS_GETTIME         70  // Get system time
#define SYS_SETTIME         71  // Set system time
#define SYS_UNAME           72  // Get system information
#define SYS_GETUID          73  // Get user ID
#define SYS_SETUID          74  // Set user ID
#define SYS_GETGID          75  // Get group ID
#define SYS_SETGID          76  // Set group ID

// FG-OS Specific System Calls
#define SYS_FG_INFO         100 // Get FG-OS system information
#define SYS_FG_DEBUG        101 // Debug system call
#define SYS_FG_PERF         102 // Performance monitoring
#define SYS_FG_COMPAT       103 // Compatibility layer control

#define SYSCALL_MAX         103 // Maximum system call number

// System Call Return Codes
#define SYSCALL_SUCCESS     0
#define SYSCALL_ERROR      -1
#define SYSCALL_INVALID    -2
#define SYSCALL_DENIED     -3
#define SYSCALL_TIMEOUT    -4

// System Call Handler Function Type
typedef int64_t (*syscall_handler_t)(uint64_t arg1, uint64_t arg2, 
                                    uint64_t arg3, uint64_t arg4,
                                    uint64_t arg5, uint64_t arg6);

// System Call Table Entry
struct syscall_entry {
    syscall_handler_t handler;  // Handler function
    const char *name;           // System call name
    uint8_t arg_count;          // Number of arguments
    uint8_t flags;              // System call flags
};

// System Call Flags
#define SYSCALL_FLAG_INTERRUPTIBLE  (1 << 0)  // Can be interrupted
#define SYSCALL_FLAG_RESTARTABLE    (1 << 1)  // Can be restarted
#define SYSCALL_FLAG_PRIVILEGED     (1 << 2)  // Requires privilege

// System Call Interface Functions
int syscall_init(void);
int64_t syscall_dispatch(uint64_t syscall_num, uint64_t arg1, uint64_t arg2,
                        uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6);
int register_syscall(uint64_t num, syscall_handler_t handler, 
                     const char *name, uint8_t arg_count, uint8_t flags);
void unregister_syscall(uint64_t num);

// System Call Handler Prototypes
int64_t sys_exit(uint64_t status);
int64_t sys_fork(void);
int64_t sys_read(uint64_t fd, uint64_t buffer, uint64_t count);
int64_t sys_write(uint64_t fd, uint64_t buffer, uint64_t count);
int64_t sys_open(uint64_t path, uint64_t flags, uint64_t mode);
int64_t sys_close(uint64_t fd);
int64_t sys_getpid(void);
int64_t sys_sleep(uint64_t milliseconds);
int64_t sys_yield(void);

// Memory Management Handlers
int64_t sys_mmap(uint64_t addr, uint64_t length, uint64_t prot, 
                uint64_t flags, uint64_t fd, uint64_t offset);
int64_t sys_munmap(uint64_t addr, uint64_t length);
int64_t sys_mprotect(uint64_t addr, uint64_t length, uint64_t prot);

// FG-OS Specific Handlers
int64_t sys_fg_info(uint64_t info_type, uint64_t buffer, uint64_t size);
int64_t sys_fg_debug(uint64_t debug_type, uint64_t param1, uint64_t param2);

// System Call Macros for User Programs
#define SYSCALL0(num) \
    syscall_dispatch(num, 0, 0, 0, 0, 0, 0)

#define SYSCALL1(num, arg1) \
    syscall_dispatch(num, arg1, 0, 0, 0, 0, 0)

#define SYSCALL2(num, arg1, arg2) \
    syscall_dispatch(num, arg1, arg2, 0, 0, 0, 0)

#define SYSCALL3(num, arg1, arg2, arg3) \
    syscall_dispatch(num, arg1, arg2, arg3, 0, 0, 0)

#define SYSCALL4(num, arg1, arg2, arg3, arg4) \
    syscall_dispatch(num, arg1, arg2, arg3, arg4, 0, 0)

#define SYSCALL5(num, arg1, arg2, arg3, arg4, arg5) \
    syscall_dispatch(num, arg1, arg2, arg3, arg4, arg5, 0)

#define SYSCALL6(num, arg1, arg2, arg3, arg4, arg5, arg6) \
    syscall_dispatch(num, arg1, arg2, arg3, arg4, arg5, arg6)

#endif // SYSCALL_H 