#include <types.h>
#include <syscall.h>
#include <current.h>
#include <proc.h>
#include <copyinout.h>
#include <vnode.h>
#include <uio.h>
#include <kern/errno.h>
#include <kern/fcntl.h>

ssize_t
sys_read(int fd, void *buf, size_t buflen) {
    struct file *file;
    struct iovec iov;
    struct uio ku;
    int *retval;

    // Validate file descriptor
    if (fd < 0 || fd >= MAX_FILES) {
        return EBADF;
    }

    // Get file from file table
    file = curproc->p_filetable->files[fd];
    if (file == NULL) {
        return EBADF;
    }

    // Check for read permission
    if ((file->flags & O_ACCMODE) == O_WRONLY) {
        return EBADF;
    }

    // Lock file
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
    *retval = buflen - ku.uio_resid;

    return 0;
}