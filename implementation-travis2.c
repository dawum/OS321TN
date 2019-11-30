/*
  MyFS: a tiny file-system written for educational purposes
  MyFS is
  Copyright 2018-19 by
  University of Alaska Anchorage, College of Engineering.
  Contributors: Christoph Lauter
				... and
				...
  and based on
  FUSE: Filesystem in Userspace
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>
  This program can be distributed under the terms of the GNU GPL.
  See the file COPYING.
  gcc -Wall myfs.c implementation.c `pkg-config fuse --cflags --libs` -o myfs
*/

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
#include <stdbool.h>


/* The filesystem you implement must support all the 13 operations
   stubbed out below. There need not be support for access rights,
   links, symbolic links. There needs to be support for access and
   modification times and information for statfs.
   The filesystem must run in memory, using the memory of size
   fssize pointed to by fsptr. The memory comes from mmap and
   is backed with a file if a backup-file is indicated. When
   the filesystem is unmounted, the memory is written back to
   that backup-file. When the filesystem is mounted again from
   the backup-file, the same memory appears at the newly mapped
   in virtual address. The filesystem datastructures hence must not
   store any pointer directly to the memory pointed to by fsptr; it
   must rather store offsets from the beginning of the memory region.
   When a filesystem is mounted for the first time, the whole memory
   region of size fssize pointed to by fsptr reads as zero-bytes. When
   a backup-file is used and the filesystem is mounted again, certain
   parts of the memory, which have previously been written, may read
   as non-zero bytes. The size of the memory region is at least 2048
   bytes.
   CAUTION:
   * You MUST NOT use any global variables in your program for reasons
   due to the way FUSE is designed.
   You can find ways to store a structure containing all "global" data
   at the start of the memory region representing the filesystem.
   * You MUST NOT store (the value of) pointers into the memory region
   that represents the filesystem. Pointers are virtual memory
   addresses and these addresses are ephemeral. Everything will seem
   okay UNTIL you remount the filesystem again.
   You may store offsets/indices (of type size_t) into the
   filesystem. These offsets/indices are like pointers: instead of
   storing the pointer, you store how far it is away from the start of
   the memory region. You may want to define a type for your offsets
   and to write two functions that can convert from pointers to
   offsets and vice versa.
   * You may use any function out of libc for your filesystem,
   including (but not limited to) malloc, calloc, free, strdup,
   strlen, strncpy, strchr, strrchr, memset, memcpy. However, your
   filesystem MUST NOT depend on memory outside of the filesystem
   memory region. Only this part of the virtual memory address space
   gets saved into the backup-file. As a matter of course, your FUSE
   process, which implements the filesystem, MUST NOT leak memory: be
   careful in particular not to leak tiny amounts of memory that
   accumulate over time. In a working setup, a FUSE process is
   supposed to run for a long time!
   It is possible to check for memory leaks by running the FUSE
   process inside valgrind:
   valgrind --leak-check=full ./myfs --backupfile=test.myfs ~/fuse-mnt/ -f
   However, the analysis of the leak indications displayed by valgrind
   is difficult as libfuse contains some small memory leaks (which do
   not accumulate over time). We cannot (easily) fix these memory
   leaks inside libfuse.
   * Avoid putting debug messages into the code. You may use fprintf
   for debugging purposes but they should all go away in the final
   version of the code. Using gdb is more professional, though.
   * You MUST NOT fail with exit(1) in case of an error. All the
   functions you have to implement have ways to indicated failure
   cases. Use these, mapping your internal errors intelligently onto
   the POSIX error conditions.
   * And of course: your code MUST NOT SEGFAULT!
   It is reasonable to proceed in the following order:
   (1)   Design and implement a mechanism that initializes a filesystem
		 whenever the memory space is fresh. That mechanism can be
		 implemented in the form of a filesystem handle into which the
		 filesystem raw memory pointer and sizes are translated.
		 Check that the filesystem does not get reinitialized at mount
		 time if you initialized it once and unmounted it but that all
		 pieces of information (in the handle) get read back correctly
		 from the backup-file.
   (2)   Design and implement functions to find and allocate free memory
		 regions inside the filesystem memory space. There need to be
		 functions to free these regions again, too. Any "global" variable
		 goes into the handle structure the mechanism designed at step (1)
		 provides.
   (3)   Carefully design a data structure able to represent all the
		 pieces of information that are needed for files and
		 (sub-)directories.  You need to store the location of the
		 root directory in a "global" variable that, again, goes into the
		 handle designed at step (1).
   (4)   Write __myfs_getattr_implem and debug it thoroughly, as best as
		 you can with a filesystem that is reduced to one
		 function. Writing this function will make you write helper
		 functions to traverse paths, following the appropriate
		 subdirectories inside the file system. Strive for modularity for
		 these filesystem traversal functions.
   (5)   Design and implement __myfs_readdir_implem. You cannot test it
		 besides by listing your root directory with ls -la and looking
		 at the date of last access/modification of the directory (.).
		 Be sure to understand the signature of that function and use
		 caution not to provoke segfaults nor to leak memory.
   (6)   Design and implement __myfs_mknod_implem. You can now touch files
		 with
		 touch foo
		 and check that they start to exist (with the appropriate
		 access/modification times) with ls -la.
   (7)   Design and implement __myfs_mkdir_implem. Test as above.
   (8)   Design and implement __myfs_truncate_implem. You can now
		 create files filled with zeros:
		 truncate -s 1024 foo
   (9)   Design and implement __myfs_statfs_implem. Test by running
		 df before and after the truncation of a file to various lengths.
		 The free "disk" space must change accordingly.
   (10)  Design, implement and test __myfs_utimens_implem. You can now
		 touch files at different dates (in the past, in the future).
   (11)  Design and implement __myfs_open_implem. The function can
		 only be tested once __myfs_read_implem and __myfs_write_implem are
		 implemented.
   (12)  Design, implement and test __myfs_read_implem and
		 __myfs_write_implem. You can now write to files and read the data
		 back:
		 echo "Hello world" > fooC:
		 echo "Hallo ihr da" >> foo
		 cat foo
		 Be sure to test the case when you unmount and remount the
		 filesystem: the files must still be there, contain the same
		 information and have the same access and/or modification
		 times.
   (13)  Design, implement and test __myfs_unlink_implem. You can now
		 remove files.
   (14)  Design, implement and test __myfs_unlink_implem. You can now
		 remove directories.
   (15)  Design, implement and test __myfs_rename_implem. This function
		 is extremely complicated to implement. Be sure to cover all
		 cases that are documented in man 2 rename. The case when the
		 new path exists already is really hard to implement. Be sure to
		 never leave the filessystem in a bad state! Test thoroughly
		 using mv on (filled and empty) directories and files onto
		 inexistant and already existing directories and files.
   (16)  Design, implement and test any function that your instructor
		 might have left out from this list. There are 13 functions
		 __myfs_XXX_implem you have to write.
   (17)  Go over all functions again, testing them one-by-one, trying
		 to exercise all special conditions (error conditions): set
		 breakpoints in gdb and use a sequence of bash commands inside
		 your mounted filesystem to trigger these special cases. Be
		 sure to cover all funny cases that arise when the filesystem
		 is full but files are supposed to get written to or truncated
		 to longer length. There must not be any segfault; the user
		 space program using your filesystem just has to report an
		 error. Also be sure to unmount and remount your filesystem,
		 in order to be sure that it contents do not change by
		 unmounting and remounting. Try to mount two of your
		 filesystems at different places and copy and move (rename!)
		 (heavy) files (your favorite movie or song, an image of a cat
		 etc.) from one mount-point to the other. None of the two FUSE
		 processes must provoke errors. Find ways to test the case
		 when files have holes as the process that wrote them seeked
		 beyond the end of the file several times. Your filesystem must
		 support these operations at least by making the holes explicit
		 zeros (use dd to test this aspect).
   (18)  Run some heavy testing: copy your favorite movie into your
		 filesystem and try to watch it out of the filesystem.
*/

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
			rootNode->path[0] = NULL;
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
	dup = strdup(path); // dup is to be manipulated 
	dup1 = strdup(path);
	dup1 = dirname(dup1); // dup1 is path
	int hasParent = -1;


	//check if it already exsists and there is parent path
	for (int i = 0; i < MAXFNUM; i++)
	{
		//has parent
		if (strcmp(rdr->fullpath,dup1) == 0 && rdr->fileSize == -1)
		{
			hasParent = 0;
			parent = rdr;
		}
		if (strcmp(rdr->fullpath,dup)==0)
		{
			//printf("dup dir found, cannot add\n");
			return EEXIST;
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
	//printf("No more dirs can be added\n");
	return EFAULT;
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
		// has parent and is a dir
		if (strcmp(root->fullpath,dup1) == 0 && root->fileSize == -1)
		{
			hasParent = 0;
			parent = root;
		}
		//sub check
		if (strcmp(root->path,dup) == 0)
		{
			//printf("has sub dir, cannnot delete dir\n");
			return ENOTEMPTY;
		}
		//found dir
		if (strcmp(root->fullpath,dup) == 0 && root->fileSize == -1)
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
		return ENOENT;
	}

}

