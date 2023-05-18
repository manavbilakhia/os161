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
#include <file_table.h>
#include <vfs.h>

int
sys_close(int fd){
    KASSERT(curthread != NULL);

    struct file *file = curproc -> p_filetable -> files[fd];
    lock_acquire(file->lock);

    int result;

    result = ft_remove_file(curproc -> p_filetable, fd);
    if(result == EBADF){
        return EBADF;
    }

    result = ft_remove_file(kproc -> p_filetable, fd);
    if(result == EBADF){
        return EBADF;
    }

    lock_release(file->lock);
    return 0;
}