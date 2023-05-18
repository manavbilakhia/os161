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


void 
sys__exit(int * exitcode){
    /*
     * Removes a process from the table of active processes.
     */
    (void)exitcode; 
    if (curthread == NULL || curproc == NULL){ 
        *exitcode = -1;
    }
    splhigh();
    ft_destroy(curproc -> p_filetable); // consider putting this step in proc_destroy
    remove_process(global_proc_table, curproc->process_id);
    proc_destroy(curproc);
    thread_exit();

    *exitcode = 0;
}