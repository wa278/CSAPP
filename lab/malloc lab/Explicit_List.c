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
#define NP(addr) (CHAR(addr) + SIZE(addr))
#define LSIZE(addr) (SIZE(CHAR(addr) - SIZE_T_SIZE))
#define LP(addr) (CHAR(addr) - LSIZE(addr))
//#define CHECK (mm_checker())
#define CHECK 0
#define CHECKz (mm_checker())
#define NEXTAREA(addr) (CHAR(addr) + SIZE_T_SIZE)
#define LASTAREA(addr) (CHAR(addr) + SIZE_T_SIZE + sizeof(size_t))

#define NEXTVALUE(addr) (*SIZE_T(NEXTAREA(addr)))
#define LASTVALUE(addr) (*SIZE_T(LASTAREA(addr)))

#define NEXTFP(addr) ((void*) NEXTVALUE(addr))
#define LASTFP(addr) ((void*) LASTVALUE(addr))

void linklastnext(void* p);
void addfirst(void* p);
void* mm_sbrk(size_t newsize);
void* findfree(void* p, size_t newsize);
int checkfreeptr(void* ptr);
int checkfreelist();
void removeblock(void* p);
int checkfreeoverlap(void* p);
int checkfreeblock(void* p);
void checkroot();
void* root = NULL;
/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    return 0;
}

void checkroot(){
    void* start = mem_heap_lo();
    void* end = mem_heap_hi();
    if  (end < start){
        root = NULL;
    }
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
    checkroot();
    int newsize = ALIGN(size + 2*SIZE_T_SIZE);
    void* p = findfree(root, newsize);
    if (p != NULL) {
        p = addblock(p, newsize);
    }else{
        p = mm_sbrk(newsize); 
    }
    if(CHECK) {
        printf("MM-CHECK WRONG %d \n", __LINE__);
    }
    return p;
}

void* mm_sbrk(size_t newsize) {
    void *p = mem_sbrk(newsize);
    if (p == (void *)-1) {
        return NULL;
    }
    *SIZE_T(p) = newsize | 1;
    *SIZE_T(FOOT(p)) = newsize | 1;
    return (void*)((char*)p + SIZE_T_SIZE);
}

void addfirst(void* p){
    if (root == NULL) {
        NEXTVALUE(p) = 0;
    }else{
        NEXTVALUE(p) = root;
        LASTVALUE(root) = p;
    }
    root = p;
    LASTVALUE(p) = 0;
}

void* findfree(void* root, size_t newsize) {
    void* ptr = root;
    while ((ptr != NULL) && (SIZE(ptr) < newsize)) {
        //printf("%x \n",ptr);
        ptr = NEXTFP(ptr);
    }
    if (ptr == NULL || SIZE(ptr) < newsize) {
        return NULL;
    }
    return ptr;
}

void removeblock(void* p){
    if (p == root){
        root = NEXTFP(p);
        if (NEXTFP(p) != NULL){
            LASTVALUE(NEXTFP(p)) = 0;
        }
    }else{
        NEXTVALUE(LASTFP(p)) = NEXTVALUE(p);
        if(NEXTFP(p) != NULL){
            LASTVALUE(NEXTFP(p)) = LASTVALUE(p);
        }
    }
    NEXTVALUE(p) = 0;
    LASTVALUE(p) = 0;
}

