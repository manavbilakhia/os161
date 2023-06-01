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
#include <endian.h>

off_t
sys_lseek(int fd, off_t pos, int whence){
    struct stat stat;
    off_t new_pos;
    int ret;
    off_t pos_64;

    join32to64((uint32_t) fd, (uint32_t) pos, (uint64_t *) &pos_64);

    if (fd < 0 || fd >= MAX_FILES) {
        return -EBADF;
    }

    struct file *file = curproc->p_filetable->files[fd];
    if(file == NULL){
        return -EBADF;
    }

    if(!VOP_ISSEEKABLE(file -> vn)){
        return -ESPIPE;
    }

    if(whence < 0){
        return -EINVAL;
    }
    if(pos + whence < 0){
        return -EINVAL;
    }

    lock_acquire(file -> lock);
    if(whence == SEEK_SET){
        new_pos = pos_64;
    } else if(whence == SEEK_CUR){
        new_pos = file -> offset + pos_64;
    } else if(whence == SEEK_END){
        ret = VOP_STAT(file -> vn, &stat);
        if(ret){
            lock_release(file -> lock);
            return -ret;
        }
        new_pos = pos_64 + stat.st_size;
    } else{
        lock_release(file -> lock);
        return -EINVAL;
    }

    file -> offset = new_pos;
    lock_release(file -> lock);
    return new_pos;
}