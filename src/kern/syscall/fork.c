#include <kern/fork.h>
#include <syscall.h>
#include <types.h>
#include <proc.h>
#include <proc_table.h>
#include <addrspace.h>
#include <mips/trapframe.h>
#include <filetable.h>
#include <kern/errno.h>


int SYS_fork(struct trapframe *tf_parent, int * return_value){
    /*
     * Documentation to be written.
     */
    KASSERT(curthread->t_addrspace != NULL);

    struct addrspace * address_space_child = NULL;
    int result_addrcopy = as_copy(curthread->t_addrspace, &address_space_child);

    if (result_addrcopy != 0) {
        *return_value = -1;
        return result_addrcopy; // will return the error specified by as_copy
    }
    struct trapframe * tf_child = (struct trapframe *)kmalloc(sizeof(struct trapframe));
    *tf_child = *tf_parent;

    if (tf_child == NULL) { return ENOMEM; }

    // copy filetable
    // create thread
    //entry point??

    fork_result = thread_fork("creating entry point for new thread", curthread->t_proc, child_entry_point)
}

void child_entry_point(){

}