void* addblock(void* p, size_t length) {
    size_t oldsize = SIZE(p);
    //printf("oldsize is %x \n", oldsize);
    //printf("length is %x \n", length);
    //printf("oldsize -length is %x\n", oldsize-length);
    if (oldsize - length <= 2*SIZE_T_SIZE) {
        *SIZE_T(p) = oldsize | 1;
        *SIZE_T(FOOT(p)) = oldsize | 1;
        removeblock(p);
        if(CHECK) {
            printf("MM-CHECK WRONG %d \n", __LINE__);
        }
    }else {
        //checkfreeoverlap(NEXTFP(p));
        *SIZE_T(p) = length | 1;
        *SIZE_T(FOOT(p)) = length | 1;
        void* np = CHAR(p) + length;
        //printf("foot %x !!!!!!!!!!!!!!\n", oldsize-length);
        removeblock(p);
        *SIZE_T(np) = oldsize-length;
        *SIZE_T(FOOT(np)) = oldsize - length;
        addfirst(np);
        if(CHECK) {
            printf("MM-CHECK WRONG %d \n", __LINE__);
        }
    }
    //printf("addblock %d \n", SIZE(p));
    //printf("root is %x, p is %x \n", root, p);
    if(CHECK) {
        /*printf("addblock %x \n", *SIZE_T(p));
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
        }*/
        printf("MM-CHECK WRONG %d \n", __LINE__);
    }
    return (void*)((char*)p + SIZE_T_SIZE);
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    ptr = CHAR(ptr) - SIZE_T_SIZE;
    //printf("ptr is %x \n", ptr);
    coalesce(ptr);
    if(CHECK) {
        printf("MM-CHECK WRONG %d \n", __LINE__);
    }
}

void coalesce(void *ptr) {
    void *end = mem_heap_hi();
    void *start = mem_heap_lo();
    size_t oldsize = SIZE(ptr);
    void *np = CHAR(ptr)+oldsize;
    int lp_used = CHAR(ptr) <= CHAR(start) || USED(LP(ptr));
    int np_used = CHAR(np) > CHAR(end) || USED(np);
    if (lp_used && np_used) {
        *SIZE_T(ptr) = oldsize;
        *SIZE_T(FOOT(ptr)) = oldsize;
        addfirst(ptr);
        //printf("ptr is %x\n", ptr);
        //printf("NEXTFP is %x \n", NEXTFP(ptr));
        if(CHECK) {
            //printf("ptr is %x\n", ptr);
            //printf("head value ptr is %x\n", *SIZE_T(ptr));
            //printf("foot value ptr is %x\n", *SIZE_T(FOOT(ptr)));
            //printf ("foot ptr is %x \n", FOOT(ptr));
            //printf("np is %x \n", np);
            //printf ("foot value is %x \n", LSIZE(np));
            //printf("lp_from_np is %x \n", LP(np));
            //printf("NEXTFP is %x \n", NEXTFP(ptr));
            //printf("root is %x \n", root);
            //printf("root NEXTFP is %x \n", NEXTFP(root));
            //printf("MM-CHECK WRONG %d \n", __LINE__);
        }
    }else if (lp_used && !np_used){
        removeblock(np);
        size_t nextsize = SIZE(np);
        *SIZE_T(ptr) = oldsize + nextsize;
        *SIZE_T(FOOT(ptr)) = oldsize + nextsize;
        addfirst(ptr);
        if(CHECK) {
            printf("MM-CHECK WRONG %d \n", __LINE__);
        }
    }else if (!lp_used && np_used) {
        void* lp = LP(ptr);
       /* printf("ptr is %x\n", ptr);
        printf("head value ptr is %x\n", *SIZE_T(ptr));
        printf("lp is %x \n", lp);
        printf("head value lp is %x\n", *SIZE_T(lp));
        printf("food value lp is %x\n", LSIZE(ptr));*/
        removeblock(lp);
        size_t newsize = oldsize + SIZE(lp);
        *SIZE_T(lp) = newsize;
        *SIZE_T(FOOT(lp)) = newsize;
        addfirst(lp);
        if(CHECK) {
        printf("MM-CHECK WRONG %d \n", __LINE__);
        }
    }else {
        void* lp = LP(ptr);
        removeblock(np);
        removeblock(lp);
        size_t newsize = oldsize + SIZE(lp) + SIZE(np);
        *SIZE_T(lp) = newsize;
        *SIZE_T(FOOT(lp)) = newsize;
        addfirst(lp);
        if(CHECK) {
            printf("MM-CHECK WRONG %d \n", __LINE__);
        }
    }
}

