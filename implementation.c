
/*  
  good example to look at
  https://embeddedartistry.com/blog/2017/2/9/implementing-malloc-first-fit-free-list#prerequisites=
    Copyright 2018-19 by
    University of Alaska Anchorage, College of Engineering.
    All rights reserved.
    Contributors:  ...
		   ...                 and
		   Christoph Lauter
    See file memory.c on how to compile this code.
    Implement the functions __malloc_impl, __calloc_impl,
    __realloc_impl and __free_impl below. The functions must behave
    like malloc, calloc, realloc and free but your implementation must
    of course not be based on malloc, calloc, realloc and free.
    Use the mmap and munmap system calls to create private anonymous
    memory mappings and hence to get basic access to memory, as the
    kernel provides it. Implement your memory management functions
    based on that "raw" access to user space memory.
    As the mmap and munmap system calls are slow, you have to find a
    way to reduce the number of system calls, by "grouping" them into
    larger blocks of memory accesses. As a matter of course, this needs
    to be done sensibly, i.e. without wasting too much memory.
    You must not use any functions provided by the system besides mmap
    and munmap. If you need memset and memcpy, use the naive
    implementations below. If they are too slow for your purpose,
    rewrite them in order to improve them!
    Catch all errors that may occur for mmap and munmap. In these cases
    make malloc/calloc/realloc/free just fail. Do not print out any 
    debug messages as this might get you into an infinite recursion!
    Your __calloc_impl will probably just call your __malloc_impl, check
    if that allocation worked and then set the fresh allocated memory
    to all zeros. Be aware that calloc comes with two size_t arguments
    and that malloc has only one. The classical multiplication of the two
    size_t arguments of calloc is wrong! Read this to convince yourself:
    https://cert.uni-stuttgart.de/ticker/advisories/calloc.en.html
    In order to allow you to properly refuse to perform the calloc instead
    of allocating too little memory, the __try_size_t_multiply function is
    provided below for your convenience.
    
*/

/* Macros */

#define GET_LENGTH(p) ((ll *)(p))->length
#define GET_MMPSTART(p) ((ll *)(p))->mmap_start
#define GET_MMPSIZE(p) ((ll *)(p))->mmap_size
#define GET_NEXT(p) ((ll *)(p))->next
#define GET_PREV(p) ((ll *)(p))->prev

#include <stddef.h>
#include <sys/mman.h>
#include <stdlib.h>

/* Predefined helper functions */

static void *__memset(void *s, int c, size_t n) {
  unsigned char *p;
  size_t i;

  if (n == ((size_t) 0)) return s;
  for (i=(size_t) 0,p=(unsigned char *)s;
       i<=(n-((size_t) 1));
       i++,p++) {
    *p = (unsigned char) c;
  }
  return s;
}

static void *__memcpy(void *dest, const void *src, size_t n) {
  unsigned char *pd;
  const unsigned char *ps;
  size_t i;

  if (n == ((size_t) 0)) return dest;
  for (i=(size_t) 0,pd=(unsigned char *)dest,ps=(const unsigned char *)src;
       i<=(n-((size_t) 1));
       i++,pd++,ps++) {
    *pd = *ps;
  }
  return dest;
}

/* Tries to multiply the two size_t arguments a and b.
   If the product holds on a size_t variable, sets the 
   variable pointed to by c to that product and returns a 
   non-zero value.
   
   Otherwise, does not touch the variable pointed to by c and 
   returns zero.
   This implementation is kind of naive as it uses a division.
   If performance is an issue, try to speed it up by avoiding 
   the division while making sure that it still does the right 
   thing (which is hard to prove).
*/
static int __try_size_t_multiply(size_t *c, size_t a, size_t b) {
  size_t t, r, q;

  /* If any of the arguments a and b is zero, everthing works just fine. */
  if ((a == ((size_t) 0)) ||
      (a == ((size_t) 0))) {
    *c = a * b;
    return 1;
  }

  /* Here, neither a nor b is zero. 
     We perform the multiplication, which may overflow, i.e. present
     some modulo-behavior.
  */
  t = a * b;

  /* Perform Euclidian division on t by a:
     t = a * q + r
     As we are sure that a is non-zero, we are sure
     that we will not divide by zero.
  */
  q = t / a;
  r = t % a;

  /* If the rest r is non-zero, the multiplication overflowed. */
  if (r != ((size_t) 0)) return 0;

  /* Here the rest r is zero, so we are sure that t = a * q.
     If q is different from b, the multiplication overflowed.
     Otherwise we are sure that t = a * b.
  */
  if (q != b) return 0;
  *c = t;
  return 1;
}

