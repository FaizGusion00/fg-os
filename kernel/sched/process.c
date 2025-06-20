/*
 * FG-OS Process Management Implementation
 * Phase 6: Process Management System
 * 
 * Developed by: Faiz Nasir
 * Company: FGCompany Official
 * 
 * Process creation, destruction, and management functionality.
 */

#include "scheduler.h"
#include "../include/kernel.h"
#include "../include/panic.h"
#include "../mm/memory.h"
#include "../arch/x86_64/arch.h"
// #include <string.h>  // Using kernel string functions instead

// Global process management variables
static struct process *process_list = NULL;    // Head of process list
static struct process *current_process = NULL; // Currently running process
static uint32_t next_pid = 1;                 // Next available PID
static uint32_t process_count = 0;             // Total number of processes
static spinlock_t process_lock = {0};          // Process list lock

// Process statistics
static struct {
    uint64_t processes_created;
    uint64_t processes_destroyed;
    uint64_t context_switches;
    uint64_t total_cpu_time;
} process_stats = {0};

// Forward declarations
static void cleanup_process_memory(struct process *proc);
static int allocate_process_memory(struct process *proc);
static void init_process_context(struct process *proc);

/**
 * @brief Initialize the process management subsystem
 * 
 * @return 0 on success, negative error code on failure
 */
int process_init(void) {
    KINFO("Initializing process management subsystem...");
    
    // Initialize process list
    process_list = NULL;
    current_process = NULL;
    next_pid = 1;
    process_count = 0;
    
    // Initialize locks
    process_lock.lock = 0;
    
    // Reset statistics
    memset(&process_stats, 0, sizeof(process_stats));
    
    KINFO("Process management subsystem initialized successfully");
    return KERN_SUCCESS;
}

/**
 * @brief Create a new process
 * 
 * @param name Process name (max 63 characters)
 * @param parent_pid Parent process ID (0 for kernel processes)
 * @return Pointer to new process on success, NULL on failure
 */
struct process* create_process(const char *name, uint32_t parent_pid) {
    if (!name) {
        KERROR("Process name cannot be NULL");
        return NULL;
    }
    
    // Allocate memory for process structure
    struct process *proc = (struct process*)kmalloc(sizeof(struct process));
    if (!proc) {
        KERROR("Failed to allocate memory for process structure");
        return NULL;
    }
    
    // Initialize process structure
    memset(proc, 0, sizeof(struct process));
    
    // Set basic process information
    proc->pid = next_pid++;
    proc->ppid = parent_pid;
    strncpy(proc->name, name, sizeof(proc->name) - 1);
    proc->name[sizeof(proc->name) - 1] = '\0';
    proc->state = PROCESS_STATE_NEW;
    
    // Set default priority and timing
    proc->priority = PRIORITY_NORMAL;
    proc->nice = 0;
    proc->time_slice = TIME_SLICE_DEFAULT;
    proc->remaining_time = proc->time_slice;
    proc->creation_time = get_system_time();
    proc->cpu_time = 0;
    proc->last_scheduled = 0;
    
    // Initialize memory layout
    if (allocate_process_memory(proc) != KERN_SUCCESS) {
        KERROR("Failed to allocate process memory");
        kfree(proc);
        return NULL;
    }
    
    // Initialize process context
    init_process_context(proc);
    
    // Initialize thread management
    proc->main_thread = NULL;
    proc->threads = NULL;
    proc->thread_count = 0;
    
    // Initialize file and signal tables (placeholder)
    proc->file_table = NULL;
    proc->signal_handlers = NULL;
    
    // Set process relationships
    proc->parent = NULL;
    proc->children = NULL;
    proc->next = NULL;
    proc->prev = NULL;
    
    // Find and set parent process
    if (parent_pid != 0) {
        struct process *parent = get_process(parent_pid);
        if (parent) {
            proc->parent = parent;
            // Add to parent's children list (simplified implementation)
            if (parent->children == NULL) {
                parent->children = proc;
            }
        }
    }
    
    // Add to global process list
    // TODO: Add proper locking for SMP safety
    if (process_list == NULL) {
        process_list = proc;
    } else {
        proc->next = process_list;
        process_list->prev = proc;
        process_list = proc;
    }
    
    process_count++;
    process_stats.processes_created++;
    
    KINFO("Created process '%s' with PID %u", name, proc->pid);
    return proc;
}

/**
 * @brief Destroy a process and clean up its resources
 * 
 * @param pid Process ID to destroy
 * @return 0 on success, negative error code on failure
 */
