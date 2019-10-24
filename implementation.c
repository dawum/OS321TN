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
#define GET_NEXT(p) ((ll *)(p))->prev


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

/* Your helper functions 

   You may also put some struct definitions, typedefs and global
   variables here. Typically, the instructor's solution starts with
   defining a certain struct, a typedef and a global variable holding
   the start of a linked list of currently free memory blocks. That 
   list probably needs to be kept ordered by ascending addresses.

*/

// struct for a LL to hold freed memory
struct linkedList {
	size_t length;
	void * mmap_start;
	size_t mmap_size; 
	void * next;
  void * prev;	
}; typedef struct linkedList ll;

#define SL sizeof(ll)
// initialize LL to hold free memory
//ll * head = NULL;


/* End of your helper functions */

/* Start of the actual malloc/calloc/realloc/free functions */

void __free_impl(void *);
void *__malloc_impl(size_t size);
// We need to return a ptr to the region starting where the head ends
void *__malloc_impl(size_t size) {
		// check if theres a space available in the free_memory LL and use from there first if possible
		// if not, create a new space with mmap.
    
    // not certain if mmap_start is what we want here and size + sizeof(ll) isn't exactly how it should look 
    // but its general indea. Rest of mmap call im 99% cetain is correct.
    if (size < 1)
    {
      return NULL;
    }
    else if (size >9223372036854775807)
    {
      return NULL;
    }
    else
    {
      void * temp = mmap(NULL, size + sizeof(ll), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS , -1, 0);
      if (temp == NULL) return NULL;
      GET_MMPSIZE(temp) = size+sizeof(ll);
      temp += sizeof(ll);
  return temp;
    }
}

void *__calloc_impl(size_t nmemb, size_t size) {
  /* STUB */
  size_t aa = 0;
  size_t * a = &aa;
   __try_size_t_multiply(a,size,nmemb);

  if (nmemb < 1 || size < 1 || *a >9223372036854775807 )
  {
  return NULL; 
  }

  else{
   void * temp = __malloc_impl(*a);
   __memset(temp,0,*a);
   temp += SL;
  

  return temp;  
  }
}

void *__realloc_impl(void *ptr, size_t size) {
  /* STUB */
  if (ptr)
  {
  ptr -= sizeof(ll);
  if (size == 0)
  {
    ptr += sizeof(ll);
    __free_impl(ptr);
    return NULL;
  }
  else if (size < 0)
  {
    ptr += SL;
    __free_impl(ptr);
    return NULL;
  }
  else if (size >9223372036854775807)
  {
    ptr += SL;
    __free_impl(ptr);
    return NULL;
  }
  else if (size == GET_MMPSIZE(ptr)-sizeof(ll))
  {
    ptr+=SL;
    return ptr;
  }
  else if (size < GET_MMPSIZE(ptr)-sizeof(ll)) // realloc to smaller size
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
    ptr += sizeof(ll);
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

// whenever free is called, munmap the *ptr using its ptr->length. Then, add it
// to the free_memory LL. Whenever a memory is allocated we first check this LL 
// for a space >= whats required. If we find one we use it and remove it from the 
// free_memory LL. If not, we create a new entry to the free_memory LL and add it at the end.
// 
void __free_impl(void *ptr) {

/* 1. Push into a linked list.
   2. sort linked list.
   3. merge blocks.
   4. if all blocks are returned free.

*/
if (ptr)
{
ptr -= sizeof(ll);
munmap(ptr,GET_MMPSIZE(ptr));
}
}

/* Push newly mmaped block or freed block to linked list. */
void __push(void *ptr){
}
/* Merge blocks into bigger chunks. */
void __searchAndMerge(){

}

/* If a block is complete, unmap. */
void __unmap(void *ptr){
}