/* End of predefined helper functions */




// Start of user defined helper functions


// struct for a LL to hold freed memory
typedef struct ll{
    size_t length;
    void * mmap_start;
    size_t mmap_size;
    void * next;
    void * prev;
} ll;

#define SL sizeof(ll) //Size of LL
#define BYTE sizeof(char) // 1 byte

// initialize LL to hold free memory
ll * head = NULL;

void __free_impl(void *ptr);

// search the global LL for a large enough chunk of memory. If found, return that space and add 
// leftover to the global LL if that leftover is large enough to hold the header.
static void * searchLL(size_t size){
    ll * temp1 = head;

    // check if head is NULL (LL is empty)
    if (temp1 == NULL)
    {
	return NULL;
    }
    while (temp1 != NULL)
    {
	if ((GET_LENGTH(temp1) - SL) >= size) // temp1 is large enough to provide space requested
	{
	    // If there isn't enough space to split up block of memory, just return it and be done.
	    if ((GET_LENGTH(temp1) - SL) - size < SL + 1) // there isn't space for header, don't create a new block
	    {
		// remove temp1 from LL and return temp1 with potential slight excess
		if (GET_NEXT(temp1) == NULL && GET_PREV(temp1) == NULL) // temp1 is only member of LL
		{
		    head = NULL;
		    return temp1;
		}
		else if (GET_PREV(temp1) == NULL) //temp1 is head of LL in this case
		{
		    head = GET_NEXT(temp1);
		    GET_PREV(head) = NULL;
		    return temp1;
		}

		else if(GET_NEXT(temp1) == NULL) //temp1 is at tail
		{
		    GET_NEXT(GET_PREV(temp1)) = NULL;
		    return temp1;
		}
		else //temp1 is somewhere in middle of LL in this case
		{
		    GET_NEXT(GET_PREV(temp1)) = GET_NEXT(temp1);
		    GET_PREV(GET_NEXT(temp1)) = GET_PREV(temp1);
		    return temp1;
		}
	    }
	    // there is space for a header in excess chunk of memory. Create a new block, temp2, and add it to LL
	    // Then, remove temp1 from LL, resize temp1, return temp1
	    else
	    {
		// create new chunk of leftover to be inserted back into LL, resize temp1
		ll * temp2;
		char * temp3;
		temp3 = (char *)temp1 + SL + size + 1; //place pointer for temp2 at end of space that will be used from temp1
		temp2 = (ll *)temp3;
		GET_LENGTH(temp2) = GET_LENGTH(temp1) - SL - size; // make temp1's length = exactly whats requested
		GET_MMPSTART(temp2) = GET_MMPSTART(temp1);
		GET_MMPSIZE(temp2) = GET_MMPSIZE(temp1);
		GET_LENGTH(temp1) = SL + size;
		// Check if at tail or head
		if (GET_NEXT(temp1) == NULL && GET_PREV(temp1) == NULL) // temp1 is the only value in LL (and head)
		{
		    GET_NEXT(temp2) = NULL;
		    GET_PREV(temp2) = NULL;
		    head = temp2;
		}
		else if(GET_NEXT(temp1) == NULL) // temp1 is tail
		{
		    GET_NEXT(temp2) = NULL;
		    GET_NEXT(GET_PREV(temp1)) = temp2;
		    GET_PREV(temp2) = GET_PREV(temp1);
		}
		else if (GET_PREV(temp1) == NULL) // temp1 is head
		{
		    GET_PREV(temp2) = NULL;
		    GET_NEXT(temp2) = GET_NEXT(temp1);
		    GET_PREV(GET_NEXT(temp1)) = temp2;
		    head = temp2;

		}
		else // temp1 is somewhere in middle of LL
		{
		    GET_NEXT(temp2) = GET_NEXT(temp1);
		    GET_PREV(temp2) = GET_PREV(temp1);
		    GET_NEXT(GET_PREV(temp1)) = temp2;
		    GET_PREV(GET_NEXT(temp1)) = temp2;
		}
		// Null out temp1's prev and next values
		GET_NEXT(temp1) = NULL;
		GET_PREV(temp1) = NULL;
		return temp1;
	    }
	}
    temp1 = GET_NEXT(temp1);
    }
    // If reached here than no space large enough was found in the LL, return NULL
    temp1 = NULL;
    return temp1;
}