/*void linklastnext(void * p) {
    if (p == root){
        root = NEXTVALUE(p);
        if (NEXTFP(p) != NULL){
            LASTVALUE(NEXTFP(p)) = &root;
        }
    }else {
        NEXTVALUE(LASTFP(p)) = NEXTVALUE(p);
        if (NEXTFP(p) != NULL){
            LASTVALUE(NEXTFP(p)) = LASTVALUE(p);
        }
    }
    NEXTVALUE(p) = 0;
    LASTVALUE(p) = 0;
}*/

/*void addfirst(void* p) {
    if (root == p) {
        printf("root == p \n");
    }
    if (root != NULL) {
        LASTVALUE(root)  = p;
    }
    NEXTVALUE(p) = root;
    LASTVALUE(p) = &root;
    root = p;
}*/

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
    copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE) - 2*SIZE_T_SIZE;
    if (size < copySize)
      copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}

int mm_checker(){
    if (checkfreelist()){//check free list block marked as free 
        return -1;
    }
    void* start = mem_heap_lo();
    void* end = mem_heap_hi();
    //printf("end is %x \n",end);

    void *p = start;
    void* lp = NULL;
    while (CHAR(p) <= CHAR(end)) {
        int size = SIZE(p);
        if(SIZE(FOOT(p)) != size) { // check foot equal head
            printf("%x FOOT %x HEAD %x Not Equal %d \n", p, size, SIZE(FOOT(p)), __LINE__);
            return -1;
        }
        if (lp != NULL) {
            void* lp_from_p = LP(p);
            if (lp_from_p != lp) { //check lp_from_p not same as lp
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
        if(!USED(p)){ // p is free
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
                    return -1;
                }
            }
            if (checkfreeptr(p)) {//check in free list
                printf("Free P Not in Free List  %d \n", __LINE__);
                return -1;
            }
            if (LASTFP(p) != NULL) {
                if (checkfreeptr(LASTFP(p))){
                    printf("LASTFP Not in Free List  %d \n", __LINE__);
                    return -1;
                }
            }
            if (NEXTFP(p) != NULL) {
                if (checkfreeptr(NEXTFP(p))){
                    printf("NEXTFP %x Not in Free List  %d \n", NEXTFP(p), __LINE__);
                    return -1;
                }
            }
            if (checkfreeoverlap(p)) {
                printf("Free Block %x overlap %d \n", p, __LINE__);
                return -1;
            }
        }else{ // p is malloc
            if(!checkfreeptr(p)){ // check malloc p in free list
                printf("Malloc P in Free List  %d \n", __LINE__);
                return -1;
            }
        }
        lp = p;
        p = NP(p);
    }

    return 0;
}
int checkfreelist(){
    void*p = root;
    if (root == NULL){
        return 0;
    }
    int num = 0;
    while(p != NULL){
        if(USED(p)){
            printf("Free List block is used  %d \n", __LINE__);
            return -1;
        }
        if (checkfreeblock(p)){
            printf("Free List block %x not a block  %d \n", p,  __LINE__);
            return -1;
        }
        p = NEXTFP(p);
        num++;
        if (num > 500){
            printf("free list circled  %d \n", __LINE__);
            return -1;
        }
    }
    return 0;
}
int checkfreeblock(void* p){
    void* start = mem_heap_lo();
    void* end = mem_heap_hi();
    while(start <= end){
        if (start == p){
            return 0;
        }
        start = NP(start);
    }
    return -1;
}
int checkfreeptr(void* ppp) {
    void* p = root;
    if (root == NULL){
        return -1;
    }
    while(p != NULL){
        if (p == ppp) {
            return 0;
        }
        p = NEXTFP(p);
    }
    return -1;
}

int checkfreeoverlap(void* ptr){
    void*p = root;
    while( p != NULL){
        if ((ptr > p) && (ptr < (void*)((char*)p+SIZE(p)))){
            return -1;
        }
        p = NEXTFP(p);
    }
    return 0;
}
