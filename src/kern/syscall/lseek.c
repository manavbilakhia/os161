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
#include <spinlock.h>

off_t
sys_lseek(int fd, off_t pos, int whence){
    struct stat stat;
    struct spinlock lock;
    off_t new_pos;
    int ret;

    spinlock_init(&lock);
    spinlock_acquire(&lock);
    if (fd < 0 || fd >= MAX_FILES) {
        spinlock_release(&lock);
        return -EBADF;
    }

    struct file *file = curproc->p_filetable->files[fd];
    if(file == NULL){
        spinlock_release(&lock);
        return -EBADF;
    }

    if(!VOP_ISSEEKABLE(file -> vn)){
        spinlock_release(&lock);
        return -ESPIPE;
    }

    if(whence < 0){
        spinlock_release(&lock);
        return -EINVAL;
    }
    if(pos + whence < 0){
        spinlock_release(&lock);
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
        spinlock_release(&lock);
        return -EINVAL;
    }

    file -> offset = new_pos;
    spinlock_release(&lock);
    return new_pos;
}