#ifndef _SHARED_BUFFER_H_
#define _SHARED_BUFFER_H_

#include <types.h>
#include <synch.h>

typedef struct Shared_Buffer Shared_Buffer;

struct Shared_Buffer{
    char *buffer;
    int in;
    int out;
    int count;
    int BUFFERSIZE;
    struct lock *buffer_lock;
    struct cv *consumer_cv;
    struct cv *producer_cv;
};

struct Shared_Buffer *shared_buffer_create(int buffersize);

void
shared_buffer_destroy(struct Shared_Buffer *buff);

void
shared_buffer_write(struct Shared_Buffer *buff, char c);

void
shared_buffer_remove(struct Shared_Buffer *buff);

#endif