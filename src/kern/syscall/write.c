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

//ssize_t
//sys_write(int fd, const void *buf, size_t nbytes){
//    char *print;
//    print = (char *) kmalloc(nbytes);
//    int ret;
//    (void) fd;
//    ret = copyin((const_userptr_t) buf, print, nbytes);
//    (void) ret;
//    kprintf("%s", ((char *) buf));
//    return nbytes;
//}

ssize_t
sys_write(int fd, const void *buf, size_t nbytes){
//code to write on the terminal
    if (fd < 3)
    {
        char *print;    
        print = (char *) kmalloc(nbytes); 
        int ret = copyin((const_userptr_t) buf, print, nbytes);
        (void) ret;
        kprintf("%s", ((char *) buf));
        return nbytes;   
    }
    else
    {
    // Check if fd is valid
    if (fd < 0 || fd >= MAX_FILES) {
        return -EBADF;
    }

    // Look up the file and check wiuth some basic error codes
    struct file *file = curproc->p_filetable->files[fd];
    if (file == NULL || !(file->flags & O_WRONLY || file->flags & O_RDWR || file->flags & O_ACCMODE || file->flags & O_RDONLY)) {
        return -EBADF;
    }
    KASSERT (file->lock != NULL);
    lock_acquire(file->lock);

    //  Write the bytes from the buffer to the file and update the current seek position of the file.
    struct uio write_uio;
    struct iovec write_iov;
    uio_kinit(&write_iov, &write_uio, (void *)buf, nbytes, file->offset, UIO_WRITE);
    int result = VOP_WRITE(file->vn, &write_uio);
    if (result) {
        lock_release(file->lock);
        return result;
    }

    // Update the file offset
    file->offset = write_uio.uio_offset;

    lock_release(file->lock);

    // Return the number of bytes written
    return nbytes - write_uio.uio_resid;
    }
}

