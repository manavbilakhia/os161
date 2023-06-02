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
#include <copyinout.h>



pid_t sys_waitpid(pid_t child_pid, userptr_t status, int options){
    /*
     * Documentation to be written.
     */
    if (!valid_pid(child_pid)){ return -EINVAL; }
    if (options != 0){ return -EINVAL; }
    if (get_proc(child_pid, global_proc_table) == NULL){ return -ESRCH; }
    if (get_proc(child_pid, global_proc_table)->parent_process_id == curproc->parent_process_id){ return -ECHILD; }

    lock_acquire(waitpidlock);
    struct proc * child_proc = get_proc(child_pid, global_proc_table);

    if (child_proc -> finished == true){ 
        copyout((const void *)get_exit_code(child_proc->process_id, global_proc_table), status, sizeof(int));
        lock_release(waitpidlock);
        proc_destroy(child_proc);
        return child_pid;
    }
    else{
        while (get_proc(child_pid, global_proc_table) -> finished != true){
            cv_wait(waitpidcv, waitpidlock);
        }
        copyout((const void *)get_exit_code(child_proc->process_id, global_proc_table), status, sizeof(int));
        lock_release(waitpidlock);
        proc_destroy(get_proc(child_pid, global_proc_table));
        return child_pid; 
    } 
}

pid_t sys_getpid(void){ return curproc->process_id; 
    /*
     * Returns the pid of the current process.
     */
}