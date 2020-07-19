#include "csapp.h"

typedef struct {
    int readcount;
    int readcnt;
    sem_t mutex;
    sem_t w;
} rw_t;

typedef struct rw_mutex{
    char host[MAXLINE];
    char port[MAXLINE];
    char url[MAXLINE];
    char* content;
    int content_length;
    struct rw_mutex *next;
}cache_t;

void rw_init(rw_t* rw);

void setcache(cache_t* cache, char* host, char* port, char* url, char* content, int content_length);

void freecache(cache_t* cache);

void addcache(cache_t* cache, cache_t** root);

cache_t* findcache(cache_t* root, char* host, char* port, char* url);

void movefirst(cache_t* cache, cache_t** root);

int removelast(cache_t** root);