// create a file in a path of given size
static int addFile(hd * handle, char * path, size_t size, char * data)
{
	nd * currNode = handle->rootDir;
	char * dup = strdup(path);
	char * dup1 = strdup(path);
    //char * error;
	int hasParent = -1;


	dup1 = dirname(dup1); // dup1 = parent 

	for (int i = 0; i < MAXFNUM; i++)
	{
		// dup found
		if (strcmp(currNode->fullpath,dup) == 0)
		{
			//printf("FIle already exists, add failed");
            //error = EEXIST;
			return EEXIST;
		}
		//check parent 
		if (strcmp(currNode->fullpath,dup1) == 0 && currNode->fileSize == -1)
		{
			hasParent = 0;
		}
		currNode++;
	}//end of dup and parent check for loop

	//reset currNode
	currNode = handle->rootDir;

	// parent dir found
	if (hasParent == 0)
	{
		for (int i = 0; i < MAXFNUM; i++)
		{
            // find an emptr nd and add file to fs 
			if (currNode->isFree == 0)
			{
				currNode->isFree = -1;
				currNode->subdirs = -1;
				strcpy(currNode->fullpath,dup);
				strcpy(currNode->name,basename(dup));
				strcpy(currNode->path,dirname(dup));
				currNode->fileSize = size;
				void * temp = searchLL(size,handle);
				currNode->offset = GETOFFSET(handle,temp);
				memcpy(GETPTR(handle,currNode->offset),data,size); // setdata
				return 0; // success!
			}
			currNode++;
		}
	}
	//printf("parent dir not found.\n");
	return ENOENT;
}

