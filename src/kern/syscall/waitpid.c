#include <syscall.h>
#include <types.h>
#include <proc_table.h>
#include <kern/errno.h>
#include <synch.h>

pid_t sys_waitpid(pid_t child_pid, userptr_t status, int options){
    if (!validpid(child_pid)){
        return EINVAL;
    }
    if (get_proc(child_pid, global_proc_table) == NULL){
        return ESRCH;
    }
    if (status == NULL){
        return EFAULT;
    }

}