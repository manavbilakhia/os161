// create 
//destroy
//write char
//remove char
#include <types.h>
#include <synch.h>

struct Shared_Buffer;
void
shared_buffer_create(int buffersize)
{
    struct Shared_Buffer *buff;
    char* name = kmalloc(sizeof(char) * size)

}
void
shared_buffer_destroy(struct Shared_Buffer *buff)
{
    
}

void
shared_buffer_write(struct Shared_Buffer *buff, char c)
{

}

void
shared_buffer_remove(struct Shared_Buffer *buff)
{

}