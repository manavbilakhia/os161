#include <types.h>
#include <syscall.h>
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

pid_t sys_fork(struct trapframe *tf_parent){
    const char *child_name = "child_proc";
    struct proc *child = proc_create(child_name);
    if (child == NULL) { 
        kfree(child);
        return -ENOMEM; 
    }
    pid_t tmp_pid = child->process_id;

    int result_addrcopy = as_copy(curproc->p_addrspace, &child->p_addrspace);

    if (result_addrcopy != 0) {
        as_destroy(child->p_addrspace);
        kfree(child);
        return -result_addrcopy; 
    }

    child -> parent_process_id = curproc -> process_id;
    struct trapframe * tf_child = trapframe_copy(tf_parent);

    if (tf_child == NULL) {
        kfree(tf_child);
        as_destroy(child->p_addrspace);
        kfree(child); 
        return -ENOMEM; 
    }

    struct file_table * child_file_table = ft_clone(curproc->p_filetable);
    child->p_filetable = child_file_table;
    int fork_result = thread_fork("creating new proc", child, child_proc_handler, tf_child, (unsigned long) child -> p_addrspace);
    if (fork_result != 0) { 
        kfree(tf_child);
        ft_destroy(child_file_table);
        as_destroy(child->p_addrspace);
        kfree(child);
        return -fork_result; 
    } 
    return (int) tmp_pid;
}

static void child_proc_handler(void * data1, unsigned long data2){
    struct trapframe *tf_child;
    tf_child = (struct trapframe *) data1;
    unsigned long address_space_child = data2;
    
    curproc->p_addrspace = (struct addrspace * ) address_space_child;
    KASSERT(curproc->p_addrspace != NULL);
    as_activate();
    tf_child -> tf_a3 = 0;
    tf_child -> tf_v0 = 0;
    tf_child -> tf_epc += 4;
    struct trapframe tf = *tf_child;
    KASSERT(data1 != NULL);
    kfree(data1);
    mips_usermode(&tf); 
}