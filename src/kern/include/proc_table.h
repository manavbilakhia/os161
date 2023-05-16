#ifndef _PROC_TABLE_H
#define _PROC_TABLE_H

#include <spinlock.h>
#include <types.h>

#define MAX_ACTIVE_PROCS 100

struct proc_table{
    /*
    Defines a process table, which maps process id's to processes.
    */
    struct spinlock pt_lock; // lock for synchronization
    volatile int active_procs; // number of active processes

    struct {
        int pid;
        struct proc* procPtr;
    } proc_table_map[2][MAX_ACTIVE_PROCS]; // 2d array mapping pids to processes

};

void proc_table_create(struct proc_table **pt);

void proc_table_destroy(struct proc_table *pt);

int add_proc(int pid, struct proc_table *pt, struct proc *p); // set int to indicate success/failure of adding

int get_proc(int pid, struct proc_table *pt, struct proc **p); // use second parameter as way of returning the process, set int to indicate success/failure of finding

int get_available_pid(struct proc_table *pt); // same strategy as previous

bool proc_table_full(struct proc_table *pt);

struct proc * remove_process(struct proc_table * pt, int pid);

bool valid_pid(int pid);

#endif /* _PROC_TABLE_H */