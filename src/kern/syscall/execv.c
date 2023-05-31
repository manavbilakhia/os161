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

    int argc = 0;
	char **argv = kmalloc(PATH_MAX);
	size_t size = 0;
	userptr_t kargs[PATH_MAX];

	kprintf("starting\n");
    /* Copying in the complex args */

	result = copyin(args, argv, 4);
	while(argv[argc] != NULL){
		result = copyin((const_userptr_t) argv[argc], &kargs[argc], 4);
		if(result){
			return -result;
		}
		argc++;
	}

	kprintf("Out of while. %d", argc);

	if(argc > __ARG_MAX){
		return -E2BIG;
	}


    for(int i = 0; i < argc; i++){
        result = copyinstr((const_userptr_t) kargs[i], (char *) argv[i], PATH_MAX, &size);
        if(result){
			kprintf("uh oh\n");
			return -result;
        }
		kprintf("%s\n", argv[i]);
    }

	kprintf("out of loop\n");
	/* Copying in the program string */
	copy_program = (char *) kmalloc(PATH_MAX);
	if(copy_program == NULL){
		kfree(copy_program);
		return -ENOMEM;
	}

	result = copyinstr((const_userptr_t) program, copy_program, PATH_MAX, &size);
	if(result){
		kfree(copy_program);
		return -result;
	}

	return 0;
}

