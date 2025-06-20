/*
 * FG-OS Thread Management Implementation
 * Phase 6: Process Management System
 * 
 * Developed by: Faiz Nasir
 * Company: FGCompany Official
 * 
 * Thread creation, destruction, and context switching functionality.
 */

#include "scheduler.h"
#include "../include/kernel.h"
#include "../include/panic.h"
#include "../mm/memory.h"
#include "../arch/x86_64/arch.h"

// Global thread management variables
static struct thread *thread_list = NULL;     // Head of thread list
static struct thread *current_thread = NULL;  // Currently running thread
static uint32_t next_tid = 1;                // Next available TID
static uint32_t thread_count = 0;            // Total number of threads
static spinlock_t thread_lock = {0};         // Thread list lock

// Thread statistics
static struct {
    uint64_t threads_created;
    uint64_t threads_destroyed;
    uint64_t context_switches;
    uint64_t total_cpu_time;
} thread_stats = {0};

// Forward declarations
static int allocate_thread_stack(struct thread *thread);
static void cleanup_thread_stack(struct thread *thread);
static void init_thread_context(struct thread *thread, void (*entry_point)(void*), void *arg);

/**
 * @brief Initialize the thread management subsystem
 * 
 * @return 0 on success, negative error code on failure
 */
int thread_init(void) {
    KINFO("Initializing thread management subsystem...");
    
    // Initialize thread list
    thread_list = NULL;
    current_thread = NULL;
    next_tid = 1;
    thread_count = 0;
    
    // Initialize locks
    thread_lock.lock = 0;
    
    // Reset statistics
    memset(&thread_stats, 0, sizeof(thread_stats));
    
    KINFO("Thread management subsystem initialized successfully");
    return KERN_SUCCESS;
}

/**
 * @brief Create a new thread
 * 
 * @param pid Parent process ID
 * @param entry_point Thread entry point function
 * @param arg Argument to pass to entry point
 * @return Pointer to new thread on success, NULL on failure
 */
struct thread* create_thread(uint32_t pid, void (*entry_point)(void*), void *arg) {
    if (!entry_point) {
        KERROR("Thread entry point cannot be NULL");
        return NULL;
    }
    
    // Find parent process
    struct process *parent_proc = get_process(pid);
    if (!parent_proc) {
        KERROR("Parent process with PID %u not found", pid);
        return NULL;
    }
    
    // Allocate memory for thread structure
    struct thread *thread = (struct thread*)kmalloc(sizeof(struct thread));
    if (!thread) {
        KERROR("Failed to allocate memory for thread structure");
        return NULL;
    }
    
    // Initialize thread structure
    memset(thread, 0, sizeof(struct thread));
    
    // Set basic thread information
    thread->tid = next_tid++;
    thread->pid = pid;
    thread->state = THREAD_STATE_NEW;
    thread->process = parent_proc;
    
    // Set default priority and timing
    thread->priority = parent_proc->priority;
    thread->time_slice = TIME_SLICE_DEFAULT;
    thread->remaining_time = thread->time_slice;
    thread->sleep_until = 0;
    
    // Allocate thread stack
    if (allocate_thread_stack(thread) != KERN_SUCCESS) {
        KERROR("Failed to allocate thread stack");
        kfree(thread);
        return NULL;
    }
    
    // Initialize thread context
    init_thread_context(thread, entry_point, arg);
    
    // Initialize synchronization
    thread->wait_queue = NULL;
    thread->mutex_list = NULL;
    
    // Set thread relationships
    thread->next = NULL;
    thread->sched_next = NULL;
    
    // Add to parent process thread list
    if (parent_proc->threads == NULL) {
        parent_proc->threads = thread;
        parent_proc->main_thread = thread; // First thread is main thread
    } else {
        thread->next = parent_proc->threads;
        parent_proc->threads = thread;
    }
    parent_proc->thread_count++;
    
    // Add to global thread list
    if (thread_list == NULL) {
        thread_list = thread;
    } else {
        thread->sched_next = thread_list;
        thread_list = thread;
    }
    
    thread_count++;
    thread_stats.threads_created++;
    
    // Set thread state to ready
    thread->state = THREAD_STATE_READY;
    
    KINFO("Created thread TID %u in process PID %u", thread->tid, pid);
    return thread;
}

/**
 * @brief Destroy a thread and clean up its resources
 * 
 * @param tid Thread ID to destroy
 * @return 0 on success, negative error code on failure
 */