// check if we can munmap an entire block
static void tryMunmap(ll *ptr)
{
    if (GET_MMPSTART(ptr) == ptr) // If given ptr is start of mmaping and an entire block, remove from LL and munmap it
    {
	if (GET_LENGTH(ptr) == GET_MMPSIZE(ptr))
	{
	    if (GET_NEXT(ptr) == NULL && GET_PREV(ptr) == NULL) // ptr is only member of LL
	    {
         	head = NULL;
	        munmap(ptr, GET_MMPSIZE(ptr));
		return;
	    }
	    else if (GET_PREV(ptr) == NULL) // ptr is head of LL in this case
	    {
      	        head = GET_NEXT(ptr);
		GET_PREV(head) = NULL;
   	        munmap(ptr, GET_MMPSIZE(ptr));
         	return;
	    }
     	    else if(GET_NEXT(ptr) == NULL) // ptr is at tail
	    {
		GET_NEXT(GET_PREV(ptr)) = NULL;
 	        munmap(ptr, GET_MMPSIZE(ptr));
		return;
	    }
	    else // ptr is somewhere in middle of LL in this case
	    {
    	        GET_NEXT(GET_PREV(ptr)) = GET_NEXT(ptr);
		GET_PREV(GET_NEXT(ptr)) = GET_PREV(ptr);
		munmap(ptr, GET_MMPSIZE(ptr));
		return;
	    }
	}
    }
    // Can't munmap ptr
    return;
}



/* Push newly mmaped block or freed block to linked list and merge it if possible */
static void push(ll * ptr)
{
//    ptr -= SL;  // move ptr to proper position
    ll * temp = head;
    if (temp == NULL)
    {
	head = ptr;
	return;
    }
    else
    {
        while (temp != NULL)
        {
	    // find proper position in free memory LL to insert given ptr into
	    if (ptr < temp) //insert ptr to the left of temp (which has to be head) and reassign head to ptr
	    {
	        GET_NEXT(ptr) = temp;
	        GET_PREV(ptr) = NULL;
	        GET_PREV(temp) = ptr;
	        head = ptr;
   	        return;
	    }
	    else // (ptr > temp)
  	    {
	        if (GET_NEXT(temp) == NULL) // if at tail, insert ptr at the end of free memory LL
	        {
	            GET_NEXT(temp) = ptr;
		    GET_NEXT(ptr) = NULL;
		    GET_PREV(ptr) = temp;
		    return;
	        }
	        else // (temp->next != NULL)
	        {
	            if ((void *)ptr < GET_NEXT(temp)) // insert ptr inbetween temp and temp->next
	            {
		        GET_NEXT(ptr) = GET_NEXT(temp);
		        GET_PREV(ptr) = temp;
	                GET_NEXT(temp) = ptr;
		        GET_PREV(GET_NEXT(ptr)) = ptr;
		        return;
	            }
	        }
	    }

        // (ptr > temp->next), traverse further down the free memory LL
        temp = GET_NEXT(temp);
        }
    }
}



