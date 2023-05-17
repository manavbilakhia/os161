#include <syscall.h>
#include <types.h>
#include <proc_table.h>
#include <kern/errno.h>
#include <synch.h>
#include <lib.h>
#include <uio.h>
#include <proc.h>
#include <current.h>
#include <addrspace.h>
#include <vnode.h>
#include <elf.h>
#include <thread.h>
#include <synch.h>
#include <kern/syscall.h>


pid_t sys_waitpid(pid_t child_pid, userptr_t status, int options){
    /*
     * Documentation to be written.
     */
    if (!valid_pid(child_pid)){ return EINVAL; }
    if (get_proc(child_pid, global_proc_table) == NULL){ return ESRCH; }
    if (status == NULL){ return EFAULT; }
    if (get_proc(child_pid, global_proc_table)->parent_process_id == curproc->parent_process_id){ return ECHILD; }

    options = options + 0; // TEMPORARY TO MAKE COMPILE REMOVE LATER
    return curproc->process_id; // TEMPORARY TO MAKE COMPILE REMOVE LATER
    
}

pid_t sys_getpid(void){ return curproc->process_id; 
    /*
     * Returns the pid of the current process.
     */
}