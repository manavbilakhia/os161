#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <uio.h>
#include <proc.h>
#include <current.h>
#include <addrspace.h>
#include <vnode.h>
#include <elf.h>
#include <syscall.h>
#include <thread.h>
#include <synch.h>


int
sys_open(const char *filename, int flags)
{
    struct file_table *ftable =curproc ->p_filetable; //ask matt how to get this to work
    struct vnode *v; 
    struct file *file;
    int result; // to store return value from vfs_open
    int *retval;

    // opening the filetable using vfs
    result  = vfs_open((char *)filename, flags, 0, &v);
    if (result)
    {
        return result; // this is the file handle that goes to read, write, close
    }

    //creating a new file struct
    file = file_create(ftable);
    if (file == NULL)
    {
        vfs_close(v);
        return ENFILE;
    }
    // setting vnode and other attribute in the file strruct

    file->vn = v;
    file->offset= 0;
    file->refcount = 1;

    ft_add_file(ftable, file); // adding file to file table

    //seting the file descriptor as the return value
    *retval = file->fd;

    return 0;

}