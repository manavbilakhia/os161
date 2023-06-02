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

#ifndef _FILE_TABLE_H_
#define _FILE_TABLE_H_

#include <types.h>
#include <vfs.h>
#include <synch.h>
#include <vnode.h>

#define MAX_FILES 20
#define MIN_FD 3

struct lock;
struct vnode;

struct file {
    struct vnode *vn;
    unsigned int offset;
    struct lock *lock;
    int flags;
    char *path;
};

struct file_table {
    struct file *files[MAX_FILES];
    int number_files;
    struct lock *lock;
};

struct file_table *ft_create(void);
void ft_destroy(struct file_table *ft);
bool ft_full(struct file_table *ft);
int file_create(struct file_table *ft, char *path, int flags, struct vnode *vn);
void file_destroy(struct file *f);
int ft_add_file(struct file_table *ft, struct file *file);
int copy_file(struct file_table *ft, int fd);
int ft_remove_file(struct file_table *ft, int fd);
int ft_look_up(struct file_table *ft, int fd);
int file_seek(struct file_table *ft, int fd);
struct file_table *ft_clone(struct file_table *ftable);
int ft_init(struct file_table *ftable);
//struct file *ft_get_file(struct file_table *ft, int fd);



#endif /* _FILE_TABLE_H_ */