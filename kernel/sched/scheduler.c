/*
 * FG-OS Scheduler Implementation
 * Phase 6: Process Management System
 * 
 * Developed by: Faiz Nasir
 * Company: FGCompany Official
 * 
 * Thread scheduling, context switching, and load balancing functionality.
 */

#include "scheduler.h"
#include "../include/kernel.h"
#include "../include/panic.h"
#include "../mm/memory.h"
#include "../arch/x86_64/arch.h"

// Scheduler configuration
static uint8_t current_policy = SCHED_POLICY_ROUND_ROBIN;
static uint32_t time_quantum = TIME_SLICE_DEFAULT;
static bool scheduler_enabled = false;
static bool preemption_enabled = false;

// Scheduler queues (simple round-robin for now)
static struct thread *ready_queue = NULL;
static struct thread *sleeping_queue = NULL;
static spinlock_t sched_lock = {0};

// Scheduler statistics
static struct scheduler_stats stats = {0};

// Timer tick counter for scheduling
static uint64_t tick_counter = 0;
static uint64_t last_schedule_time = 0;

// Forward declarations
static struct thread* select_next_thread(void);
static void context_switch(struct thread *prev, struct thread *next);
static void add_to_ready_queue(struct thread *thread);
static struct thread* remove_from_ready_queue(void);
static void update_sleep_queue(void);
static void update_thread_statistics(struct thread *thread, uint64_t time_used);

/**
 * @brief Initialize the scheduler subsystem
 * 
 * @return 0 on success, negative error code on failure
 */
int scheduler_init(void) {
    KINFO("Initializing scheduler subsystem...");
    
    // Initialize scheduler state
    current_policy = SCHED_POLICY_ROUND_ROBIN;
    time_quantum = TIME_SLICE_DEFAULT;
    scheduler_enabled = false;
    preemption_enabled = false;
    
    // Initialize queues
    ready_queue = NULL;
    sleeping_queue = NULL;
    
    // Initialize locks
    sched_lock.lock = 0;
    
    // Reset statistics
    memset(&stats, 0, sizeof(struct scheduler_stats));
    
    // Reset counters
    tick_counter = 0;
    last_schedule_time = get_system_time();
    
    KINFO("Scheduler subsystem initialized successfully");
    return KERN_SUCCESS;
}

/**
 * @brief Enable the scheduler
 * 
 * @param enable_preemption Whether to enable preemptive scheduling
 */
void scheduler_enable(bool enable_preemption) {
    scheduler_enabled = true;
    preemption_enabled = enable_preemption;
    
    KINFO("Scheduler enabled (preemption: %s)", 
          enable_preemption ? "ON" : "OFF");
}

/**
 * @brief Disable the scheduler
 */
void scheduler_disable(void) {
    scheduler_enabled = false;
    preemption_enabled = false;
    
    KINFO("Scheduler disabled");
}

/**
 * @brief Main scheduling function - select and switch to next thread
 */
void schedule(void) {
    if (!scheduler_enabled) {
        return;
    }
    
    struct thread *current = get_current_thread();
    struct thread *next = select_next_thread();
    
    // No thread to schedule
    if (!next) {
        if (!current) {
            // No threads at all - halt CPU
            arch_halt();
        }
        return;
    }
    
    // Same thread - reset time slice and continue
    if (current == next) {
        if (current) {
            current->remaining_time = current->time_slice;
        }
        return;
    }
    
    // Update statistics
    stats.context_switches++;
    
    // Calculate CPU time used by current thread
    uint64_t current_time = get_system_time();
    if (current && last_schedule_time > 0) {
        uint64_t time_used = current_time - last_schedule_time;
        update_thread_statistics(current, time_used);
    }
    
    // Perform context switch
    context_switch(current, next);
    
    // Update scheduling time
    last_schedule_time = current_time;
    
    KDEBUG("Scheduled thread TID %u (was TID %u)", 
           next ? next->tid : 0, current ? current->tid : 0);
}

/**
 * @brief Yield CPU to next thread (voluntary context switch)
 */
void yield(void) {
    if (!scheduler_enabled) {
        return;
    }
    
    struct thread *current = get_current_thread();
    if (current) {
        // Reset time slice for current thread
        current->remaining_time = current->time_slice;
        
        // Add current thread back to ready queue if it's still runnable
        if (current->state == THREAD_STATE_RUNNING) {
            current->state = THREAD_STATE_READY;
            add_to_ready_queue(current);
        }
    }
    
    // Schedule next thread
    schedule();
}