int destroy_thread(uint32_t tid) {
    struct thread *thread = get_thread(tid);
    if (!thread) {
        KERROR("Thread with TID %u not found", tid);
        return KERN_NOTFOUND;
    }
    
    // Cannot destroy currently running thread without proper scheduling
    if (thread == current_thread) {
        KERROR("Cannot destroy currently running thread");
        return KERN_BUSY;
    }
    
    KINFO("Destroying thread TID %u", tid);
    
    // Set thread state to terminated
    thread->state = THREAD_STATE_TERMINATED;
    
    // Clean up stack
    cleanup_thread_stack(thread);
    
    // Remove from parent process thread list
    if (thread->process) {
        struct thread **curr = &thread->process->threads;
        while (*curr) {
            if (*curr == thread) {
                *curr = thread->next;
                break;
            }
            curr = &(*curr)->next;
        }
        
        // Update main thread if necessary
        if (thread->process->main_thread == thread) {
            thread->process->main_thread = thread->process->threads;
        }
        
        thread->process->thread_count--;
    }
    
    // Remove from global thread list
    if (thread_list == thread) {
        thread_list = thread->sched_next;
    } else {
        struct thread *curr = thread_list;
        while (curr && curr->sched_next != thread) {
            curr = curr->sched_next;
        }
        if (curr) {
            curr->sched_next = thread->sched_next;
        }
    }
    
    // Free the thread structure
    kfree(thread);
    
    thread_count--;
    thread_stats.threads_destroyed++;
    
    KINFO("Thread TID %u destroyed successfully", tid);
    return KERN_SUCCESS;
}

/**
 * @brief Get thread by TID
 * 
 * @param tid Thread ID to find
 * @return Pointer to thread on success, NULL if not found
 */
struct thread* get_thread(uint32_t tid) {
    struct thread *thread = thread_list;
    
    while (thread) {
        if (thread->tid == tid) {
            return thread;
        }
        thread = thread->sched_next;
    }
    
    return NULL;
}

/**
 * @brief Get currently running thread
 * 
 * @return Pointer to current thread, NULL if none
 */
struct thread* get_current_thread(void) {
    return current_thread;
}

/**
 * @brief Set the currently running thread
 * 
 * @param thread Pointer to thread to set as current
 */
void set_current_thread(struct thread *thread) {
    current_thread = thread;
    if (thread) {
        thread->state = THREAD_STATE_RUNNING;
    }
}

/**
 * @brief Set thread priority
 * 
 * @param tid Thread ID
 * @param priority New priority level (0-30)
 * @return 0 on success, negative error code on failure
 */
int set_thread_priority(uint32_t tid, uint8_t priority) {
    if (priority >= PRIORITY_LEVELS) {
        return KERN_INVALID;
    }
    
    struct thread *thread = get_thread(tid);
    if (!thread) {
        return KERN_NOTFOUND;
    }
    
    thread->priority = priority;
    KINFO("Set thread %u priority to %u", tid, priority);
    return KERN_SUCCESS;
}

/**
 * @brief Get thread priority
 * 
 * @param tid Thread ID
 * @return Priority level, or PRIORITY_LEVELS on error
 */
uint8_t get_thread_priority(uint32_t tid) {
    struct thread *thread = get_thread(tid);
    if (!thread) {
        return PRIORITY_LEVELS; // Invalid priority indicates error
    }
    
    return thread->priority;
}

/**
 * @brief Make thread sleep for specified time
 * 
 * @param milliseconds Sleep duration in milliseconds
 */
void sleep(uint64_t milliseconds) {
    if (!current_thread) {
        return;
    }
    
    current_thread->sleep_until = get_system_time() + milliseconds;
    current_thread->state = THREAD_STATE_SLEEPING;
    
    // Trigger scheduler to switch to another thread
    yield();
}

/**
 * @brief Wake up a sleeping thread
 * 
 * @param thread Thread to wake up
 */
void wakeup(struct thread *thread) {
    if (!thread) {
        return;
    }
    
    if (thread->state == THREAD_STATE_SLEEPING) {
        thread->state = THREAD_STATE_READY;
        thread->sleep_until = 0;
        KINFO("Woke up thread TID %u", thread->tid);
    }
}

/**
 * @brief Print list of threads for a process
 * 
 * @param pid Process ID (0 for all threads)
 */
