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
#include <file_table.h>

int
sys_close(int fd){
    KASSERT(curthread != NULL);

    if (fd < 0 || fd >= MAX_FILES) {
        return -EBADF;
    }

    struct file *file = curproc -> p_filetable -> files[fd];
    if (file == NULL) {
        return -EBADF;
    }
    /* Issue ith STDIN in fle table */

    lock_acquire(file->lock);
    
    vfs_close(file -> vn);
    
    lock_release(file->lock);
    return 0;
}