static int deleteFile(hd * handle, char * path)
{
	nd * curr = handle->rootDir;
	char * dup = strdup(path);
	for (int i = 0; i < MAXFNUM; i++)
	{
		// found file and is not dir
		if (strcmp(curr->fullpath,dup) == 0 && curr->fileSize > -1)
		{
			// push its data back to free ll
			push(GETPTR(handle,curr->offset),handle);
			curr->isFree = 0;
			return 0;
		}
		curr++;
	}
	//printf("file not found, del failed\n");
	return ENOENT;
}

//for debug purpose
static void printNodes(hd * hd)
{
	nd * root = hd->rootDir;
	for (int i = 0; i < MAXFNUM; i++)
	{
		if (root->isFree == -1)
			{
				printf("Current Index    : %d\n",i);
				printf("Fullpath:%s ",root->fullpath);
				printf("Path:%s ",root->path);
				printf("filename:%s ",root->name);
				printf("isFree:%d ",root->isFree);
				printf("fileSize:%d ",root->fileSize);
				printf("subdirs:%d ",root->subdirs);
				printf("offset:%d\n",root->offset);
				if (root->offset != -1) printf("data:%s\n",GETPTR(hd,root->offset));
			}		
		root++;
	}
}

/* End of helper functions */

/* Implements an emulation of the stat system call on the filesystem
   of size fssize pointed to by fsptr.
   If path can be followed and describes a file or directory
   that exists and is accessable, the access information is
   put into stbuf.
   On success, 0 is returned. On failure, -1 is returned and
   the appropriate error code is put into *errnoptr.
   man 2 stat documents all possible error codes and gives more detail
   on what fields of stbuf need to be filled in. Essentially, only the
   following fields need to be supported:
   st_uid      the value passed in argument
   st_gid      the value passed in argument
   st_mode     (as fixed values S_IFDIR | 0755 for directories,
								S_IFREG | 0755 for files)
   st_nlink    (as many as there are subdirectories (not files) for directories
				(including . and ..),
				1 for files)
   st_size     (supported only for files, where it is the real file size)
   st_atim
   st_mtim
*/