int destroy_process(uint32_t pid) {
    struct process *proc = get_process(pid);
    if (!proc) {
        KERROR("Process with PID %u not found", pid);
        return KERN_NOTFOUND;
    }
    
    // Cannot destroy currently running process without proper scheduling
    if (proc == current_process) {
        KERROR("Cannot destroy currently running process");
        return KERN_BUSY;
    }
    
    KINFO("Destroying process '%s' (PID %u)", proc->name, proc->pid);
    
    // Set process state to terminated
    proc->state = PROCESS_STATE_TERMINATED;
    
    // Clean up all threads (simplified for now)
    // TODO: Implement proper thread cleanup
    proc->thread_count = 0;
    proc->threads = NULL;
    proc->main_thread = NULL;
    
    // Clean up memory
    cleanup_process_memory(proc);
    
    // Remove from process list
    if (proc->prev) {
        proc->prev->next = proc->next;
    } else {
        process_list = proc->next;
    }
    
    if (proc->next) {
        proc->next->prev = proc->prev;
    }
    
    // Update parent's children list (simplified)
    if (proc->parent && proc->parent->children == proc) {
        proc->parent->children = NULL; // Simplified - should handle multiple children
    }
    
    // Free the process structure
    kfree(proc);
    
    process_count--;
    process_stats.processes_destroyed++;
    
    KINFO("Process PID %u destroyed successfully", pid);
    return KERN_SUCCESS;
}

/**
 * @brief Get process by PID
 * 
 * @param pid Process ID to find
 * @return Pointer to process on success, NULL if not found
 */
struct process* get_process(uint32_t pid) {
    struct process *proc = process_list;
    
    while (proc) {
        if (proc->pid == pid) {
            return proc;
        }
        proc = proc->next;
    }
    
    return NULL;
}

/**
 * @brief Get currently running process
 * 
 * @return Pointer to current process, NULL if none
 */
struct process* get_current_process(void) {
    return current_process;
}

/**
 * @brief Set the currently running process
 * 
 * @param proc Pointer to process to set as current
 */
void set_current_process(struct process *proc) {
    current_process = proc;
    if (proc) {
        proc->state = PROCESS_STATE_RUNNING;
        proc->last_scheduled = get_system_time();
    }
}

/**
 * @brief Set process priority
 * 
 * @param pid Process ID
 * @param priority New priority level (0-30)
 * @return 0 on success, negative error code on failure
 */
int set_process_priority(uint32_t pid, uint8_t priority) {
    if (priority >= PRIORITY_LEVELS) {
        return KERN_INVALID;
    }
    
    struct process *proc = get_process(pid);
    if (!proc) {
        return KERN_NOTFOUND;
    }
    
    proc->priority = priority;
    KINFO("Set process %u priority to %u", pid, priority);
    return KERN_SUCCESS;
}

/**
 * @brief Get process priority
 * 
 * @param pid Process ID
 * @return Priority level, or PRIORITY_LEVELS on error
 */
uint8_t get_process_priority(uint32_t pid) {
    struct process *proc = get_process(pid);
    if (!proc) {
        return PRIORITY_LEVELS; // Invalid priority indicates error
    }
    
    return proc->priority;
}

/**
 * @brief Suspend a process
 * 
 * @param pid Process ID to suspend
 * @return 0 on success, negative error code on failure
 */
int suspend_process(uint32_t pid) {
    struct process *proc = get_process(pid);
    if (!proc) {
        return KERN_NOTFOUND;
    }
    
    if (proc->state == PROCESS_STATE_RUNNING || proc->state == PROCESS_STATE_READY) {
        proc->state = PROCESS_STATE_STOPPED;
        KINFO("Suspended process %u", pid);
        return KERN_SUCCESS;
    }
    
    return KERN_ERROR;
}

/**
 * @brief Resume a suspended process
 * 
 * @param pid Process ID to resume
 * @return 0 on success, negative error code on failure
 */
int resume_process(uint32_t pid) {
    struct process *proc = get_process(pid);
    if (!proc) {
        return KERN_NOTFOUND;
    }
    
    if (proc->state == PROCESS_STATE_STOPPED) {
        proc->state = PROCESS_STATE_READY;
        KINFO("Resumed process %u", pid);
        return KERN_SUCCESS;
    }
    
    return KERN_ERROR;
}

/**
 * @brief Send signal to process (placeholder implementation)
 * 
 * @param pid Process ID
 * @param signal Signal number
 * @return 0 on success, negative error code on failure
 */
int kill_process(uint32_t pid, int signal) {
    struct process *proc = get_process(pid);
    if (!proc) {
        return KERN_NOTFOUND;
    }
    
    // Simplified signal handling - just terminate for now
    if (signal == 9) { // SIGKILL
        return destroy_process(pid);
    }
    
    KINFO("Sent signal %d to process %u", signal, pid);
    return KERN_SUCCESS;
}

/**
 * @brief Wait for process termination (placeholder)
 * 
 * @param pid Process ID to wait for
 * @param status Pointer to store exit status
 * @return 0 on success, negative error code on failure
 */
