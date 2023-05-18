#include <syscall.h>
#include <types.h>
#include <proc.h>
#include <proc_table.h>
#include <addrspace.h>
#include <mips/trapframe.h>
#include <file_table.h>
#include <kern/errno.h>
#include <kern/syscall.h>
#include <current.h>
#include <lib.h>

static void child_entry_point(void * data1, unsigned long data2);

int sys_fork(struct trapframe *tf_parent, int * return_value){
    /*
     * Documentation to be written.
     */
    KASSERT(curproc->p_addrspace != NULL);

    // create new proc and associate processes
    const char *child_name = "child_proc";
    struct proc *child = proc_create(child_name);
    child->parent_process_id = curproc -> process_id;

    if (child == NULL) { return ENOMEM; }

    // get address space, file table, and trapframe for new proc
    struct addrspace * address_space_child = NULL;
    int result_addrcopy = as_copy(curproc->p_addrspace, &address_space_child);

    if (result_addrcopy != 0) {
        *return_value = -1;
        return result_addrcopy; // will return the error specified by as_copy
    }
    struct trapframe * tf_child = (struct trapframe *)kmalloc(sizeof(struct trapframe));
    *tf_child = *tf_parent;

    if (tf_child == NULL) { return ENOMEM; }

    struct file_table * parent_file_table = curproc->p_filetable; // need to get filetable merge (git)
    struct file_table * child_file_table = child->p_filetable;
    *parent_file_table = *child_file_table;

    int fork_result = thread_fork("creating entry point for new proc", child, child_entry_point, tf_child, (unsigned long) address_space_child);
    if (fork_result != 0) { return fork_result; } // need to deallocate space in these if checks, make sure to stop memory leaks
    
    *return_value = child->process_id;
    return 0;
}

static void child_entry_point(void * data1, unsigned long data2){
    /*
     * Documentation to be written.
     */
    KASSERT(curproc->p_addrspace != NULL);
    struct trapframe *tf_child = (struct trapframe *) data1;
    unsigned long address_space_child = data2;
    
    curproc->p_addrspace = (struct addrspace * ) address_space_child;
    as_activate();
    tf_child -> tf_a3 = 0;
    tf_child -> tf_v0 = 0;
    tf_child -> tf_epc += 4;
    mips_usermode(tf_child); // NEED TO SET PARENT_PROCESS_ID (ASSUMING THIS STRATEGY IS STILL USED)
}