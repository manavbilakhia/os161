#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <uio.h>
#include <proc.h>
#include <current.h>
#include <addrspace.h>
#include <vnode.h>
#include <elf.h>
#include <syscall.h>
#include <thread.h>
#include <synch.h>
#include <proc_table.h>


int
sys__exit(int exitcode){
    /*
     * Removes a process from the table of active processes.
     */
    KASSERT(curthread != NULL);
    splhigh();
    // close the open files of the proc
    remove_process(global_proc_table, curproc->process_id);
    proc_destroy(curproc);
    thread_exit();
}