// After pushing to proper position, attempt to merge with surrounding nodes if possible
static void merge(ll * ptr)
{
//    ptr = (char *) ptr; // cast ptr to a char ptr so we can increment by one byte at a time
    if (ptr == NULL) // ptr is NULL
    {
	return;
    }
    else if (GET_PREV(ptr) == NULL && GET_NEXT(ptr) == NULL) // ptr is only value in LL
    {
	tryMunmap(ptr);
	return;
    }
    else if (GET_PREV(ptr) == NULL) // at head of LL, just check if we can merge with right node
    {
	//check if one starts where the other ends and they are part of the same mmapping
	if (((void *)ptr + (GET_LENGTH(ptr)) + BYTE) == GET_NEXT(ptr) && GET_MMPSTART(ptr) == GET_MMPSTART(GET_NEXT(ptr)))
	{
	    //merge current node with next node
	    GET_LENGTH(ptr) = (GET_LENGTH(ptr) + GET_LENGTH(GET_NEXT(ptr)));
	    GET_NEXT(ptr) = (GET_NEXT(GET_NEXT(ptr)));
	    tryMunmap(ptr);
	    return;
	}
    }
    else if (GET_NEXT(ptr) == NULL) // at tail of LL, just check if ptr can merge with left node
    {
	if ((GET_PREV(ptr) + (GET_LENGTH(GET_PREV(ptr)) + BYTE)) == (void *)ptr && GET_MMPSTART(GET_PREV(ptr)) == GET_MMPSTART(ptr))
	{
	    //merge left node with current node
	    GET_LENGTH(GET_PREV(ptr)) = (GET_LENGTH(GET_PREV(ptr)) + GET_LENGTH(ptr));
	    GET_NEXT(GET_PREV(ptr)) = NULL;
	    ptr = GET_PREV(ptr);
	    tryMunmap(ptr);
	    return;
	}
    }

    else // ptr is not at head or end of LL
    {
	// check if we can merge with both left and right node
	if ((GET_PREV(ptr) + GET_LENGTH(GET_PREV(ptr) + BYTE)) == (void *)ptr && GET_MMPSTART(GET_PREV(ptr)) == GET_MMPSTART(ptr) && (ptr + GET_LENGTH(ptr) + BYTE) == GET_NEXT(ptr) && GET_MMPSTART(ptr) == GET_MMPSTART(GET_NEXT(ptr)))
	{
	    GET_LENGTH(GET_PREV(ptr)) = (GET_LENGTH(GET_PREV(ptr)) + GET_LENGTH(ptr) + GET_LENGTH(GET_NEXT(ptr)));
	    GET_NEXT(GET_PREV(ptr)) = GET_NEXT(GET_NEXT(ptr));
	    ptr = GET_PREV(ptr);
	    tryMunmap(ptr);
	    return;
	}
	// check if we can merge with left node
	else if ((GET_PREV(ptr) + GET_LENGTH(GET_PREV(ptr)) + BYTE) == (void *)ptr && GET_MMPSTART(GET_PREV(ptr)) == GET_MMPSTART(ptr))
	{
	    //merge left node with current node
	    GET_LENGTH(GET_PREV(ptr)) = (GET_LENGTH(GET_PREV(ptr)) + GET_LENGTH(ptr));
	    GET_NEXT(GET_PREV(ptr)) = GET_NEXT(ptr);
	    ptr = GET_PREV(ptr);
	    tryMunmap(ptr);
	    return;

	}
	// check if we can merge with right node
	else if(((void *)ptr + GET_LENGTH(ptr) + BYTE) == GET_NEXT(ptr) && GET_MMPSTART(ptr) == GET_MMPSTART(GET_NEXT(ptr)))
	{
	    //merge current node with next node
	    GET_LENGTH(ptr) = (GET_LENGTH(ptr) + GET_LENGTH(GET_NEXT(ptr)));
	    GET_NEXT(ptr) = (GET_NEXT(GET_NEXT(ptr)));
	    tryMunmap(ptr);
	    return;
	}
	else // can't merge with either left or right node
	{
	    return;
	}
    }
}

/* End of your helper functions */



