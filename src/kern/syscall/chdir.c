#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <current.h>
#include <thread.h>
#include <vfs.h>
#include <syscall.h>
#include <uio.h>
#include <proc.h>
#include <addrspace.h>
#include <vnode.h>
#include <elf.h>
#include <thread.h>
#include <synch.h>
#include <limits.h>
#include <copyinout.h>

int
sys_chdir(const char *pathname)
{
    char *kern_pathname = (char *)kmalloc(sizeof(char)*(PATH_MAX+1));
    size_t actual_len;
    int result;

    result = copyinstr((const_userptr_t)pathname, kern_pathname, PATH_MAX+1, &actual_len);
    if (result) {
        kfree(kern_pathname);
        return result;
    }

    result = vfs_chdir(kern_pathname);
    kfree(kern_pathname);
    return result;
}