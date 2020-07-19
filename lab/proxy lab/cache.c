#include "cache.h"

void rw_init(rw_t* rw){
    rw->readcount = 0;
    rw->readcnt = 0;
    Sem_init(&rw->mutex, 0, 1);
    Sem_init(&rw->w, 0, 1);
}

void setcache(cache_t* cache, char* host, char* port, char* url, char* content, int content_length)
{
    strcpy(cache->host, host);
    strcpy(cache->port, port);
    strcpy(cache->url, url);
    cache->content = content;
    cache->content_length = content_length;
    cache->next = NULL;
}

void freecache(cache_t* cache){
    Free(cache->content);
    Free(cache);
}

void addcache(cache_t* cache, cache_t** root){
    if (*root != NULL){
        cache->next = *root;
    }
    *root = cache;
}

cache_t* findcache(cache_t* root, char* host, char* port, char* url){
    if (root == NULL){
        return NULL;
    }
    cache_t* p = root;
    cache_t* ret = NULL;
    while(p){
        if (!(strcmp(p->host, host) || strcmp(p->port, port) || strcmp(p->url, url))){
            ret = p;
            break;
        }
        p = p->next;
    }
    return ret;
}

void movefirst(cache_t* cache, cache_t** root){
    if (*root == cache){
        return;
    }
    cache_t* p = *root;
    while(p->next){
        if (p->next == cache){
            break;
        }
        p = p->next;
    }
    p->next = (*root)->next;
    addcache(cache, root);
}

int removelast(cache_t** root){
    if ((*root)->next == NULL){
        *root = NULL;
    }
    cache_t* p = *root;
    while(p->next && p->next->next ){
        p = p->next;
    }
    int length = p->next->content_length;
    freecache(p->next);
    p->next = NULL;
    return length;
}


