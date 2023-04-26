// create 
//destroy
//write char
//remove char
#include <types.h>
#include <synch.h>
#include <shared_buffer.h>

struct Shared_Buffer *shared_buffer_create(int buffersize)
{
    Shared_Buffer *buff = kmalloc(sizeof(Shared_Buffer));
    if (buff == NULL) {
        return NULL;
    }
    buff->buffer = kmalloc(sizeof(char) * buffersize);
    if (buff->buffer == NULL) {
        kfree(buff);
        return NULL;
    }
    buff->in = 0;
    buff->out = 0;
    buff->count = 0;
    buff->BUFFERSIZE = buffersize;
    buff->buffer_lock = lock_create("buffer_lock");
    buff->consumer_cv = cv_create("consumer_cv");
    buff->producer_cv = cv_create("producer_cv");

    return buff;
}
void
shared_buffer_destroy(struct Shared_Buffer *buff)
{
    lock_destroy(buff->buffer_lock);
    cv_destroy(buff->producer_cv);
    cv_destroy(buff->consumer_cv);
    kfree(buff->buffer);
    kfree(buff);
}

void
shared_buffer_write(struct Shared_Buffer *buff, char c)
{
    lock_acquire(buff->buffer_lock);
    while (buff->count == buff->BUFFERSIZE) 
    {
        cv_wait(buff->producer_cv, buff->buffer_lock);
    }
    buff->buffer[buff->in] = c;
    buff->in = (buff->in + 1) % buff->BUFFERSIZE;
    buff->count++;
    cv_signal(buff->consumer_cv, buff->buffer_lock);
    lock_release(buff->buffer_lock);
}

void
shared_buffer_remove(struct Shared_Buffer *buff)
{
    lock_acquire(buff->buffer_lock);
    while (buff->count == 0) 
    {
        cv_wait(buff->consumer_cv, buff->buffer_lock);
    }
    char c = buff->buffer[buff->out];  
    buff->out = (buff->out + 1) % buff->BUFFERSIZE;
    buff->count--;
    cv_signal(buff->producer_cv, buff->buffer_lock);
    lock_release(buff->buffer_lock);
    return c;
}