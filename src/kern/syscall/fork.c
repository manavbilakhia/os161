#include <syscall.h>
#include <types.h>
#include <proc.h>
#include <proc_table.h>
#include <addrspace.h>
#include <mips/tlb.h>
#include <mips/trapframe.h>
#include <file_table.h>
#include <kern/errno.h>
#include <kern/syscall.h>
#include <current.h>
#include <lib.h>

static void child_proc_handler(void * data1, unsigned long data2);

int sys_fork(struct trapframe *tf_parent, int * return_value){
    /*
     * Documentation to be written.
     */
    const char *child_name = "child_proc";
    struct proc *child = proc_create_runprogram(child_name);
    if (child == NULL) { return -ENOMEM; }

    struct addrspace * address_space_child = NULL;
    int result_addrcopy = as_copy(curproc->p_addrspace, &address_space_child);

    if (result_addrcopy != 0) {
        *return_value = -1;
        return result_addrcopy; 
    }

    child -> parent_process_id = curproc -> process_id;
    struct trapframe * tf_child = trapframe_copy(tf_parent);

    if (tf_child == NULL) { return -ENOMEM; }

    struct file_table * parent_file_table = curproc->p_filetable;
    struct file_table * child_file_table = child->p_filetable;
    memcpy(child_file_table, parent_file_table, sizeof(struct file_table));

    int fork_result = thread_fork("creating new proc", child, child_proc_handler, tf_child, (unsigned long) address_space_child);
    if (fork_result != 0) { 
        kfree(tf_child);
        as_destroy(address_space_child);
        return fork_result; 
    } 
    
    *return_value = child->process_id;
    return 0;
}

static void child_proc_handler(void * data1, unsigned long data2){
    /*
     * Documentation to be written.
     */
    struct trapframe *tf_child;
    tf_child = (struct trapframe *) data1;
    unsigned long address_space_child = data2;
    
    curproc->p_addrspace = (struct addrspace * ) address_space_child;
    as_activate();
    KASSERT(curproc->p_addrspace != NULL);

    tf_child -> tf_a3 = 0;
    tf_child -> tf_v0 = 0;
    tf_child -> tf_epc += 4;
    struct trapframe tf = *tf_child;
    KASSERT(data1 != NULL);
    kfree(data1);
    mips_usermode(&tf); 
}