/* Start of the actual malloc/calloc/realloc/free functions */

//void __free_impl(void * ptr);
//void *__malloc_impl(size_t size);
// We need to return a ptr to the region starting where the head ends
void *__malloc_impl(size_t size) {
    // check if theres a space available in the free_memory LL and use from there first if possible
    // if not, create a new space with mmap.
    void * temp1;
//    ll * temp2;

    if (size == (size_t) 0)
    {
        return NULL;
    }
    else if (size > 9223372036854775807)
    {
        return NULL;
    }
    else
    {
        // Search free memory LL for a space large enough to fit the size requested
        temp1 = searchLL(size);

        // If we cant find a large enough preexisting space in the free memory LL, mmap in a new space & add it to LL
        if (temp1 == NULL)
        {
            temp1 = mmap(NULL, size + SL, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS , -1, 0);
            if (temp1 == NULL)
	    {
	        return NULL; // mmap failed to find space requested
	    }
//	    temp2 = (ll *) temp1;
//	    temp1 = *(struct ll *)temp1;
	    GET_MMPSIZE(temp1) = size + SL;
	    GET_MMPSTART(temp1) = temp1;
	    GET_LENGTH(temp1) = size + SL;

	    push(temp1);
    	    // Search free memory LL for a space large enough to fit the size requested
    	    temp1 = searchLL(size);
//	    temp2 = (ll *) temp1;
            if (temp1 == NULL)
	    {
	        return NULL; // mmap failed to find space requested
	    }
//	    temp3 = (char *)temp2 + SL;
//	    temp2 = (ll *)temp3;
//	    temp2 += SL;
	    temp1 += SL;
	    return temp1;
        }
        // We were able to find a large enough preexisting space in free memory LL
        else
        {
	    temp1 += SL;
	    return temp1;
	}
    }
}


void *__calloc_impl(size_t nmemb, size_t size)
{
    size_t aa = 0;
    size_t * a = &aa;
    __try_size_t_multiply(a,size,nmemb);

    if (nmemb < 1 || size < 1 || *a >9223372036854775807 )
    {
        return NULL;
    }

    else
    {
        void * temp1 = __malloc_impl(*a);
//	ll * temp2;
//	temp2 = (ll *) temp1;
        __memset(temp1,0,*a);

        return temp1;
    }
}


void *__realloc_impl(void *ptr, size_t size)
{
    if (ptr)
    {
        ptr -= SL;
        if (size == 0)
	{
            ptr += SL;
            __free_impl(ptr);
   	    return NULL;
        }
        else if (size < (size_t)0)
  	{
   	    ptr += SL;
   	   __free_impl(ptr);
  	    return NULL;
  	}
 	else if (size > (size_t)9223372036854775807)
        {
            ptr += SL;
            __free_impl(ptr);
            return NULL;
 	}
        else if (size == GET_MMPSIZE(ptr) - SL)
        {
            ptr+=SL;
            return ptr;
        }
        else if (size < GET_MMPSIZE(ptr) - SL) // realloc to smaller size
        {
            void * temp = __malloc_impl(size);
            ptr += SL;
            __memcpy(temp,ptr,size);
            __free_impl(ptr);
            return temp;
        }
        else // realloc to larger size
        {
            void * temp = __malloc_impl(size);
   	    ptr += SL;
            void * tempp = ptr - SL;
            __memcpy(temp,ptr,GET_MMPSIZE(tempp)-SL);
            __free_impl(ptr);
            return temp;
        }
    }
    else
    {
        return NULL;
    }
}


void __free_impl(void * ptr)
{
/* 1. Push into a linked list.
   2. sort linked list.
   3. merge blocks.
   4. if all blocks are returned free.
32*/
    if (ptr == NULL)
    {
        return;
    }
    else
    {
//if (ptr)
//{
//ptr -= sizeof(ll);
//munmap(ptr,GET_MMPSIZE(ptr));
//}
    ptr -= SL;
//    ll * temp;
//    temp = (ll *)&ptr;
    push(ptr);
    merge(ptr);
    return;
    }

}





