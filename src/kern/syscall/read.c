#include <types.h>
#include <syscall.h>
#include <current.h>
#include <proc.h>
#include <copyinout.h>
#include <vnode.h>
#include <uio.h>
#include <kern/errno.h>
#include <kern/fcntl.h>
#include <kern/unistd.h>

ssize_t
sys_read(int fd, void *buf, size_t buflen) {
    struct file *file;
    struct iovec iov;
    struct uio ku;
    char *buffer = (char *)kmalloc(sizeof(char) * buflen);

    if (fd < 0 || fd >= MAX_FILES) {
        kfree(buffer);
        return -EBADF;
    }

    if (buf == NULL)
    {
        kfree(buffer);
        return -EFAULT;
    }

    // handle console input
    if (fd == STDOUT_FILENO || fd == STDERR_FILENO)
    {
        kfree(buffer);
        return -EBADF;
    }
    if (fd == STDIN_FILENO) {
        size_t i;
        int ch;

        if (buffer == NULL)
        {
            return -EIO;
        }

        for (i = 0; i < buflen; i++) {
            ch = getch();

            if (ch < 0) {
                kfree(buffer);
                return ch; // getch() returned an error
            }

            buffer[i] = ch;

            if (ch == '\n' || ch == '\r') {
                buffer[i] = '\n';
                break; // end of line
            }
        }

        int result = copyout((const void*)buffer, (userptr_t)buf, (size_t)(i+1));
        kfree(buffer);
        if (result) {
            return -EFAULT;
        }
       
        return i; // return the number of characters read
    }

    // Get file from file table
    file = curproc->p_filetable->files[fd];
    if (file == NULL) {
        kfree(buffer);
        return -EBADF;
    }

    // Check for read permission
    if ((file->flags & O_ACCMODE) == O_WRONLY) {
        kfree(buffer);
        return -EBADF;
    }

    // Lock file
    KASSERT (file->lock != NULL);
    lock_acquire(file->lock);

    // Setup uio structure
    uio_kinit(&iov, &ku, buffer, buflen, file->offset, UIO_READ);

    // Perform read
    int result = VOP_READ(file->vn, &ku);
    if (result) {
        kfree(buffer);
        lock_release(file->lock);
        return -result;
    }
    result = copyout((const void*)buffer, (userptr_t)buf, (size_t)(buflen - ku.uio_resid));
    kfree(buffer);
    if (result) {
        lock_release(file->lock);
        return -EFAULT;
    }

    // Update offset
    file->offset = ku.uio_offset;

    // Unlock file
    lock_release(file->lock);

    // Return number of bytes read
    return buflen - ku.uio_resid;
}