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

struct lock *lock;


static void free_arg(char ** argv, int argc){
	for(int j = 0; j < argc; j++){
		kfree(argv[j]);
	}
	kfree(argv);
}

int
sys_execv(const char *program, char **args)
{
	
	struct addrspace *as;
	struct vnode *v;
	vaddr_t entrypoint, stackptr;
	int result;
    
    char **argv;
    char *copy_program;

    int argc;

    /* Copying in the complex args */
    for(argc = 0; args[argc]; argc++);

	if(argc > __ARG_MAX){
		return -E2BIG;
	}

    argv = (char **) kmalloc(sizeof(char **) * argc);
    if(argv == NULL){
        return -ENOMEM;
    }

    for(int i = 0; i < argc; i++){
		argv[i] = kmalloc(sizeof(char *));
		if(argv[i] == NULL){
			free_arg(argv, argc);
			return -ENOMEM;
		}

        result = copyin((const_userptr_t) args[i], argv[i], (size_t) strlen(args[i]));
        if(result){
			free_arg(argv, argc);
			return -result;
        }
    }

	/* Copying in the program string */
	copy_program = (char *) kmalloc(sizeof(strlen(program)));
	if(copy_program == NULL){
		free_arg(argv, argc);
		return -ENOMEM;
	}

	result = copyin((const_userptr_t) program, copy_program, strlen(program));
	if(result){
		free_arg(argv, argc);
		kfree(copy_program);
		return -result;
	}

	/* Open the file. */
	result = vfs_open((char *) program, O_RDONLY, 0, &v);
	if (result) {
		free_arg(argv, argc);
		kfree(copy_program);
		return -result;
	}

	/* We should be a new process. */
	KASSERT(proc_getas() == NULL);

	/* Create a new address space. */
	as = as_create();
	if (as == NULL) {
		vfs_close(v);
		free_arg(argv, argc);
		kfree(copy_program);
		return -ENOMEM;
	}

	/* Switch to it and activate it. */
	proc_setas(as);
	as_activate();

	/* Load the executable. */
	result = load_elf(v, &entrypoint);
	if (result) {
		/* p_addrspace will go away when curproc is destroyed */
		vfs_close(v);
		free_arg(argv, argc);
		kfree(copy_program);
		return -result;
	}

	/* Done with the file now. */
	vfs_close(v);

	/* Define the user stack in the address space */
	result = as_define_stack(as, &stackptr);
	if (result) {
		/* p_addrspace will go away when curproc is destroyed */
		free_arg(argv, argc);
		kfree(copy_program);
		return -result;
	}

	//Need to add the stack copying here. Also need copy out at some point

	/* Warp to user mode. */
	//enter_new_process(argc /*argc*/, argv /*userspace addr of argv*/,
			  //NULL /*userspace addr of environment*/,
			  //stackptr, entrypoint);

	/* enter_new_process does not return. */
	panic("enter_new_process returned\n");
	return -EINVAL;
}

