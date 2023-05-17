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

int sys_open(const char *filename, int flags){
    //Check if filename is a valid pointer
    if(filename == NULL){
        return EFAULT;
    }

    struct file_table *ftable = curproc->p_filetable;
    KASSERT(ftable != NULL);
    char *kpath;
    size_t actual;
    int result;

    // Copy the filename into kernel space
    kpath = (char *) kmalloc(sizeof(char)*(PATH_MAX+1));
    if(kpath == NULL){
        return ENOMEM;
    }


    result = copyinstr((userptr_t)filename, kpath, PATH_MAX+1, &actual);
    if(result){
        kfree(kpath);
        return result;
    }

    // Ensure filename is null-terminated stackoverflow is the big beast
    kpath[actual-1] = '\0';

    struct vnode *vn;
    result = vfs_open(kpath, flags, 0, &vn);
    if (result) {
        kfree(kpath);
        return result;
    }

    // Create file and return file descriptor or an error code
    int fd = file_create(ftable, kpath);
    if(fd < MIN_FD){
        vfs_close(vn);
        kfree(kpath);
        return fd;
    }

    //Update vnode in file
    ftable->files[fd]->vn = vn;

    kfree(kpath);
    return fd;
}