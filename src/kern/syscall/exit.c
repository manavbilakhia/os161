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


int
sys__exit(int exitcode){
    /*
     * Removes a process from the table of active processes.
     */
    if (curthread == NULL || curproc == NULL){ 
        exitcode = -1; // SWITCH TO PROPER ERROR CODE
        return -1;
    }
    splhigh();
    ft_destroy(curproc ->p_filetable);
    (void)exitcode;
    remove_process(global_proc_table, curproc->process_id);
    proc_destroy(curproc);
    thread_exit();

    exitcode = 0;
    return 0;
}