int __myfs_getattr_implem(void *fsptr, size_t fssize, int *errnoptr,
	uid_t uid, gid_t gid,
	const char *path, struct stat *stbuf) {
		initHandle(fsptr,fssize);
		hd * handle = fsptr;
		nd * curr = handle->rootDir;
		char * dup = strdup(path);

	stbuf->st_uid = uid;
	stbuf->st_gid = gid;

	for (int i = 0; i < MAXFNUM; i++)
	{
		if (strcmp(dup,curr->fullpath) == 0 && curr->fileSize == -1)
		{
			stbuf->st_mode = S_IFDIR | 0755;
			stbuf->st_nlink = curr->subdirs + 2;
			stbuf->st_atime = time(NULL);
			stbuf->st_mtime = time(NULL);
			return 0;
		}
		else if (strcmp(dup,curr->fullpath) == 0)
		{
		stbuf->st_mode = S_IFREG | 0755;
		stbuf->st_nlink = 1;
		stbuf->st_size = curr->fileSize;
		stbuf->st_atime = time(NULL);
		stbuf->st_mtime = time(NULL);
		return 0;
		}
		curr++;
	}
	return -1;
}

/* Implements an emulation of the readdir system call on the filesystem
   of size fssize pointed to by fsptr.
   If path can be followed and describes a directory that exists and
   is accessable, the names of the subdirectories and files
   contained in that directory are output into *namesptr. The . and ..
   directories must not be included in that listing.
   If it needs to output file and subdirectory names, the function
   starts by allocating (with calloc) an array of pointers to
   characters of the right size (n entries for n names). Sets
   *namesptr to that pointer. 
   
   It then goes over all entries in that array and allocates, for each of them an array of
   characters of the right size (to hold the i-th name, together
   with the appropriate '\0' terminator). It puts the pointer
   into that i-th array entry and fills the allocated array
   of characters with the appropriate name. 
   
   The calling function
   will call free on each of the entries of *namesptr and
   on *namesptr.
   The function returns the number of names that have been
   put into namesptr.
   
   If no name needs to be reported because the directory does
   not contain any file or subdirectory besides . and .., 0 is
   returned and no allocation takes place.
   On failure, -1 is returned and the *errnoptr is set to
   the appropriate error code.
   The error codes are documented in man 2 readdir.
   In the case memory allocation with malloc/calloc fails, failure is
   indicated by returning -1 and setting *errnoptr to EINVAL.
*/

int __myfs_readdir_implem(void *fsptr, size_t fssize, int *errnoptr,

	const char *path, char ***namesptr) {

	initHandle(fsptr,fssize);
	hd * handle = fsptr;
	nd * rdr = handle->rootDir;

	bool dirFound = false;
	char * dup;
	dup = strdup(path); 

	//1st check if given path exists
	for (int i = 0; i < MAXFNUM; i++)
	{

		//path is found but is for a file
		if (strcmp(rdr->fullpath,dup)==0 && rdr->fileSize > 0)
		{
			*errnoptr = ENOTDIR;
			return -1;
		}

		//path is found and is directory
		if (strcmp(rdr->fullpath,dup)==0 && rdr->fileSize == -1)
		{	
			dirFound = true;
			break;
		}

	rdr++;
	}
	//no directory found from given path, return errrnoptr
	if (dirFound == false)
	{
		*errnoptr = ENOENT;
		return -1;
	}

	//reset rdr
	rdr = handle->rootDir;

	//find the number of files and subdirectories contained within the directory given
	int nameCount = 0;
	char * temp[1000];   //infeasible to expect more then 1000 files/subdirectories

	for (int i = 0; i < MAXFNUM; i++)
	{
		//if path is found, extract name of file/directory and store it in a temporary array of char ptrs
		if (strcmp(rdr->path,dup)==0)
		{	
			temp[nameCount] = rdr->name;	
			nameCount++;
		}
	rdr++;
	}
    //if no names found, return 0
    if (nameCount == 0)
    {
        return 0;
    }
	//printf("nameCount: %d", nameCount);
    //printf("name: %s", temp[0]);
    
	*namesptr = calloc(nameCount, 1);

	for(int i = 0; i < nameCount; i++)
	{
		*namesptr[i] = temp[nameCount];
 	}
     
	return nameCount;
	
}

