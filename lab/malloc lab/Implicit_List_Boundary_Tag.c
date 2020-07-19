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

#define SIZE(size) (*SIZE_T(size) & ~0x7)
#define CHAR(addr) ((char*) (addr))
#define SIZE_T(addr) ((size_t*) (addr))
#define FOOT(addr) (CHAR(addr) + SIZE(addr) - SIZE_T_SIZE)
#define LPUSED(size) (*SIZE_T(size) & 2)
#define NP(addr) (CHAR(addr) + SIZE(addr))
#define LSIZE(addr) (SIZE(CHAR(addr) - SIZE_T_SIZE))
#define LP(addr) (CHAR(addr) - LSIZE(addr))
//#define CHECK (mm_checker())
#define CHECK 0
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
    if (size == 0) {
        return NULL;
    }
    void* start = mem_heap_lo();
    void* end = mem_heap_hi();
    int newsize = ALIGN(size + SIZE_T_SIZE);
    void* p = start;
    int use_lp = 1;
    while ((p <= end) && (USED(p) || SIZE(p) < newsize)) {
        use_lp = USED(p);
        p = (CHAR(p) + SIZE(p));
    }
    if (p > end) {
        p = mem_sbrk(newsize);
        if (p == (void *)-1)
        return NULL;
        *SIZE_T(p) = newsize;
        *SIZE_T(FOOT(p)) = newsize;
    }
    p = addblock(p, newsize, use_lp);
    if(CHECK) {
        printf("MM-CHECK WRONG %d \n", __LINE__);
    }
    return p;
}

void* addblock(void* p, size_t length, int use_lp) {
    size_t oldsize = SIZE(p);
    if (oldsize - length < 2*SIZE_T_SIZE) {
        *SIZE_T(p) = oldsize | 1;
        void* np = NP(p);
        if (np <= mem_heap_hi()) {
            *SIZE_T(np) = *SIZE_T(np)|2;
        }
    }else {
        *SIZE_T(p) = length | 1;
        void* np = CHAR(p) + length;
        *SIZE_T(np) =oldsize-length;
        //printf("foot %x !!!!!!!!!!!!!!\n", oldsize-length);
        *SIZE_T(FOOT(np)) = oldsize - length;
    }
    if (use_lp){
        *SIZE_T(p) = *SIZE_T(p) | 2;
    }
    //printf("addblock %d \n", SIZE(p));

    if(CHECK) {
        printf("addblock %x \n", *SIZE_T(p));
        printf("use_lp %d \n", use_lp);
        void* start = mem_heap_lo();
        void* end = mem_heap_hi();
        printf ("start %x \n", start);
        printf ("end %x \n", end);
        printf ("p %x \n", p);
        if (p > start) {
            void *lp = LP(p);
            printf ("lp %x \n", lp);
            printf ("lszie is %x \n", *(size_t*)(CHAR(p)-SIZE_T_SIZE));
            printf("lp value %x \n", *SIZE_T(lp));
        }
        void* np = NP(p);
        if (np <= end) {
            printf("np value %x \n", *SIZE_T(np));
        }
        printf("MM-CHECK WRONG %d \n", __LINE__);
    }
    return (void*)((char*)p + SIZE_T_SIZE);
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    void* start = mem_heap_lo();
    ptr = CHAR(ptr) - SIZE_T_SIZE;
    //printf("ptr is %x \n", ptr);
    coalesce(ptr);
    if(CHECK) {
        printf("MM-CHECK WRONG %d \n", __LINE__);
    }
}

void coalesce(void *ptr) {
    void *end = mem_heap_hi();
    void* start = mem_heap_lo();
    size_t oldsize = SIZE(ptr);
    void *np = CHAR(ptr)+oldsize;
    int lp_used = LPUSED(ptr);
    int np_used = CHAR(np) > CHAR(end) || USED(np);
    if (lp_used && np_used) {
        *SIZE_T(ptr) = oldsize;
        *SIZE_T(FOOT(ptr)) = oldsize;
        if (np <= end){
            *SIZE_T(np) = (*SIZE_T(np) & ~2);
        }
    if(CHECK) {
        /*printf("ptr is %x\n", ptr);
        printf("head value ptr is %x\n", *SIZE_T(ptr));
        printf("foot value ptr is %x\n", *SIZE_T(FOOT(ptr)));
        printf ("foot ptr is %x \n", FOOT(ptr));
        printf("np is %x \n", np);
        printf ("foot value is %x \n", LSIZE(np));
        printf("lp_from_np is %x \n", LP(np));*/
        printf("MM-CHECK WRONG %d \n", __LINE__);
    }
    }else if (lp_used && !np_used){
        size_t nextsize = SIZE(np);
        *SIZE_T(ptr) = oldsize + nextsize;
        *SIZE_T(FOOT(ptr)) = oldsize + nextsize;
    if(CHECK) {
        printf("MM-CHECK WRONG %d \n", __LINE__);
    }
    }else if (!lp_used && np_used) {
        void* lp = LP(ptr);
        size_t newsize = oldsize + SIZE(lp);
        *SIZE_T(lp) = newsize;
        *SIZE_T(FOOT(lp)) = newsize;
        if (np <= end){
            *SIZE_T(np) = (*SIZE_T(np) & ~2);
        }
    if(CHECK) {
        printf("MM-CHECK WRONG %d \n", __LINE__);
    }
    }else {
        void* lp = LP(ptr);
        size_t newsize = oldsize + SIZE(lp) + SIZE(np);
        *SIZE_T(lp) = newsize;
        *SIZE_T(FOOT(lp)) = newsize;
    if(CHECK) {
        printf("MM-CHECK WRONG %d \n", __LINE__);
    }
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

int mm_checker(){
    void* start = mem_heap_lo();
    void* end = mem_heap_hi();

    void *p = start;
    void* lp = NULL;
    while (CHAR(p) <= CHAR(end)) {
        int size = SIZE(p);
        if(!USED(p)){ // p is free
            if(SIZE(FOOT(p)) != size) { // check foot equal head
                printf("FOOT HEAD Not Equal %d \n", __LINE__);
                return -1;
            }
            if(lp != NULL) { 
                if (!USED(lp)){ // check lp p double free
                    printf("LP P Both Free  %d \n", __LINE__);
                    return -1;
                }
            }
            void *np = NP(p);
            if (np <= end) {
                if (!USED(np)) { //check np p both free
                    printf("NP P Both Free  %d \n", __LINE__);
                }
            }
        }else{ // p is malloc
            if (LPUSED(p)) { // lp is used
                if(lp != NULL){
                    if (!USED(lp)){ // lp status not same as p show
                        printf("lp Status not same as p shows  %d \n", __LINE__);
                        return  -1;
                    }
                }
            }else{ // lp is free
                if (lp != NULL){
                    void* lp_from_p = LP(p);// check lp not same as
                    if (USED(lp)) {
                        printf("lp Status not same as p shows  %d \n", __LINE__);
                        return -1;
                    }
                    if (lp_from_p != lp) {//check lp_from_p not same as lp
                        printf("p is %x \n", p);
                        printf("p value is %x \n", *SIZE_T(p));
                        printf("lp is %x \n", lp);
                        printf("lp value is %x \n", *SIZE_T(lp));
                        printf("lp_from_p is %x \n", lp_from_p);
                        printf("lp_from_p value is %x \n", *SIZE_T(lp_from_p));
                        printf("lp_from_p not same as lp %d \n", __LINE__);
                        return -1;
                    }
                }
            }
        }
        lp = p;
        p = NP(p);
    }
    return 0;
}