void print_thread_list(uint32_t pid) {
    printf("\n");
    if (pid == 0) {
        printf("╔══════════════════════════════════════════════════════════════╗\n");
        printf("║                         THREAD LIST                         ║\n");
    } else {
        printf("╔══════════════════════════════════════════════════════════════╗\n");
        printf("║                    THREADS FOR PID %3u                      ║\n", pid);
    }
    printf("╠═════╤═════╤═════════════════╤══════════╤═══════════╤══════════╣\n");
    printf("║ TID │ PID │ STATE           │ PRIORITY │ TIME SLICE│ SLEEP    ║\n");
    printf("╠═════╪═════╪═════════════════╪══════════╪═══════════╪══════════╣\n");
    
    struct thread *thread = thread_list;
    int count = 0;
    
    while (thread && count < 20) { // Limit display to 20 threads
        // Filter by PID if specified
        if (pid != 0 && thread->pid != pid) {
            thread = thread->sched_next;
            continue;
        }
        
        const char *state_str;
        switch (thread->state) {
            case THREAD_STATE_NEW:        state_str = "NEW            "; break;
            case THREAD_STATE_READY:      state_str = "READY          "; break;
            case THREAD_STATE_RUNNING:    state_str = "RUNNING        "; break;
            case THREAD_STATE_BLOCKED:    state_str = "BLOCKED        "; break;
            case THREAD_STATE_SLEEPING:   state_str = "SLEEPING       "; break;
            case THREAD_STATE_TERMINATED: state_str = "TERMINATED     "; break;
            default:                      state_str = "UNKNOWN        "; break;
        }
        
        printf("║ %3u │ %3u │ %s │ %8u  │ %8u  │ %7lu  ║\n",
               thread->tid, thread->pid, state_str, thread->priority, 
               thread->remaining_time, thread->sleep_until);
        
        thread = thread->sched_next;
        count++;
    }
    
    if (thread != NULL) {
        printf("║     │     │ ... (more)      │          │           │          ║\n");
    }
    
    printf("╚═════╧═════╧═════════════════╧══════════╧═══════════╧══════════╝\n");
    printf("Total Threads: %u | Created: %lu | Destroyed: %lu\n",
           thread_count, thread_stats.threads_created, thread_stats.threads_destroyed);
    printf("\n");
}

/**
 * @brief Get thread statistics
 * 
 * @return Pointer to thread statistics structure
 */
void* get_thread_stats(void) {
    static struct {
        uint32_t total_threads;
        uint32_t active_threads;
        uint32_t runnable_threads;
        uint64_t threads_created;
        uint64_t threads_destroyed;
        uint64_t context_switches;
    } stats;
    
    // Count runnable threads
    uint32_t runnable = 0;
    struct thread *thread = thread_list;
    while (thread) {
        if (thread->state == THREAD_STATE_READY || thread->state == THREAD_STATE_RUNNING) {
            runnable++;
        }
        thread = thread->sched_next;
    }
    
    stats.total_threads = thread_count;
    stats.active_threads = thread_count;
    stats.runnable_threads = runnable;
    stats.threads_created = thread_stats.threads_created;
    stats.threads_destroyed = thread_stats.threads_destroyed;
    stats.context_switches = thread_stats.context_switches;
    
    return &stats;
}

// Helper functions implementation

/**
 * @brief Allocate stack for a thread
 * 
 * @param thread Thread to allocate stack for
 * @return 0 on success, negative error code on failure
 */
static int allocate_thread_stack(struct thread *thread) {
    // Default stack size: 64KB
    const size_t stack_size = 64 * 1024;
    
    // Allocate stack memory
    void *stack = kmalloc(stack_size);
    if (!stack) {
        KERROR("Failed to allocate thread stack");
        return KERN_NOMEM;
    }
    
    // Set up stack information
    thread->stack_base = (uint64_t)stack;
    thread->stack_size = stack_size;
    thread->stack_pointer = thread->stack_base + stack_size - sizeof(uint64_t);
    
    // Initialize stack with canary values for debugging
    uint64_t *stack_words = (uint64_t*)stack;
    for (size_t i = 0; i < stack_size / sizeof(uint64_t); i++) {
        stack_words[i] = 0xDEADBEEFCAFEBABE;
    }
    
    return KERN_SUCCESS;
}

/**
 * @brief Clean up thread stack
 * 
 * @param thread Thread to clean up stack for
 */
static void cleanup_thread_stack(struct thread *thread) {
    if (thread->stack_base) {
        // Clear stack memory before freeing
        memset((void*)thread->stack_base, 0, thread->stack_size);
        kfree((void*)thread->stack_base);
        
        thread->stack_base = 0;
        thread->stack_size = 0;
        thread->stack_pointer = 0;
    }
}

/**
 * @brief Initialize thread CPU context
 * 
 * @param thread Thread to initialize context for
 * @param entry_point Thread entry point function
 * @param arg Argument to pass to entry point
 */
static void init_thread_context(struct thread *thread, void (*entry_point)(void*), void *arg) {
    // Initialize CPU context structure
    memset(&thread->context, 0, sizeof(struct cpu_state));
    
    // Set up initial CPU state
    thread->context.rip = (uint64_t)entry_point;  // Instruction pointer
    thread->context.rsp = thread->stack_pointer;  // Stack pointer
    thread->context.rbp = thread->stack_pointer;  // Base pointer
    thread->context.rdi = (uint64_t)arg;          // First argument register
    
    // Set up segment registers (user mode)
    thread->context.cs = 0x08;  // Kernel code segment for now
    thread->context.ds = 0x10;  // Kernel data segment
    thread->context.es = 0x10;
    thread->context.fs = 0x10;
    thread->context.gs = 0x10;
    thread->context.ss = 0x10;  // Stack segment
    
    // Set up flags (enable interrupts)
    thread->context.rflags = 0x202;  // IF flag set
    
    KINFO("Initialized thread context for TID %u", thread->tid);
} 