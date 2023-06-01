/*
 * Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2008, 2009
 *	The President and Fellows of Harvard College.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE UNIVERSITY OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <types.h>
#include <kern/errno.h>
#include <kern/fcntl.h>
#include <lib.h>
#include <proc.h>
#include <current.h>
#include <addrspace.h>
#include <vm.h>
#include <vfs.h>
#include <syscall.h>
#include <test.h>
#include <kern/limits.h>
#include <copyinout.h>
#include <synch.h>
#include <limits.h>



// static void free_arg(char ** argv, int argc){
	// for(int j = 0; j < argc; j++){
		// kfree(argv[j]);
	// }
// }

int
sys_execv(userptr_t program, userptr_t args)
{
	int result;
    
    char *copy_program;

    int argc;
	char **kargs;
	size_t size;
	struct addrspace *as;
	struct vnode *v;
	vaddr_t entrypoint, stackptr;

	// kprintf("Check 1\n");

	/* Checking to see if the program is viable */
	if (program == NULL){
		return -EFAULT;
	}

	/* Copying in the program from the user */
	copy_program = (char *) kmalloc(PATH_MAX);
	if(copy_program == NULL){
		return -ENOMEM;
	}

	// kprintf("Check 2\n");

	result = copyinstr(program, copy_program, PATH_MAX, &size);
	if(result){
		kfree(copy_program);
		return -result;
	}
	// kprintf("PROGRAM NAME: %s\n", copy_program);

	/* Checking to see if the args are viable */
	if(args == NULL){
		kfree(copy_program);
		return -EFAULT;
	}

	/* Getting argc */
	for(argc = 0; argc < PATH_MAX; argc++){
		char * tmp;
		result = copyin(args + argc * sizeof(char*), &tmp, sizeof(char*));
		if(result){
			kfree(copy_program);
			return -result;
		}

		if(tmp == NULL){
			break;
		}
	}
	// kprintf("%d", argc);

	// kprintf("Check 3\n");
	if(argc > __ARG_MAX){
		kfree(copy_program);
		return -E2BIG;
	}

	kargs = kmalloc((argc + 1) * sizeof(char *));
	if(kargs == NULL){
		return -ENOMEM;
	}

	/* Copying in the individual pointers */
	for(int i = 0; i < argc; i++){
		kargs[i] = kmalloc(PATH_MAX);
		if(kargs[i] == NULL){
			for(int j = 0; j < i; j++){
				kfree(kargs[j]);
			}
			kfree(copy_program);
			kfree(kargs);
			return -ENOMEM;
		}

		char * tmp = NULL;
		result = copyin(args + i * sizeof(char*), &tmp, sizeof(char*));
		if(result){
			for(int j = 0; j < i; j++){
				kfree(kargs[j]);
			}
			kfree(copy_program);
			kfree(kargs);
			return -result;
		}
		// kprintf("Printing the tmp: %s\n", tmp);

		result = copyinstr((const_userptr_t) tmp, kargs[i], PATH_MAX, &size);
		if(result){
			for(int j = 0; j <= i; j++){
				kfree(kargs[j]);
			}
			kfree(copy_program);
			kfree(kargs);
			return -ENOMEM;
		}
		// kprintf("Printing the kargs: %s\n", kargs[i]);
	}

	kargs[argc] = NULL;

	//  kprintf("Check 4\n");

	result = vfs_open(copy_program, O_RDONLY, 0, &v);
	if (result) {
		// kprintf("VFS_OPEN Error\n");
		for(int i = 0; i < argc; i++){
				kfree(kargs[i]);
		}
		kfree(copy_program);
		kfree(kargs);
		return -result;
	}

	// kprintf("Before making new address space");
	/* Create a new address space. */
	as = as_create();
	if (as == NULL) {
		for(int i = 0; i < argc; i++){
			kfree(kargs[i]);
		}
		kfree(copy_program);
		kfree(kargs);
		return -ENOMEM;
	}

	// kprintf("Check 5\n");
	/* Switch to it and activate it. */
	struct addrspace *old_space = proc_setas(as);
	as_activate();

	/* Load the executable. */
	result = load_elf(v, &entrypoint);
	vfs_close(v);
	if (result) {
		/* p_addrspace will go away when curproc is destroyed */
		as_deactivate();
		as = proc_setas(old_space);
		as_destroy(as);
		for(int i = 0; i < argc; i++){
			kfree(kargs[i]);
		}
		kfree(copy_program);
		kfree(kargs);
		return -result;
	}

	/* Don't need old address space */
	as_destroy(old_space);

	// kprintf("Check 6\n");
	/* Define the user stack in the address space */
	result = as_define_stack(as, &stackptr);
	if (result) {
		/* p_addrspace will go away when curproc is destroyed */
		for(int i = 0; i < argc; i++){
			kfree(kargs[i]);
		}
		kfree(copy_program);
		kfree(kargs);
		return -result;
	}

	/* Copying from kernel to user space */
	vaddr_t *argptrs = kmalloc((argc + 1) * sizeof(vaddr_t));
	if(argptrs == NULL){
		for(int i = 0; i < argc; i++){
			kfree(kargs[i]);
		}
		kfree(copy_program);
		kfree(kargs);
		return -ENOMEM;
	}

	// kprintf("Check 7\n");
	for(int i = argc - 1; i >= 0; i--){
		size_t length = strlen(kargs[i]) + 1;
		// kprintf("Before stackptr roundup\n");
		stackptr -= ROUNDUP(length, 4);
		// kprintf("Before copyoutstr %d\n", i);
		result = copyoutstr(kargs[i], (userptr_t) stackptr, length, NULL);
		// kprintf("After copyoutstr %d\n", i);
		// kprintf("Length: %d", length);
		
		if(result){
			kprintf("This is bad\n");
			kfree(argptrs);
			for(int i = 0; i < argc; i++){
				kfree(kargs[i]);
			}
			kfree(copy_program);
			kfree(kargs);
			// kprintf("%d", result);
			return -result;
		}
		argptrs[i] = stackptr;
	}

	argptrs[argc] = (vaddr_t) NULL;

	// kprintf("Check 8\n");
	stackptr -= ROUNDUP((argc + 1) * sizeof(vaddr_t), 4);
	result = copyout(argptrs, (userptr_t) stackptr, (argc + 1) * sizeof(vaddr_t));
	if(result){
			kfree(argptrs);
			for(int i = 0; i < argc; i++){
				kfree(kargs[i]);
			}
			kfree(copy_program);
			kfree(kargs);
			return -result;
	}

	kfree(argptrs);
	for(int i = 0; i < argc; i++){
		kfree(kargs[i]);
	}
	kfree(copy_program);
	kfree(kargs);
	// kprintf("Execv Run");

	// kprintf("About to enter a new process\n");
	/* Warp to user mode. */
	enter_new_process(argc, (userptr_t) stackptr, NULL, stackptr, entrypoint);
	//add argc and userptr_t adj_stack

	/* enter_new_process does not return. */
	panic("enter_new_process returned\n");
	return -EINVAL;
}

