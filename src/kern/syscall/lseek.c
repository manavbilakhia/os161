#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <uio.h>
#include <proc.h>
#include <current.h>
#include <syscall.h>
#include <vnode.h>
#include <vfs.h>
#include <kern/fcntl.h>
#include <copyinout.h>
#include <file_table.h>
#include <kern/seek.h>
#include <synch.h>
#include <stat.h>

off_t
lseek(int fd, off_t pos, int whence){
    struct stat stat;
    off_t new_pos;
    int ret;

    if (fd < 0 || fd >= MAX_FILES) {
        return -EBADF;
    }

    struct file *file = curproc->p_filetable->files[fd];
    if(file == NULL){
        return -ESPIPE;
    }

    if(pos + whence < 0){
        return -EINVAL;
    }

    if(whence == SEEK_SET){
        new_pos = pos;
    } else if(whence == SEEK_CUR){
        new_pos = file -> offset + pos;
    } else if(whence == SEEK_END){
        ret = VOP_STAT(file -> vn, &stat);
        if(ret){
            return -ret;
        }
        new_pos = pos + stat.st_size;
    } else{
        return -EINVAL;
    }

    spinlock_acquire(&curproc -> p_lock);
    file -> offset = new_pos;
    spinlock_release(&curproc -> p_lock);
    return new_pos;
}