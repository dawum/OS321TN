#include <libgen.h>
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
#include <time.h>



#define GETPTR(p,o) ((void *)(p))+o 
#define GETOFFSET(p,o) (void *) o - (void *) p 
#define MAGICNUMBER ((uint32_t) 22222222)
#define GET_LENGTH(p) ((ll *)(p))->length
#define GET_NEXT(p) ((ll *)(p))->next
#define GET_PREV(p) ((ll *)(p))->prev
#define MAXFNSIZE ((size_t) 25)
#define MAXFNUM ((size_t) 1000)

#define SL sizeof(ll) // Size of LL
#define BYTE sizeof(char) // 1 byte

//set head to point to space far enough from fsptr to have room to store directory/file metadata
//ll * head = fsptr + 100000

/* Helper types and functions */

// struct to hold directories and file metad


// struct for a LL to hold freed memory
struct memory {
	size_t length;
	void * next;
	void * prev;
}memory;
typedef struct memory ll;

struct node {
    char path[MAXFNSIZE]; 
	char name[MAXFNSIZE]; 
	char fullpath[MAXFNSIZE];
	size_t offset;
	size_t subdirs;
    int fileSize;
	int isFree;	
}node;
typedef struct node nd;
// handle struct 
struct handle {
	uint32_t magicNum;
	ll * freeMem;
	nd * rootDir;
	size_t size;
}handle;
typedef struct handle hd;

// search the global LL for a large enough chunk of memory. If found, return that space and add 
// leftover to the global LL if that leftover is large enough to hold the header.
static void * searchLL(size_t size, hd * myHandle) {
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
static void push(ll * ptr, hd * myHandle)
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

static int initHandle(void * fsptr, size_t size)
{
	size_t minReqSize = sizeof(handle) + (sizeof(node) * MAXFNUM) + (sizeof(memory) + 1);
	char * slash = malloc(1);
	slash[0] = '/';
	// check size of block 
	if (size < minReqSize)
	{
		printf("Init failed. Given space is too small.\n");
		return -1;
	}
	else
	{
		//check magic number
		hd * handle = fsptr;
		handle->rootDir = fsptr + sizeof(hd);
		handle->freeMem = fsptr + sizeof(hd) + (sizeof(nd) * MAXFNUM);
		
		//magic number found
		if (handle->magicNum == MAGICNUMBER)
		{
			//do nothing 
			return 0;
		}
		//magic number not found
		else
		{
			handle->magicNum = MAGICNUMBER;
			nd * rootNode = handle->rootDir;
			//setup rootnode
			rootNode->fileSize = -1;
			rootNode->isFree = -1;
			rootNode->offset = -1;
			rootNode->fullpath[0] = '/';
			rootNode->path[0] = '/';
			rootNode->name[0] = '/';
			return 0;
		}
	}	
}

static int addDirectory(hd * hd, char * path){
	nd * rdr = hd->rootDir;
	nd * parent;
	char * dup;
	char * dup1;
	dup = strdup(path);
	dup1 = strdup(path);
	dup1 = dirname(dup1);
	int hasParent = -1;


	//check if it already exsists and there is parent path
	for (int i = 0; i < MAXFNUM; i++)
	{
		//has parent
		if (strcmp(rdr->fullpath,dup1) == 0)
		{
			hasParent = 0;
			parent = rdr;
		}
		if (strcmp(rdr->fullpath,dup)==0)
		{
			printf("dup dir found, cannot add\n");
			return -1;
		}
		else
		{
			rdr++;
		}
	}
	// go back to root dir
	rdr = hd->rootDir;

	// attempt to find empty slot and add node
	if (hasParent == 0)
	{
	for (int i = 0; i < MAXFNUM; i++)
	{
		if (rdr->isFree == 0)
		{
			rdr->isFree = -1;
			rdr->fileSize = -1;
			rdr->subdirs = 0;
			strcpy(rdr->fullpath,dup);
			strcpy(rdr->name,basename(dup));
			strcpy(rdr->path,dirname(dup));
			rdr->offset = -1;
			parent->subdirs++; // increment parent subdir count
			return 0;
		}
		else
		{
			rdr++;
		}
	} 
	}
	printf("No more dirs can be added\n");
	return -1;
}
static int deleteDirectory(hd * hd, char * path)
{
	char * dup = strdup(path);
	char * dup1 = strdup(path);
	dup1 = dirname(dup1); // dup1 = path
	nd * root = hd->rootDir;
	nd * parent;
	int hasParent = -1;
	int dirFound = -1;
	nd * found;
	// make sure there is dir and has no subs
	for (int i = 0; i < MAXFNUM; i++)
	{
		// has parent
		if (strcmp(root->fullpath,dup1) == 0)
		{
			hasParent = 0;
			parent = root;
		}
		//sub check
		if (strcmp(root->path,dup) == 0)
		{
			printf("has sub dir, cannnot delete dir\n");
			return -1;
		}
		//found dir
		if (strcmp(root->fullpath,dup) == 0)
		{
			dirFound = 0;
			found = root;
		}
		root++;
	}
	// has no sub and dir found and has parent
	if (dirFound == 0 && hasParent == 0)
	{
		parent->subdirs--; // decrement parent subdir count
		found->isFree = 0;
		return 0;
	}
	else 
	{
		return -1;
	}

}
//for debug purpose
static void printNodes(hd * hd)
{
	nd * root = hd->rootDir;
	for (int i = 0; i < MAXFNUM; i++)
	{
		if (root->isFree == -1)
			{
				printf("%s %s %s %d %d %d\n",root->fullpath,root->path,root->name,root->fileSize,root->offset,root->subdirs,root->isFree);
			}		
		root++;
	}
}
int main(int argc, char **argv)
{

    void * ptr = malloc(1000000);
	
	
	initHandle(ptr,1000000);

	char f[100] = "/dir1";
	 addDirectory(ptr,f);

	 strcpy(f,"/dir1/dir2");

	 addDirectory(ptr,f);

	 strcpy(f,"/dir1/dir2/dir3");

	 addDirectory(ptr,f);

	 printNodes(ptr);

	 strcpy(f,"/dir1/dir2");
	 deleteDirectory(ptr,f);

	 printNodes(ptr);



free(ptr);
	
    
    return 0;
}
