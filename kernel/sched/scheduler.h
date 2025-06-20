/*
 * FG-OS Process Scheduler Header
 * Phase 4: Kernel Architecture Design
 * 
 * Developed by: Faiz Nasir
 * Company: FGCompany Official
 * 
 * Process and thread scheduling architecture.
 */

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <types.h>
#include "../arch/x86_64/arch.h"

// Process States
typedef enum {
    PROCESS_STATE_NEW = 0,      // Process being created
    PROCESS_STATE_READY,        // Ready to run
    PROCESS_STATE_RUNNING,      // Currently executing
    PROCESS_STATE_WAITING,      // Waiting for I/O or event
    PROCESS_STATE_STOPPED,      // Stopped/suspended
    PROCESS_STATE_ZOMBIE,       // Terminated but not cleaned up
    PROCESS_STATE_TERMINATED    // Completely terminated
} process_state_t;

// Thread States
typedef enum {
    THREAD_STATE_NEW = 0,       // Thread being created
    THREAD_STATE_READY,         // Ready to be scheduled
    THREAD_STATE_RUNNING,       // Currently running
    THREAD_STATE_BLOCKED,       // Blocked on resource
    THREAD_STATE_SLEEPING,      // Sleeping for specified time
    THREAD_STATE_TERMINATED     // Thread terminated
} thread_state_t;

// Priority Levels
#define PRIORITY_HIGH           0
#define PRIORITY_NORMAL         10
#define PRIORITY_LOW            20
#define PRIORITY_IDLE           30
#define PRIORITY_LEVELS         31

// Process Control Block (PCB)
struct process {
    uint32_t pid;               // Process ID
    uint32_t ppid;              // Parent process ID
    char name[64];              // Process name
    process_state_t state;      // Current state
    
    // Memory management
    uint64_t page_directory;    // Page directory physical address
    uint64_t heap_start;        // Heap start address
    uint64_t heap_end;          // Heap end address
    uint64_t stack_start;       // Stack start address
    uint64_t stack_end;         // Stack end address
    
    // Timing information
    uint64_t creation_time;     // Process creation time
    uint64_t cpu_time;          // Total CPU time used
    uint64_t last_scheduled;    // Last time scheduled
    
    // Priority and scheduling
    uint8_t priority;           // Process priority
    uint8_t nice;               // Nice value
    uint32_t time_slice;        // Time slice in milliseconds
    uint32_t remaining_time;    // Remaining time slice
    
    // Thread management
    struct thread *main_thread; // Main thread
    struct thread *threads;     // Thread list
    uint32_t thread_count;      // Number of threads
    
    // File descriptors and resources
    void *file_table;           // File descriptor table
    void *signal_handlers;      // Signal handler table
    
    // Process relationships
    struct process *parent;     // Parent process
    struct process *children;   // Child processes
    struct process *next;       // Next in process list
    struct process *prev;       // Previous in process list
};

// Thread Control Block (TCB)
struct thread {
    uint32_t tid;               // Thread ID
    uint32_t pid;               // Parent process ID
    thread_state_t state;       // Current state
    
    // CPU context
    struct cpu_state context;   // Saved CPU context
    uint64_t stack_pointer;     // Stack pointer
    uint64_t stack_base;        // Stack base address
    size_t stack_size;          // Stack size
    
    // Scheduling
    uint8_t priority;           // Thread priority
    uint32_t time_slice;        // Time slice
    uint32_t remaining_time;    // Remaining time
    uint64_t sleep_until;       // Sleep until time
    
    // Synchronization
    void *wait_queue;           // Wait queue if blocked
    void *mutex_list;           // Owned mutexes
    
    // Thread relationships
    struct process *process;    // Parent process
    struct thread *next;        // Next thread in process
    struct thread *sched_next;  // Next in scheduler queue
};

// Scheduler Statistics
struct scheduler_stats {
    uint64_t context_switches;  // Total context switches
    uint64_t processes_created; // Processes created
    uint64_t threads_created;   // Threads created
    uint64_t total_cpu_time;    // Total CPU time
    uint32_t active_processes;  // Active process count
    uint32_t active_threads;    // Active thread count
    uint32_t runnable_threads;  // Runnable thread count
};

// Scheduler Functions

// Initialization Functions
int process_init(void);
int thread_init(void);

// Helper Functions  
void set_current_process(struct process *proc);
void set_current_thread(struct thread *thread);
uint64_t get_system_time(void);

// Process Management
struct process* create_process(const char *name, uint32_t parent_pid);
int destroy_process(uint32_t pid);
struct process* get_process(uint32_t pid);
struct process* get_current_process(void);

// Thread Management
struct thread* create_thread(uint32_t pid, void (*entry_point)(void*), void *arg);
int destroy_thread(uint32_t tid);
struct thread* get_thread(uint32_t tid);
struct thread* get_current_thread(void);

// Scheduling
int scheduler_init(void);
void schedule(void);
void yield(void);
void sleep(uint64_t milliseconds);
void wakeup(struct thread *thread);

// Priority Management
int set_process_priority(uint32_t pid, uint8_t priority);
int set_thread_priority(uint32_t tid, uint8_t priority);
uint8_t get_process_priority(uint32_t pid);
uint8_t get_thread_priority(uint32_t tid);

// Process Control
int suspend_process(uint32_t pid);
int resume_process(uint32_t pid);
int kill_process(uint32_t pid, int signal);
int wait_process(uint32_t pid, int *status);

// Statistics and Information
struct scheduler_stats* get_scheduler_stats(void);
void print_process_list(void);
void print_thread_list(uint32_t pid);

// Scheduler Control
void scheduler_enable(bool enable_preemption);
void scheduler_disable(void);
void scheduler_tick(void);
void scheduler_add_thread(struct thread *thread);
void scheduler_remove_thread(struct thread *thread);
int scheduler_set_policy(uint8_t policy);
uint8_t scheduler_get_policy(void);
void print_scheduler_status(void);

// Scheduler Policies
#define SCHED_POLICY_ROUND_ROBIN    1
#define SCHED_POLICY_PRIORITY       2
#define SCHED_POLICY_CFS            3  // Completely Fair Scheduler
#define SCHED_POLICY_REALTIME       4

// Time Slice Constants (in milliseconds)
#define TIME_SLICE_DEFAULT          20
#define TIME_SLICE_INTERACTIVE      10
#define TIME_SLICE_BATCH           100
#define TIME_SLICE_REALTIME          5

#endif // SCHEDULER_H 