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
#include <copyinout.h>

ssize_t
sys_write(int fd, const void *buf, size_t nbytes){
    char *print;
    print = (char *) kmalloc(nbytes);
    int ret;
    (void) fd;
    ret = copyin((const_userptr_t) buf, print, nbytes);
    (void) ret;
    kprintf("%s", ((char *) buf));
    return nbytes;
}