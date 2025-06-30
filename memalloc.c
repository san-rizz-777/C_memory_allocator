#include<stdio.h>   //For standard input output
#include<pthread.h>   //Global locking mechanism
#include<unistd.h>   //for sbrk
#include<string.h>    //For string manupulations
#include<stdint.h>


typedef char ALIGN[16];    //for alignment of header to actual memory block

union header{
    struct{
        size_t size;    //For storing the size
        unsigned is_free;   //For indication of is free or not
        union header *next;
    }s;

    //force the header to align to 16 bytes

    ALIGN stub;
};

typedef union header header_t;

header_t *head = NULL;   //for tracking the linked lists
header_t *tail = NULL;
pthread_mutex_t global_malloc_lock;    //Locking before doing any memory operations to avoid data races

//Defining the get_free_block function to check for avaiable free chunks instead asking to OS
header_t* get_free_block(size_t size)
{
    header_t *curr = head;

    while(curr)
    {
        if(curr->s.is_free && curr->s.size == size)    //Checking if we have free chunk of sufficient size
        {
            return curr;
        }
        curr = curr->s.next;
    }

    //if not found return null pointer
 return NULL;
}

void* malloc(size_t size)
{
    size_t total_size;
    void *block;
    header_t *header;

    write(2, "Heyyyy!!I am using malloc!!\n", 14);

    if(!size)  return NULL;    //Checking if size requirement is nothing

    pthread_mutex_lock (&global_malloc_lock);  //Locking before the operations
    header = get_free_block(size);

    if(header)
    {
        /* Woah, found a free block to accomodate requested memory. */
        header->s.is_free = 0;
        pthread_mutex_unlock(&global_malloc_lock);   //Unlocking
        return (void*)(header+1);    //As keeping the header hidden return pointer pointing to next right to end of header
    }

    total_size = sizeof(header_t) + size;   //Size of the header + actual memory requirement
    block = (void*)sbrk(total_size);
    //On failure sbrk( ) returns (void*)-1
    if(block == (void*)-1)  {
        pthread_mutex_unlock(&global_malloc_lock);
    return NULL;}

    header = (header_t*)block;  //type cast to header_t

    //Updating the size and free,next,head and tail pointers
    header->s.size = size;
    header->s.is_free = 0;
    header->s.next = NULL;
    if(!head) head = header;
    if(tail)  tail->s.next = header;
    tail = header;
    pthread_mutex_unlock(&global_malloc_lock);   //Unlocking the lock
    
    return (void*)(header+1);    //Hiding the header and returning the actual memory block
}


void free(void *block)
{
    header_t *header, *tmp; 
    void *programBreak;

    if(!block)  return;   //if pointer is not pointing to any memory
    pthread_mutex_lock(&global_malloc_lock);

    header = (header_t*)(block -1 );      //Initially it was point to start of actual size now we got our header
/* program break is the end of the process's data segment */
    programBreak = (void*)sbrk(0);   //Points to end of heap now

    //Checking if end of the header+actual memory block is same as end of heap
    
	/*
	   Check if the block to be freed is the last one in the
	   linked list. If it is, then we could shrink the size of the
	   heap and release memory to OS. Else, we will keep the block
	   but mark it as free.
	 */
    if((char*)block + header->s.size == programBreak)
    {
        if(head==tail)
        {
            head=tail=NULL;
        }
        else{
            while(tmp){
            tmp = head;
            if(tmp->s.next==tail)
            {
                tail->s.next = NULL;
                tail = tmp;
            }
            tmp = tmp->s.next;
            }
        }
         /*
		   sbrk() with a negative argument decrements the program break.
		   So memory is released by the program to OS.
		*/

        sbrk(0 - header->s.size - sizeof(header_t));

        /* Note: This lock does not really assure thread
		   safety, because sbrk() itself is not really
		   thread safe. Suppose there occurs a foregin sbrk(N)
		   after we find the program break and before we decrement
		   it, then we end up realeasing the memory obtained by
		   the foreign sbrk().
		*/
    pthread_mutex_unlock(&global_malloc_lock);
    return;
    }

    header->s.is_free=1;    //If not at the  end of heap just mark it as free
    pthread_mutex_unlock(&global_malloc_lock);
}

void *calloc(size_t num, size_t nsize)
{
    //num is size of array and nsize is size of indivual element in it
       size_t size;
       void* block;

       if(!num || !nsize)  return NULL;

       size = nsize*num;
//Checking for multiplicative overflow
       if(nsize!=size/num)  return NULL;
       block = malloc(size);            //getting the pointer from malloc
       if(!block)  return NULL;
//The memory is all set to zero
       memset(block,0,size);

       return block;
}

// realloc() changes the size of the given memory block to the size given.
void *realloc(void* block, size_t size)
{
    header_t *header;
    void* ret;

    if(!block || !size)     return malloc(size);

    header = (header_t*)(block-1);    //Pointing towards the start of the header 

    if(header->s.size>=size)   return block;    //If my block already has the size to accomodate the requested size return the block

    //if not then allocate the memory by malloc and reallocate the contents to bigger memory block
    ret = malloc(size);

    if(ret)
    {
        memcpy(ret,block,header->s.size);    //Copy all the contents of block to ret
        free(block);    //Now free the existing memory block 
    }
    
    return ret;
}

/* A debug function to print the entire link list */

void print_mem_list() {
	header_t *curr = head;
	char buffer[256];
	int len;

	// Print head and tail pointers
	len = snprintf(buffer, sizeof(buffer),
	               "head = %p, tail = %p\n", (void*)head, (void*)tail);
	write(2, buffer, len);

	while (curr) {
		len = snprintf(buffer, sizeof(buffer),
		               "addr = %p, size = %zu, is_free=%u, next=%p\n",
		               (void*)curr, curr->s.size, curr->s.is_free, (void*)curr->s.next);
		write(2, buffer, len);
		curr = curr->s.next;
	}
}

