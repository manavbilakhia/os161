// structure similar to the linkedlist test
//make sure to add the function signature in test.h. 
// edit menu.c
#include <types.h>
#include <lib.h>
#include <thread.h>
#include <synch.h>
#include <shared_buffer.h>
#include <test.h>


static struct semaphore *test_sem;

static void shared_buffer_test_producer(void *buff_ptr, unsigned long which)
{
    struct Shared_Buffer *buff = (struct Shared_Buffer *) buff_ptr;
    int i;

    for (i = 0; i < 10; i++) {
        char c = 'A' + (i + which * 10) % 26;
        shared_buffer_write(buff, c);
        kprintf("Producer %lu wrote: %c\n", which, c);
    }

    V(test_sem);
}

static void shared_buffer_test_consumer(void *buff_ptr, unsigned long which)
{
    struct Shared_Buffer *buff = (struct Shared_Buffer *) buff_ptr;
    int i;

    for (i = 0; i < 10; i++) {
        char c = shared_buffer_remove(buff);
        kprintf("Consumer %lu read: %c\n", which, c);
    }

    V(test_sem);
}

int shared_buffer_test_run(int nargs, char **args)
{
    (void) nargs;
    (void) args;
    test_sem = sem_create("test_sem", 0);
   
    struct Shared_Buffer *buff = shared_buffer_create(5);

    thread_fork("producer1",
                NULL,
                shared_buffer_test_producer,
                buff,
                1);

    thread_fork("producer2",
                NULL,
                shared_buffer_test_producer,
                buff,
                2);

    thread_fork("consumer1",
                NULL,
                shared_buffer_test_consumer,
                buff,
                1);
   
    thread_fork("consumer2",
                NULL,
                shared_buffer_test_consumer,
                buff,
                2);

    for (int i = 0; i < 4; i++) {
        P(test_sem);
    }

    shared_buffer_destroy(buff);
    sem_destroy(test_sem);
    return 0;
}