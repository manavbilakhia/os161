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
#include <spl.h>
#include <file_table.h>
#include <kern/wait.h>


void 
sys__exit(int exitcode){ 
    /*
     * Removes a process from the table of active processes.
     */
    struct proc * curproc_stack = curproc;
    if (curproc == NULL) { panic("missing process for exit call"); }
    if (curproc_stack == NULL) {panic("failed to make local copy");}
    curproc_stack -> finished = true;
   //ft_destroy(curproc_stack->p_filetable);

    set_exit_code(curproc_stack->process_id, global_proc_table, _MKWAIT_EXIT(exitcode));

    remove_process(global_proc_table, curproc -> process_id);

    KASSERT(waitpidlock != NULL);
    
    lock_acquire(waitpidlock);
    proc_remthread(curthread);
    cv_signal(waitpidcv, waitpidlock);

    KASSERT(curproc_stack != NULL);
    proc_destroy(curproc_stack);
    lock_release(waitpidlock);
    thread_exit();
}
