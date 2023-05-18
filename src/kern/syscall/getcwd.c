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

int
sys___getcwd(char *buf, size_t buflen){
    struct uio uio;
    struct iovec iov;
    
    spinlock_acquire(&curproc->p_lock);
    uio_kinit(&iov, &uio, buf, buflen, 0, UIO_READ);

    int result = vfs_getcwd(&uio);
    if(result){
        spinlock_release(&curproc->p_lock);
        return result;
    }

    int ret = buflen - uio.uio_resid;
    spinlock_release(&curproc->p_lock);
    return ret;
}