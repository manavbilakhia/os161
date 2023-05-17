#ifndef _PROC_TABLE_H
#define _PROC_TABLE_H

#include <spinlock.h>
#include <types.h>
#include <proc.h>

#define MAX_ACTIVE_PROCS 1000

extern struct proc_table *global_proc_table;

struct proc_table{
    /*
    Defines a process table, which maps process id's to processes.
    */
    struct spinlock pt_lock; // lock for synchronization
    volatile int active_procs; // number of active processes

    struct proc* proc_table_map[MAX_ACTIVE_PROCS];

    volatile int next_available_spot;
};

void proc_table_create(struct proc_table **pt);

void proc_table_destroy(struct proc_table *pt);

int add_proc(pid_t pid, struct proc_table *pt, struct proc *p); // set int to indicate success/failure of adding

struct proc * get_proc(pid_t pid, struct proc_table *pt); // use second parameter as way of returning the process, set int to indicate success/failure of finding

pid_t get_available_pid(struct proc_table *pt); // same strategy as previous

struct proc * remove_process(struct proc_table * pt, pid_t pid);

bool valid_pid(pid_t pid);

#endif /* _PROC_TABLE_H */