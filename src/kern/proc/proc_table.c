#include <proc_table.h>
#include <types.h>
#include <lib.h>
#include <synch.h>
#include <proc.h>
#include <kern/errno.h>
#include <spinlock.h>

#define MAX_ACTIVE_PROCS 100

void proc_table_create(struct proc_table ** pt){
    /*
    Creates a process table.
    */
    *pt = kmalloc(sizeof(struct proc_table));
    if (*pt == NULL) { kprintf("Hey! Where's my process table?"); }

    spinlock_init(&((*pt) -> pt_lock));
    (*pt) -> active_procs = 0;
}

void proc_table_destroy(struct proc_table * pt){
    /*
    Deallocates space for a process table
    */
    KASSERT(pt != NULL);

    if (pt -> active_procs != 0){
        for (int i = 0; i < 2; i++){
            for (int j = 0; j < MAX_ACTIVE_PROCS; j++){
                if (pt -> proc_table_map[i][j].procPtr != NULL){ proc_destroy(pt -> proc_table_map[i][j].procPtr); }
            }
            kfree(pt -> proc_table_map[i]);
        }
    }
    spinlock_cleanup(&pt->pt_lock);
    kfree(pt);
}

int add_proc(int pid, struct proc_table *pt, struct proc * p){
    /*
    Adds a process to the process table, if not full. Returns 0 upon success, else an error. 
    */
    KASSERT(pt != NULL);

    if (!valid_pid(pid)) { return EINVAL; }

    if (proc_table_full(pt)) { return -1; } //need to go define proper error code in errno.h

    spinlock_acquire(&pt -> pt_lock);

    pt -> proc_table_map[0][pid].pid = pid;
    pt -> proc_table_map[1][pid].procPtr = p;
    pt -> active_procs++;

    spinlock_release(&pt -> pt_lock);

    return 0;
}

int get_proc(int pid, struct proc_table *pt, struct proc **p){
    /*
    Returns a process from the process table, given a pid. Returns 0 upon success, else an error. */
    KASSERT(pt != NULL);
    if (!valid_pid(pid)) { return EINVAL; } 

    spinlock_acquire(&pt -> pt_lock);

    *p = pt->proc_table_map[1][pid].procPtr;

    if (*p == NULL) { return ESRCH; } 

    spinlock_release(&pt -> pt_lock);

    return 0;
}

int get_available_pid(struct proc_table *pt){
    /*
    Gets the next available pid
    */
    KASSERT(pt != NULL);
    spinlock_acquire(&pt->pt_lock);

    for (int i = 0; i < 2; i++){
        for (int j = 0; j < MAX_ACTIVE_PROCS; j++){
            if (pt -> proc_table_map[i][j].procPtr == NULL){
                pt->proc_table_map[i][j].pid = j;
                spinlock_release(&pt->pt_lock);
                return j;
            }
        }
    }
    spinlock_release(&pt->pt_lock);
    return -1; // need to define proper error in errno.h
}

bool proc_table_full(struct proc_table *pt){
    /*
    Returns whether or not the process table is full
    */
    KASSERT(pt != NULL);
    spinlock_acquire(&pt->pt_lock);

    for (int i = 0; i < 2; i++){
        for (int j = 0; j < MAX_ACTIVE_PROCS; j++){
            if (pt -> proc_table_map[i][j].procPtr == NULL){
                spinlock_release(&pt->pt_lock);
                return false;
            }
        }
    }
    spinlock_release(&pt->pt_lock);
    return true;
}

struct proc * remove_process(struct proc_table *pt, int pid){
    /*
    Removes a process from the process table
    */
    KASSERT(pt != NULL);
    if (!valid_pid(pid)){ return NULL; } // we need to replace with proper error code, need to return a pointer.

    spinlock_acquire(&pt -> pt_lock); 

    struct proc * removed_proc = pt -> proc_table_map[1][pid].procPtr;
    if (removed_proc != NULL) {
        pt -> proc_table_map[1][pid].procPtr = NULL;
        pt -> active_procs--;
    }

    spinlock_release(&pt -> pt_lock);
    return removed_proc;
    
}

bool valid_pid(int pid){ return (pid >= 0 && pid < MAX_ACTIVE_PROCS); 
    /*
    Checks if the pid given is valid.
    */
    }
