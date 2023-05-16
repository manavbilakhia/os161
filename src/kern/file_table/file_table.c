/*
 * Copyright (c) 2013
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

/*
 * Process support.
 *
 * There is (intentionally) not much here; you will need to add stuff
 * and maybe change around what's already present.
 *
 * p_lock is intended to be held when manipulating the pointers in the
 * proc structure, not while doing any significant work with the
 * things they point to. Rearrange this (and/or change it to be a
 * regular lock) as needed.
 *
 * Unless you're implementing multithreaded user processes, the only
 * process that will have more than one thread is the kernel process.
 */

#include <types.h>
#include <spl.h>
#include <proc.h>
#include <current.h>
#include <addrspace.h>
#include <vnode.h>
#include <file_table.h>
#include <synch.h>

struct file_table *ftable;

struct file_table *ft_create(void){
    ftable = kmalloc(sizeof(struct file_table));
    
    if(ftable == NULL){
        return NULL;
    }

    ftable -> lock = lock_create("file_table_lock");
    if(ftable -> lock == NULL){
        ft_destroy(ftable);
        return NULL;
    }

    ftable -> number_files = 0;
    return ftable;
}

void ft_destroy(struct file_table *ftable){
    KASSERT(ftable != NULL);

    for(int i = 0; i < MAX_FILES; i++){
        file_destroy(ftable -> files[i]);
    }
    lock_destroy(ftable -> lock);
    kfree(ftable);
}

bool ft_full(struct file_table *ftable){
    return (ftable -> number_files == MAX_FILES);
}

/**
 * Creates a file atomic
*/
struct file *file_create(struct file_table *ftable){
    KASSERT(!ft_full(ftable));
    lock_acquire(ftable -> lock);

    struct file *file = kmalloc(sizeof(struct file));
    if (file == NULL){
        return NULL;
    }

    file -> lock = lock_create("file_lock");
    if(file -> lock == NULL){
        kfree(file);
        return NULL;
    }

    file -> vn = NULL;
    file -> offset = 0;
    file -> refcount = 1;

    ftable -> number_files++; //will need to add file to the file table
    
    lock_release(ftable -> lock);
    return file;
}

/**
 * Destroys a file and decrements the ref counts
*/
void file_destroy(struct file *file){
    KASSERT(file != NULL);

    lock_destroy(file -> lock);
    if(file -> vn != NULL){
        vfs_close(file -> vn);
    }

    file -> refcount--;

    kfree(file);
}

void ft_add_file(struct file_table *ftable, struct file *file){
    KASSERT(ftable != NULL);
    lock_acquire(ftable -> lock);
    int i = 0;

    while(ftable -> files[i] != NULL){
        i++;
    }

    ftable -> files[i] = file;
    lock_release(ftable -> lock);
}

struct file *copy_file(struct file_table *ftable){
    KASSERT(ftable != NULL);
    lock_acquire(ftable -> lock);
    struct file *copy = file_create(ftable);
    copy -> refcount++;

    lock_release(ftable -> lock);
    return copy;
}