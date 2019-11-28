#include <stddef.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <libgen.h>

#define GETPTR(p,o) ((void *)(p))+o 
#define GETOFFSET(p,o) (void *) o - (void *) p 
#define MAGICNUMBER ((uint32_t) 22222222)
#define GET_LENGTH(p) ((ll *)(p))->length
#define GET_NEXT(p) ((ll *)(p))->next
#define GET_PREV(p) ((ll *)(p))->prev

#define SL sizeof(ll) // Size of LL
#define BYTE sizeof(char) // 1 byte


// struct to hold directories and file metadata
typedef struct node{
	char * name;
//	size_t parent;		//location of parent node in LL structure of nodes
//	size_t next;		//location of next node in LL structure of nodes
	size_t size;		//size of file to be stored, NULL if a directory node
	size_t offset;		//location of this node in the fileSystem
	size_t fileLocation; //location of file to be stored, NULL if a directory node
	int *subdirs;        // Subdir count (unused if not is_dir)
} nd;
//typedef struct node nd;


// struct for a LL to hold freed memory
typedef struct ll {
	size_t length;
	void * next;
	void * prev;
} ll;

// handle struct 
typedef struct handle {
	uint32_t magicNum;
	ll * freeMem;
	nd * rootDir;
	size_t size;
} handle;
//typedef struct myfs_handle handle;

/* YOUR HELPER FUNCTIONS GO HERE */

void __free_impl(void *ptr);

// search the global LL for a large enough chunk of memory. If found, return that space and add 
// leftover to the global LL if that leftover is large enough to hold the header.
static void * searchLL(size_t size, handle * myHandle) {
	ll * temp1 = myHandle->freeMem;
//	ll * temp1 = head;

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
					myHandle->freeMem = NULL;
					return temp1;
				}
				else if (GET_PREV(temp1) == NULL) //temp1 is head of LL in this case
				{
					myHandle->freeMem = GET_NEXT(temp1);
					GET_PREV(myHandle->freeMem) = NULL;
					return temp1;
				}

				else if (GET_NEXT(temp1) == NULL) //temp1 is at tail
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
				GET_LENGTH(temp1) = SL + size;
				// Check if at tail or head
				if (GET_NEXT(temp1) == NULL && GET_PREV(temp1) == NULL) // temp1 is the only value in LL (and head)
				{
					GET_NEXT(temp2) = NULL;
					GET_PREV(temp2) = NULL;
					myHandle->freeMem = temp2;
				}
				else if (GET_NEXT(temp1) == NULL) // temp1 is tail
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
					myHandle->freeMem = temp2;

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


