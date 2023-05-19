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

    if (fd < 0 || fd >= MAX_FILES) {
        return -EBADF;
    }

        if (buf == NULL)
    {
        return -EFAULT;
    }

    // handle console input
    if (fd == STDIN_FILENO) {
        size_t i;
        int ch;
        char *buffer = (char *)buf;

        for (i = 0; i < buflen; i++) {
            ch = getch();

            if (ch < 0) {
                return ch; // getch() returned an error
            }

            buffer[i] = ch;

            if (ch == '\n' || ch == '\r') {
                break; // end of line
            }
        }

        return i; // return the number of characters read
    }

    // Get file from file table
    file = curproc->p_filetable->files[fd];
    if (file == NULL) {
        return -EBADF;
    }

    // Check for read permission
    if ((file->flags & O_ACCMODE) == O_WRONLY) {
        return -EBADF;
    }

    // Lock file
    KASSERT (file->lock != NULL);
    lock_acquire(file->lock);

    // Setup uio structure
    uio_kinit(&iov, &ku, buf, buflen, file->offset, UIO_READ);

    // Perform read
    int result = VOP_READ(file->vn, &ku);
    if (result) {
        lock_release(file->lock);
        return result;
    }

    // Update offset
    file->offset = ku.uio_offset;

    // Unlock file
    lock_release(file->lock);

    // Return number of bytes read
    return buflen - ku.uio_resid;
}