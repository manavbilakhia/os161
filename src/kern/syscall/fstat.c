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
#include <vfs.h>
#include <limits.h>
#include <copyinout.h>
#include <stat.h>
#include <file_table.h>

/* Implementation of fstat syscall */
int
sys_fstat(int fd, userptr_t statbuf)
{   
        if(statbuf == NULL) {
        return -EFAULT;
    }

    /* Check if the file descriptor is valid */
    if(fd < 0 || fd >= MAX_FILES) {
        return -EBADF;
    }
    
    /* Look up the file descriptor in the file table */
    int result = ft_look_up(curproc->p_filetable, fd);
    if(result < 0) {
        /* Return the error if the file descriptor is invalid */
        return result;
    }
    
    /* Get the file */
    struct file *file = curproc->p_filetable->files[fd];
    
    /* Check if the file is valid */
    if(file == NULL) {
        return -EBADF;
    }

    /* Create a stat structure to hold the file's information */
    struct stat info;
    int err = VOP_STAT(file->vn, &info);
    if(err!= 0) {
        return -err;
    }

    /* Copy the stat structure to user space */
    err = copyout(&info, statbuf,  sizeof(struct stat));
    if(err!= 0) {
        /* If an error occurred during the copy, return EFAULT */
        return -err;
    }

    /* If everything is successful, return 0 */
    return 0;
}