/* Push block to linked list of free memory */
static void push(ll * ptr, handle * myHandle)
{
//	ll * temp = head;
	ll * temp = myHandle->freeMem;

	if (temp == NULL)
	{
		myHandle->freeMem = ptr;
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
				myHandle->freeMem = ptr;
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
	if (ptr == NULL) // ptr is NULL
	{
		return;
	}
	else if (GET_PREV(ptr) == NULL && GET_NEXT(ptr) == NULL) // ptr is only value in LL
	{
		return;
	}
	else if (GET_PREV(ptr) == NULL) // at head of LL, just check if we can merge with right node
	{
		//check if one starts where the other ends and they are part of the same mmapping
		if (((void *)ptr + (GET_LENGTH(ptr)) + BYTE) == GET_NEXT(ptr))
		{
			//merge current node with next node
			GET_LENGTH(ptr) = (GET_LENGTH(ptr) + GET_LENGTH(GET_NEXT(ptr)));
			GET_NEXT(ptr) = (GET_NEXT(GET_NEXT(ptr)));
			return;
		}
	}
	else if (GET_NEXT(ptr) == NULL) // at tail of LL, just check if ptr can merge with left node
	{
		if ((GET_PREV(ptr) + (GET_LENGTH(GET_PREV(ptr)) + BYTE)) == (void *)ptr)
		{
			//merge left node with current node
			GET_LENGTH(GET_PREV(ptr)) = (GET_LENGTH(GET_PREV(ptr)) + GET_LENGTH(ptr));
			GET_NEXT(GET_PREV(ptr)) = NULL;
			ptr = GET_PREV(ptr);
			return;
		}
	}

	else // ptr is not at head or end of LL
	{
		// check if we can merge with both left and right node
		if ((GET_PREV(ptr) + GET_LENGTH(GET_PREV(ptr) + BYTE)) == (void *)ptr && ((ptr + GET_LENGTH(ptr) + BYTE) == GET_NEXT(ptr)))
		{
			//merge with both left and right nodes
			GET_LENGTH(GET_PREV(ptr)) = (GET_LENGTH(GET_PREV(ptr)) + GET_LENGTH(ptr) + GET_LENGTH(GET_NEXT(ptr)));
			GET_NEXT(GET_PREV(ptr)) = GET_NEXT(GET_NEXT(ptr));
			ptr = GET_PREV(ptr);
			return;
		}
		// check if we can merge with left node
		else if ((GET_PREV(ptr) + GET_LENGTH(GET_PREV(ptr)) + BYTE) == (void *)ptr)
		{
			//merge left node with current node
			GET_LENGTH(GET_PREV(ptr)) = (GET_LENGTH(GET_PREV(ptr)) + GET_LENGTH(ptr));
			GET_NEXT(GET_PREV(ptr)) = GET_NEXT(ptr);
			ptr = GET_PREV(ptr);
			return;

		}
		// check if we can merge with right node
		else if (((void *)ptr + GET_LENGTH(ptr) + BYTE) == GET_NEXT(ptr))
		{
			//merge current node with next node
			GET_LENGTH(ptr) = (GET_LENGTH(ptr) + GET_LENGTH(GET_NEXT(ptr)));
			GET_NEXT(ptr) = (GET_NEXT(GET_NEXT(ptr)));
			return;
		}
		else // can't merge with either left or right node
		{
			return;
		}
	}
}


// function to get the handle containing global variables and init information
static handle * getHandle(void *fsptr, size_t size) {
	handle * myHandle;
	size_t s;
	// check if size of filesystem is large enough
//	if (size < sizeof(struct handle)) return NULL;
	myHandle = (handle*)fsptr;
	// if first time mounting, initialize values 
	if (myHandle->magicNum != MAGICNUMBER)
	{
		s = size - sizeof(handle);		
		ll * head = fsptr + (1000 * sizeof(nd));	//create first ll block to hold free memory 
		head->length = s;
		head->next = NULL;
		head->prev = NULL;
		nd * rootDir = fsptr + sizeof(handle); 		//location of rootDirectory is immediately after the handle
		// initialize rootDir variables here //
		myHandle->magicNum = MAGICNUMBER;
		myHandle->size = s;
		myHandle->rootDir = rootDir;  
		myHandle->freeMem = head; 
	}
	return myHandle;
}

void printPtrs(ll * freeMem)
{
	ll * temp = freeMem;
	while (temp != NULL)
	{
		printf("length of ptr = %ld\n", temp->length);
		temp = temp->next;
	}
	printf("All ptrs in list printed above\n\n\n");
}

int main( int argc, const char* argv[] )
{
	size_t fssize = 10000;
	void * fsptr = malloc(10000);
	handle * myHandle = getHandle(fsptr, fssize);

	printPtrs(myHandle->freeMem);

	ll * temp1 = searchLL(1000, myHandle);
	printPtrs(myHandle->freeMem);

//	printf("length of temp1 = %ld", temp1->length);
//	printf("length of freeMemLL = %ld", myHandle->freeMem->length);

	push(temp1, myHandle);
	printPtrs(myHandle->freeMem);


//	printf("length of temp1 = %ld", temp1->length);
//	printf("length of freeMemLL = %ld", myHandle->freeMem->length);

	merge(temp1);
	printPtrs(myHandle->freeMem);

//	printf("length of freeMemLL = %ld", myHandle->freeMem->length);

}
