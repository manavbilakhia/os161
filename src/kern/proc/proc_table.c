#include <types.h>
#include <proc_table.h>
#include <lib.h>
#include <synch.h>
#include <proc.h>
#include <kern/errno.h>
#include <spinlock.h>

#define MAX_ACTIVE_PROCS 1000

struct proc_table *global_proc_table;

void proc_table_create(void){
    /*
    Creates a process table.
    */
    global_proc_table = kmalloc(sizeof(struct proc_table));
    KASSERT(global_proc_table != NULL);

    spinlock_init(&((global_proc_table) -> pt_lock));
    (global_proc_table) -> active_procs = 0;

    for (int j = 2; j < MAX_ACTIVE_PROCS; j++) { (global_proc_table) -> proc_table_map[j] = NULL; } // is this line necessary?
}

void proc_table_destroy(struct proc_table * pt){
    /*
    Deallocates space for a process table
    */
    KASSERT(pt != NULL);

    if (pt -> active_procs != 0){
        for (int j = 0; j < MAX_ACTIVE_PROCS; j++){
                if (pt -> proc_table_map[j]!= NULL){ proc_destroy(pt -> proc_table_map[j]); }
        }
    }
    spinlock_cleanup(&pt->pt_lock);
    kfree(pt);
}

int add_proc(pid_t pid, struct proc_table *pt, struct proc * p){
    /*
    Adds a process to the process table, if not full. Returns 0 upon success, else an error. 
    */
    KASSERT(pt != NULL);

    if (!valid_pid(pid)) { return EINVAL; }

    spinlock_acquire(&pt -> pt_lock);

    pt -> proc_table_map[pid] = p;
    pt -> active_procs++;

    spinlock_release(&pt -> pt_lock);

    return 0;
}

struct proc * get_proc(int pid, struct proc_table *pt){
    /*
    Returns a process from the process table, given a pid. Returns 0 upon success, else an error. */
    KASSERT(pt != NULL);
    if (!valid_pid(pid)) { return NULL; } 

    spinlock_acquire(&pt -> pt_lock);

    struct proc *p = pt->proc_table_map[pid];

    if (p == NULL){
        spinlock_release(&pt -> pt_lock);
        return NULL; 
    } 

    spinlock_release(&pt -> pt_lock);

    return p;
}

pid_t get_available_pid(struct proc_table *pt){
    /*
    Gets the next available pid
    */
    KASSERT(pt != NULL);
    spinlock_acquire(&pt->pt_lock);

    for (int j = 2; j < MAX_ACTIVE_PROCS; j++){
        if (pt -> proc_table_map[j] == NULL){
            spinlock_release(&pt->pt_lock);
            return j;
        }
    }
    spinlock_release(&pt->pt_lock);
    return -1; // need to define proper error in errno.h
}

struct proc * remove_process(struct proc_table *pt, pid_t pid){
    /*
    Removes a process from the process table
    */
    KASSERT(pt != NULL);
    if (!valid_pid(pid)){ return NULL; } // we need to replace with proper error code, need to return a pointer.

    spinlock_acquire(&pt -> pt_lock); 

    struct proc * removed_proc = pt -> proc_table_map[pid];
    if (removed_proc != NULL) {
        pt -> proc_table_map[pid] = NULL;
        pt -> active_procs--;
    }

    spinlock_release(&pt -> pt_lock);
    return removed_proc;
    
}

bool valid_pid(pid_t pid){ return (pid >= 2 && pid < MAX_ACTIVE_PROCS); 
    /*
    Checks if the pid given is valid.
    */
    }