/**
 * @brief Timer tick handler for preemptive scheduling
 */
void scheduler_tick(void) {
    if (!scheduler_enabled) {
        return;
    }
    
    tick_counter++;
    
    // Update sleeping threads
    update_sleep_queue();
    
    // Handle preemptive scheduling
    if (preemption_enabled) {
        struct thread *current = get_current_thread();
        if (current && current->remaining_time > 0) {
            current->remaining_time--;
            
            // Time slice expired - preempt
            if (current->remaining_time == 0) {
                current->state = THREAD_STATE_READY;
                add_to_ready_queue(current);
                schedule();
            }
        }
    }
}

/**
 * @brief Add thread to scheduler ready queue
 * 
 * @param thread Thread to add to ready queue
 */
void scheduler_add_thread(struct thread *thread) {
    if (!thread) {
        return;
    }
    
    thread->state = THREAD_STATE_READY;
    thread->remaining_time = thread->time_slice;
    add_to_ready_queue(thread);
    
    KDEBUG("Added thread TID %u to ready queue", thread->tid);
}

/**
 * @brief Remove thread from scheduler queues
 * 
 * @param thread Thread to remove
 */
void scheduler_remove_thread(struct thread *thread) {
    if (!thread) {
        return;
    }
    
    // Remove from ready queue
    if (ready_queue == thread) {
        ready_queue = thread->sched_next;
    } else {
        struct thread *curr = ready_queue;
        while (curr && curr->sched_next != thread) {
            curr = curr->sched_next;
        }
        if (curr) {
            curr->sched_next = thread->sched_next;
        }
    }
    
    // Remove from sleeping queue
    if (sleeping_queue == thread) {
        sleeping_queue = thread->sched_next;
    } else {
        struct thread *curr = sleeping_queue;
        while (curr && curr->sched_next != thread) {
            curr = curr->sched_next;
        }
        if (curr) {
            curr->sched_next = thread->sched_next;
        }
    }
    
    thread->sched_next = NULL;
    
    KDEBUG("Removed thread TID %u from scheduler queues", thread->tid);
}

/**
 * @brief Set scheduling policy
 * 
 * @param policy New scheduling policy
 * @return 0 on success, negative error code on failure
 */
int scheduler_set_policy(uint8_t policy) {
    switch (policy) {
        case SCHED_POLICY_ROUND_ROBIN:
        case SCHED_POLICY_PRIORITY:
        case SCHED_POLICY_CFS:
        case SCHED_POLICY_REALTIME:
            current_policy = policy;
            KINFO("Set scheduling policy to %u", policy);
            return KERN_SUCCESS;
        default:
            return KERN_INVALID;
    }
}

/**
 * @brief Get current scheduling policy
 * 
 * @return Current scheduling policy
 */
uint8_t scheduler_get_policy(void) {
    return current_policy;
}

/**
 * @brief Get scheduler statistics
 * 
 * @return Pointer to scheduler statistics
 */
struct scheduler_stats* get_scheduler_stats(void) {
    // Update current statistics
    stats.active_processes = 0;  // Will be updated by process manager
    stats.active_threads = 0;    // Will be updated by thread manager
    stats.runnable_threads = 0;
    
    // Count runnable threads
    struct thread *thread = ready_queue;
    while (thread) {
        stats.runnable_threads++;
        thread = thread->sched_next;
    }
    
    return &stats;
}

/**
 * @brief Print scheduler status and statistics
 */
void print_scheduler_status(void) {
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                      SCHEDULER STATUS                       ║\n");
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║ Status: %-8s │ Policy: %-12s │ Quantum: %4u ms ║\n",
           scheduler_enabled ? "ENABLED" : "DISABLED",
           current_policy == SCHED_POLICY_ROUND_ROBIN ? "ROUND_ROBIN" :
           current_policy == SCHED_POLICY_PRIORITY ? "PRIORITY" :
           current_policy == SCHED_POLICY_CFS ? "CFS" :
           current_policy == SCHED_POLICY_REALTIME ? "REALTIME" : "UNKNOWN",
           time_quantum);
    printf("║ Preemption: %-4s │ Tick Counter: %-12lu │ Switches: %6lu ║\n",
           preemption_enabled ? "ON" : "OFF", tick_counter, stats.context_switches);
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║ Ready Threads: %3u │ Sleeping Threads: %3u │ Total CPU: %6lu ║\n",
           stats.runnable_threads, 0, stats.total_cpu_time); // TODO: count sleeping
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    printf("\n");
}

