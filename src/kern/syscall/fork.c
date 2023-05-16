#include <kern/fork.h>
#include <syscall.h>
#include <types.h>
#include <proc.h>
#include <proc_table.h>
#include <addrspace.h>
#include <mips/trapframe.h>
#include <filetable.h>
#include <kern/errno.h>


int sys_fork(struct trapframe *tf_parent, int * return_value){
    /*
     * Documentation to be written.
     */
    KASSERT(curthread->t_addrspace != NULL);

    // create new proc
    const char *child_name = "child_proc";
    struct proc *child = proc_create(child_name);
    if (child == NULL) { return ENOMEM; }

    // get address space, file table, and trapframe for new proc
    struct addrspace * address_space_child = NULL;
    int result_addrcopy = as_copy(curthread->t_addrspace, &address_space_child);

    if (result_addrcopy != 0) {
        *return_value = -1;
        return result_addrcopy; // will return the error specified by as_copy
    }
    struct trapframe * tf_child = (struct trapframe *)kmalloc(sizeof(struct trapframe));
    *tf_child = *tf_parent;

    if (tf_child == NULL) { return ENOMEM; }

    struct file_table * parent_file_table = curproc->p_filetable; // need to get filetable merge
    struct file_table * child_file_table = child->p_filetable;
    *parent_file_table = *child_file_table;

    int fork_result = thread_fork("creating entry point for new proc", &child, child_entry_point, tf_child, (unsigned long) address_space_child);
    if (fork_result != 0) { return fork_result; } // need to deallocate space in these if checks, make sure to stop memory leaks
    
    *return_value = get_pid(child);
    return 0;
}

void child_entry_point(struct trapframe * tf_child, unisigned long address_space_child){
    curthread->t_addrspace = (struct addrspace * ) address_space_child;
    KASSERT(curthread->t_addrspace != NULL);


    // what next??????

    // update trapframe???
}