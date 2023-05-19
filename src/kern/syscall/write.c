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
#include <kern/unistd.h>


ssize_t
sys_write(int fd, const void *buf, size_t nbytes){
    // Check if fd is valid
    int result;
    if (fd < 0 || fd >= MAX_FILES ) {
        return -EBADF;
    }
    if (buf == NULL)
        return -EFAULT;
    char *buffer = (char* ) kmalloc (nbytes);
    if (buffer == NULL)
    {
        return -EIO;
    }
    result = copyin((const_userptr_t)buf, buffer, nbytes);
    if (result !=0)
        return -result;
    // handle console output
    if (fd == STDIN_FILENO)
    {
        kfree(buffer);
        return EBADF;
    }
    if (fd == STDOUT_FILENO || fd == STDERR_FILENO) {
        size_t i;

        for (i = 0; i < nbytes; i++) {
            putch(buffer[i]);
        }
        kfree (buffer);
        return i; // return the number of characters written
    }
    // Look up the file and check wiuth some basic error codes
    struct file *file = curproc->p_filetable->files[fd];
    if (file == NULL || !(file->flags & O_WRONLY || file->flags & O_RDWR || file->flags & O_ACCMODE || file->flags & O_RDONLY)) {
        return -EBADF;
    }
    lock_acquire(file->lock);

    //  Write the bytes from the buffer to the file and update the current seek position of the file.
    struct uio write_uio;
    struct iovec write_iov;
    uio_kinit(&write_iov, &write_uio, (void *)buf, nbytes, file->offset, UIO_WRITE);
    result = VOP_WRITE(file->vn, &write_uio);
    if (result) {
        lock_release(file->lock);
        return -result;
    }

    // Update the file offset
    file->offset = write_uio.uio_offset;

    lock_release(file->lock);

    // Return the number of bytes written
    return nbytes - write_uio.uio_resid;
    }