// Helper functions implementation

/**
 * @brief Select next thread to run based on scheduling policy
 * 
 * @return Pointer to next thread, or NULL if none available
 */
static struct thread* select_next_thread(void) {
    switch (current_policy) {
        case SCHED_POLICY_ROUND_ROBIN:
        default:
            return remove_from_ready_queue();
        
        case SCHED_POLICY_PRIORITY:
            // TODO: Implement priority-based selection
            return remove_from_ready_queue();
        
        case SCHED_POLICY_CFS:
            // TODO: Implement CFS algorithm
            return remove_from_ready_queue();
        
        case SCHED_POLICY_REALTIME:
            // TODO: Implement real-time scheduling
            return remove_from_ready_queue();
    }
}

/**
 * @brief Perform context switch between threads
 * 
 * @param prev Previous thread (can be NULL)
 * @param next Next thread (must not be NULL)
 */
static void context_switch(struct thread *prev, struct thread *next) {
    if (!next) {
        KERROR("Cannot switch to NULL thread");
        return;
    }
    
    // Save current thread context (if any)
    if (prev && prev->state == THREAD_STATE_RUNNING) {
        // TODO: Save CPU registers to prev->context
        // This would typically be done in assembly code
        prev->state = THREAD_STATE_READY;
    }
    
    // Set new current thread
    set_current_thread(next);
    next->state = THREAD_STATE_RUNNING;
    next->remaining_time = next->time_slice;
    
    // Update process context if necessary
    if (!prev || prev->process != next->process) {
        set_current_process(next->process);
        
        // Switch page directory (memory context)
        if (next->process && next->process->page_directory) {
            // TODO: Switch to process page directory
            // arch_switch_page_directory(next->process->page_directory);
        }
    }
    
    // Restore new thread context
    // TODO: Restore CPU registers from next->context
    // This would typically be done in assembly code
    
    KDEBUG("Context switched to thread TID %u", next->tid);
}

/**
 * @brief Add thread to ready queue
 * 
 * @param thread Thread to add
 */
static void add_to_ready_queue(struct thread *thread) {
    if (!thread) {
        return;
    }
    
    thread->sched_next = ready_queue;
    ready_queue = thread;
    thread->state = THREAD_STATE_READY;
}

/**
 * @brief Remove and return next thread from ready queue
 * 
 * @return Next thread from ready queue, or NULL if empty
 */
static struct thread* remove_from_ready_queue(void) {
    if (!ready_queue) {
        return NULL;
    }
    
    struct thread *thread = ready_queue;
    ready_queue = thread->sched_next;
    thread->sched_next = NULL;
    
    return thread;
}

/**
 * @brief Update sleeping threads and wake up those whose time has expired
 */
static void update_sleep_queue(void) {
    uint64_t current_time = get_system_time();
    struct thread *thread = sleeping_queue;
    struct thread *prev = NULL;
    
    while (thread) {
        struct thread *next = thread->sched_next;
        
        // Check if sleep time has expired
        if (thread->sleep_until <= current_time) {
            // Remove from sleeping queue
            if (prev) {
                prev->sched_next = next;
            } else {
                sleeping_queue = next;
            }
            
            // Wake up thread
            wakeup(thread);
            add_to_ready_queue(thread);
        } else {
            prev = thread;
        }
        
        thread = next;
    }
}

/**
 * @brief Update thread CPU time statistics
 * 
 * @param thread Thread to update
 * @param time_used CPU time used in current time slice
 */
static void update_thread_statistics(struct thread *thread, uint64_t time_used) {
    if (!thread || !thread->process) {
        return;
    }
    
    // Update thread statistics
    // TODO: Add thread-specific CPU time tracking
    
    // Update process statistics
    thread->process->cpu_time += time_used;
    
    // Update global statistics
    stats.total_cpu_time += time_used;
} 