/* Implements an emulation of the mknod system call for regular files
   on the filesystem of size fssize pointed to by fsptr.

   This function is called only for the creation of regular files.

   If a file gets created, it is of size zero and has default
   ownership and mode bits.

   The call creates the file indicated by path.

   On success, 0 is returned.

   On failure, -1 is returned and *errnoptr is set appropriately.
   The error codes are documented in man 2 mknod.
*/
int __myfs_mknod_implem(void *fsptr, size_t fssize, int *errnoptr,
	const char *path) {

	initHandle(fsptr,fssize);
	hd * handle = fsptr;
    int error = 0;
    //call addFile to create file if possible. returns error if any
    error = addFile(handle, (char *)path, 0, 0);

    if (error != 0)
    {
        *errnoptr = error;
        return -1; //fail
    }

    return 0; //success
}

/* Implements an emulation of the unlink system call for regular files
   on the filesystem of size fssize pointed to by fsptr.
   This function is called only for the deletion of regular files.
   On success, 0 is returned.
   On failure, -1 is returned and *errnoptr is set appropriately.
   The error codes are documented in man 2 unlink.
*/
int __myfs_unlink_implem(void *fsptr, size_t fssize, int *errnoptr,
	const char *path) {

	initHandle(fsptr,fssize);
	hd * handle = fsptr;
    int error = 0;

    error = deleteFile(handle, (char *)path);

    if (error != 0)
    {
        *errnoptr = error;
        return -1; //fail
    }

    return 0; //success

}

/* Implements an emulation of the rmdir system call on the filesystem
   of size fssize pointed to by fsptr.
   The call deletes the directory indicated by path.
   On success, 0 is returned.
   On failure, -1 is returned and *errnoptr is set appropriately.
   The function call must fail when the directory indicated by path is
   not empty (if there are files or subdirectories other than . and ..).
   The error codes are documented in man 2 rmdir.
*/
int __myfs_rmdir_implem(void *fsptr, size_t fssize, int *errnoptr,
	const char *path) {

	initHandle(fsptr,fssize);
	hd * handle = fsptr;
    int error = 0;

    error = deleteDirectory(handle, (char *)path);

    if (error != 0)
    {
        *errnoptr = error;
        return -1; //fail
    }

	return 0; //success

}

/* Implements an emulation of the mkdir system call on the filesystem
   of size fssize pointed to by fsptr.
   The call creates the directory indicated by path.
   On success, 0 is returned.
   On failure, -1 is returned and *errnoptr is set appropriately.
   The error codes are documented in man 2 mkdir.
*/
int __myfs_mkdir_implem(void *fsptr, size_t fssize, int *errnoptr,
	const char *path) {

	initHandle(fsptr,fssize);
	hd * handle = fsptr;
    int error = 0;

    error = addDirectory(handle, (char *)path);

    if (error != 0)
    {
        *errnoptr = error;
        return -1; //fail
    }

	return 0; //success
}

/* Implements an emulation of the rename system call on the filesystem
   of size fssize pointed to by fsptr.
   The call moves the file or directory indicated by from to to.
   On success, 0 is returned.
   On failure, -1 is returned and *errnoptr is set appropriately.
   Caution: the function does more than what is hinted to by its name.
   In cases the from and to paths differ, the file is moved out of
   the from path and added to the to path.
   The error codes are documented in man 2 rename.
*/
int __myfs_rename_implem(void *fsptr, size_t fssize, int *errnoptr,
	const char *from, const char *to) {
	/* STUB */
	return -1;
}