int wait_process(uint32_t pid, int *status) {
    struct process *proc = get_process(pid);
    if (!proc) {
        return KERN_NOTFOUND;
    }
    
    // Simplified implementation - just check if process is terminated
    if (proc->state == PROCESS_STATE_TERMINATED || proc->state == PROCESS_STATE_ZOMBIE) {
        if (status) {
            *status = 0; // Success exit code
        }
        return KERN_SUCCESS;
    }
    
    return KERN_BUSY; // Process still running
}

/**
 * @brief Print list of all processes
 */
void print_process_list(void) {
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                        PROCESS LIST                         ║\n");
    printf("╠═════╤═════════════════════╤══════════╤═══════════╤══════════╣\n");
    printf("║ PID │ NAME                │ STATE    │ PRIORITY  │ CPU TIME ║\n");
    printf("╠═════╪═════════════════════╪══════════╪═══════════╪══════════╣\n");
    
    struct process *proc = process_list;
    int count = 0;
    
    while (proc && count < 20) { // Limit display to 20 processes
        const char *state_str;
        switch (proc->state) {
            case PROCESS_STATE_NEW:        state_str = "NEW     "; break;
            case PROCESS_STATE_READY:      state_str = "READY   "; break;
            case PROCESS_STATE_RUNNING:    state_str = "RUNNING "; break;
            case PROCESS_STATE_WAITING:    state_str = "WAITING "; break;
            case PROCESS_STATE_STOPPED:    state_str = "STOPPED "; break;
            case PROCESS_STATE_ZOMBIE:     state_str = "ZOMBIE  "; break;
            case PROCESS_STATE_TERMINATED: state_str = "DEAD    "; break;
            default:                       state_str = "UNKNOWN "; break;
        }
        
        printf("║ %3u │ %-19s │ %s │ %8u  │ %7lu  ║\n",
               proc->pid, proc->name, state_str, proc->priority, proc->cpu_time);
        
        proc = proc->next;
        count++;
    }
    
    if (proc != NULL) {
        printf("║     │ ... (%u more)      │          │           │          ║\n",
               process_count - count);
    }
    
    printf("╚═════╧═════════════════════╧══════════╧═══════════╧══════════╝\n");
    printf("Total Processes: %u | Created: %lu | Destroyed: %lu\n",
           process_count, process_stats.processes_created, process_stats.processes_destroyed);
    printf("\n");
}

/**
 * @brief Get process statistics
 * 
 * @return Pointer to process statistics structure
 */
void* get_process_stats(void) {
    static struct {
        uint32_t total_processes;
        uint32_t active_processes;
        uint64_t processes_created;
        uint64_t processes_destroyed;
        uint64_t context_switches;
    } stats;
    
    stats.total_processes = process_count;
    stats.active_processes = process_count; // Simplified
    stats.processes_created = process_stats.processes_created;
    stats.processes_destroyed = process_stats.processes_destroyed;
    stats.context_switches = process_stats.context_switches;
    
    return &stats;
}

// Helper functions implementation

/**
 * @brief Allocate memory regions for a process
 * 
 * @param proc Process to allocate memory for
 * @return 0 on success, negative error code on failure
 */
static int allocate_process_memory(struct process *proc) {
    // Simplified memory allocation
    // In a real implementation, this would set up page tables, virtual memory regions, etc.
    
    // Set up basic memory layout
    proc->heap_start = USER_BASE + 0x400000;  // 4MB offset for code/data
    proc->heap_end = proc->heap_start;
    proc->stack_start = USER_BASE + 0x40000000; // 1GB for stack region
    proc->stack_end = proc->stack_start + 0x100000; // 1MB stack initially
    
    // Allocate a page directory (placeholder)
    proc->page_directory = 0; // Will be set up by VMM later
    
    return KERN_SUCCESS;
}

/**
 * @brief Clean up process memory regions
 * 
 * @param proc Process to clean up memory for
 */
static void cleanup_process_memory(struct process *proc) {
    // Free page directory and mapped pages
    if (proc->page_directory) {
        // TODO: Free page directory and all mapped pages
        proc->page_directory = 0;
    }
    
    // Reset memory regions
    proc->heap_start = 0;
    proc->heap_end = 0;
    proc->stack_start = 0;
    proc->stack_end = 0;
}

/**
 * @brief Initialize process CPU context
 * 
 * @param proc Process to initialize context for
 */
static void init_process_context(struct process *proc) {
    // Initialize basic process context
    // This would typically set up initial CPU state for the process
    // For now, we'll just zero the context area
    
    // CPU context will be set up when the main thread is created
    UNUSED(proc);
}

/**
 * @brief Get current system time (placeholder)
 * 
 * @return Current system time in milliseconds
 */
uint64_t get_system_time(void) {
    // Placeholder implementation
    // In a real system, this would read from the system timer
    static uint64_t fake_time = 0;
    return ++fake_time;
} 