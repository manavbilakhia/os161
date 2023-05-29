#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <current.h>
#include <thread.h>
#include <vfs.h>
#include <syscall.h>
#include <uio.h>
#include <proc.h>
#include <addrspace.h>
#include <vnode.h>
#include <elf.h>
#include <thread.h>
#include <synch.h>
#include <limits.h>
#include <copyinout.h>

int
sys_dup2(int oldfd, int newfd)
{
    // Check if oldfd is a valid file descriptor 
    if (oldfd < 0 || oldfd >= MAX_FILES || curproc->p_filetable->files[oldfd] == NULL) {
        return -EBADF;
    }
    
    // Lock the file table before modifying it
    lock_acquire(curproc->p_filetable->lock);
    
    // Check if newfd is a valid file descriptor 
    if (newfd < 0 || newfd >= MAX_FILES) {
        lock_release(curproc->p_filetable->lock);
        return -EBADF;
    }
    
    // If oldfd equals newfd, return newfd without doing anything 
    if (oldfd == newfd) {
        lock_release(curproc->p_filetable->lock);
        return newfd;
    }   
    // Check if process's file table is full
    if (curproc->p_filetable->number_files >= MAX_FILES) {
        lock_release(curproc->p_filetable->lock);
        return -EMFILE;
    }
    
    // If newfd points to an open file, close it
    if (curproc->p_filetable->files[newfd] != NULL) {
        int err = ft_remove_file(curproc->p_filetable, newfd);
        if (err < 0) {
            lock_release(curproc->p_filetable->lock);
            return err;
        }
    }

    // Clone the file pointed by oldfd to newfd.
    struct file *oldfile = curproc->p_filetable->files[oldfd];
    // Increment reference count of vnode 
    VOP_INCREF(oldfile->vn);
    curproc->p_filetable->files[newfd] = oldfile;

    lock_release(curproc->p_filetable->lock);

    return newfd;
}