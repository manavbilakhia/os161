#include <proc_table.h>
#include <types.h>
#include <lib.h>
#include <synch.h>
#include <proc.h>
#include <kern/errno.h>
#include <spinlock.h>

#define MAX_ACTIVE_PROCS 1000

void proc_table_create(struct proc_table ** pt){
    /*
    Creates a process table.
    */
    *pt = kmalloc(sizeof(struct proc_table));
    if (*pt == NULL) { kprintf("Hey! Where's my process table?"); }

    spinlock_init(&((*pt) -> pt_lock));
    (*pt) -> active_procs = 0;
    (*pt) -> next_available_spot = 0;

    for (int j = 0; j < MAX_ACTIVE_PROCS; j++) { (*pt) -> proc_table_map[j] = NULL; }
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
    pt -> next_available_spot++;

    spinlock_release(&pt -> pt_lock);

    return 0;
}

int get_proc(int pid, struct proc_table *pt, struct proc **p){
    /*
    Returns a process from the process table, given a pid. Returns 0 upon success, else an error. */
    KASSERT(pt != NULL);
    if (!valid_pid(pid)) { return EINVAL; } 

    spinlock_acquire(&pt -> pt_lock);

    *p = pt->proc_table_map[pid];

    if (*p == NULL) { return ESRCH; } 

    spinlock_release(&pt -> pt_lock);

    return 0;
}

pid_t get_available_pid(struct proc_table *pt){
    /*
    Gets the next available pid
    */
    KASSERT(pt != NULL);
    spinlock_acquire(&pt->pt_lock);

    for (int j = 0; j < MAX_ACTIVE_PROCS; j++){
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

bool valid_pid(pid_t pid){ return (pid >= 0 && pid < MAX_ACTIVE_PROCS); 
    /*
    Checks if the pid given is valid.
    */
    }
