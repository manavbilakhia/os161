#include <types.h>
#include <proc.h>
#include <current.h>
#include <vnode.h>
#include <file_table.h>
#include <uio.h>
#include <syscall.h>
#include <errmsg.h>

ssize_t
sys_read(int fd, void *buf, size_t buflen) {
    struct file_table *ftable = curproc->p_filetable;
    struct file *file;
    int result;
    int *retval;
    
    // Check if the file descriptor is within a valid range
    if (fd < 0 || fd >= MAX_FILES)
        return EBADF;
    
    // Check if the file is open in the file table
    file = ftable->files[fd];
    if (file == NULL)
        return EBADF;
    
    // Acquire the lock for the file
    lock_acquire(file->lock);
    
    // Create a uio structure for reading into the buffer
    struct uio u;
    uio_kinit(&u, buf, buflen, file->offset, UIO_READ);
    
    // Read from the file using VOP_READ
    result = VOP_READ(file->vn, &u);
    if (result) {
        lock_release(file->lock);
        return result;
    }
    
    // Update the offset based on the number of bytes read
    file->offset += u.uio_offset - file->offset;
    
    // Set the number of bytes read as the return value
    *retval = u.uio_offset - file->offset;
    
    // Release the lock for the file
    lock_release(file->lock);

    return 0;
}