/* Implements an emulation of the truncate system call on the filesystem
   of size fssize pointed to by fsptr.
   The call changes the size of the file indicated by path to offset
   bytes.
   When the file becomes smaller due to the call, the extending bytes are
   removed. When it becomes larger, zeros are appended.
   On success, 0 is returned.
   On failure, -1 is returned and *errnoptr is set appropriately.
   The error codes are documented in man 2 truncate.
*/
int __myfs_truncate_implem(void *fsptr, size_t fssize, int *errnoptr,
	const char *path, off_t offset) {
	/* STUB */
	return -1;
}

/* Implements an emulation of the open system call on the filesystem
   of size fssize pointed to by fsptr, without actually performing the opening
   of the file (no file descriptor is returned).
   The call just checks if the file (or directory) indicated by path
   can be accessed, i.e. if the path can be followed to an existing
   object for which the access rights are granted.
   On success, 0 is returned.
   On failure, -1 is returned and *errnoptr is set appropriately.
   The two only interesting error codes are
   * EFAULT: the filesystem is in a bad state, we can't do anything
   * ENOENT: the file that we are supposed to open doesn't exist (or a
			 subpath).
   It is possible to restrict ourselves to only these two error
   conditions. It is also possible to implement more detailed error
   condition answers.
   The error codes are documented in man 2 open.
*/
int __myfs_open_implem(void *fsptr, size_t fssize, int *errnoptr,
	const char *path) {
	/* STUB */
	return -1;
}

/* Implements an emulation of the read system call on the filesystem
   of size fssize pointed to by fsptr.
   The call copies up to size bytes from the file indicated by
   path into the buffer, starting to read at offset. See the man page
   for read for the details when offset is beyond the end of the file etc.
   On success, the appropriate number of bytes read into the buffer is
   returned. The value zero is returned on an end-of-file condition.
   On failure, -1 is returned and *errnoptr is set appropriately.
   The error codes are documented in man 2 read.
*/
int __myfs_read_implem(void *fsptr, size_t fssize, int *errnoptr,
	const char *path, char *buf, size_t size, off_t offset) {
	/* STUB */
	return -1;
}

/* Implements an emulation of the write system call on the filesystem
   of size fssize pointed to by fsptr.
   The call copies up to size bytes to the file indicated by
   path into the buffer, starting to write at offset. See the man page
   for write for the details when offset is beyond the end of the file etc.
   On success, the appropriate number of bytes written into the file is
   returned. The value zero is returned on an end-of-file condition.
   On failure, -1 is returned and *errnoptr is set appropriately.
   The error codes are documented in man 2 write.
*/
int __myfs_write_implem(void *fsptr, size_t fssize, int *errnoptr,
	const char *path, const char *buf, size_t size, off_t offset) {
	/* STUB */
	return -1;
}

/* Implements an emulation of the utimensat system call on the filesystem
   of size fssize pointed to by fsptr.
   The call changes the access and modification times of the file
   or directory indicated by path to the values in ts.
   On success, 0 is returned.
   On failure, -1 is returned and *errnoptr is set appropriately.
   The error codes are documented in man 2 utimensat.
*/
int __myfs_utimens_implem(void *fsptr, size_t fssize, int *errnoptr,
	const char *path, const struct timespec ts[2]) {
	/* STUB */
	return -1;
}

/* Implements an emulation of the statfs system call on the filesystem
   of size fssize pointed to by fsptr.
   The call gets information of the filesystem usage and puts in
   into stbuf.
   On success, 0 is returned.
   On failure, -1 is returned and *errnoptr is set appropriately.
   The error codes are documented in man 2 statfs.
   Essentially, only the following fields of struct statvfs need to be
   supported:
   f_bsize   fill with what you call a block (typically 1024 bytes)
   f_blocks  fill with the total number of blocks in the filesystem
   f_bfree   fill with the free number of blocks in the filesystem
   f_bavail  fill with same value as f_bfree
   f_namemax fill with your maximum file/directory name, if your
			 filesystem has such a maximum
*/
int __myfs_statfs_implem(void *fsptr, size_t fssize, int *errnoptr,
	struct statvfs* stbuf) {
	/* STUB */
	return -1;
}
