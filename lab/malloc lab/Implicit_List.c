/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "ateam",
    /* First member's full name */
    "Harry Bovik",
    /* First member's email address */
    "bovik@cs.cmu.edu",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

#define USED(size) (*SIZE_T(size) & 1)

#define SIZE(size) (*SIZE_T(size) & ~1)
#define CHAR(addr) ((char*) (addr))
#define SIZE_T(addr) ((size_t*) (addr))
/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    void* start = mem_heap_lo();
    void* end = mem_heap_hi();
    int newsize = ALIGN(size + SIZE_T_SIZE);
    void* p = start;
    while ((p <= end) && (USED(p) || SIZE(p) < newsize)) {
        p = (CHAR(p) + SIZE(p));
    }
    
    if (p > end) {
        p = mem_sbrk(newsize);
        if (p == (void *)-1)
        return NULL;
        *SIZE_T(p) = newsize;
    }
    p = addblock(p, newsize);
    return p;
}

void* addblock(void* p, size_t length) {
    size_t oldsize = SIZE(p);
    if (oldsize - length <= SIZE_T_SIZE) {
        *SIZE_T(p) = oldsize | 1;
    }else {
        *SIZE_T(p) = length | 1;
        void* np = CHAR(p) + length;
        *SIZE_T(np) =oldsize-length; 
    }
    //printf("addblock %d \n", SIZE(p));
    return (void*)((char*)p + SIZE_T_SIZE);
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    void *start = mem_heap_lo();
    void *end = mem_heap_hi();
    void *p = start;
    ptr = CHAR(ptr) - SIZE_T_SIZE;
    //printf("before head: %d \n", *SIZE_T(ptr));
    if (p == ptr) {
        coalesce(ptr);
        return;
    }
    void *np = CHAR(p) + SIZE(p);
    while((np <= end) && (np != ptr)) {
        p = CHAR(p) + SIZE(p);
        np = CHAR(np) + SIZE(np);
    }
    if (np != ptr) {
        exit(1);
    }
    coalesce(ptr);
    if (!USED(p)) 
        coalesce(p);
    //printf ("End head : %d \n", *SIZE_T(ptr));
}

void coalesce(void *ptr) {
    void *end = mem_heap_hi();
    size_t oldsize = SIZE(ptr);
    void *np = CHAR(ptr)+oldsize;
    if (CHAR(np) > CHAR(end) || USED(np)) {
        *SIZE_T(ptr) = oldsize;
    }else {
        size_t nextsize = SIZE(np);
        *SIZE_T(ptr) = oldsize+nextsize;
    }
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;

    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;
    copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE) - SIZE_T_SIZE;
    if (size < copySize)
      copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}














