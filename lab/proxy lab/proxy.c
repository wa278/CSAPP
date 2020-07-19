#include <stdio.h>
#include "csapp.h"
#include "sbuf.h"
#include "cache.h"
/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400
#define NTHREADS 10
#define SBUFSIZE 20

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";
sbuf_t sbuf;
cache_t* root = NULL;
rw_t rw ;
void doit(int fd);
void clienterror(int fd, char *cause, char *errnum,
        char * shortmsg, char *longmsg);
void*  thread(void* vargp);

int main(int argc, char** argv)
{
    int i, listenfd, connfd;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    pthread_t tid;
    /* Check command line args */
    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n",argv[0]);
        exit(1);
    }

    for(i = 0; i < NTHREADS; i++){
        Pthread_create(&tid, NULL, thread, NULL);
    }
    sbuf_init(&sbuf, SBUFSIZE);
    rw_init(&rw);
    listenfd = Open_listenfd(argv[1]);
    while(1) {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        Getnameinfo((SA *)&clientaddr, clientlen, hostname, 
                MAXLINE, port, MAXLINE, 0);
        printf("Accept connection from (%sm %s)\n", hostname, port);
        sbuf_insert(&sbuf, connfd);
    }
}

void doit(int fd)
{
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char host[MAXLINE] = "";
    char port[MAXLINE] = "";
    rio_t rio_client;
    rio_t rio_server;
    int fd_l = fd;
    int fd_r;

    int find_cache = 0;
    Rio_readinitb(&rio_client, fd_l);
    
    if (!Rio_readlineb(&rio_client, buf, MAXLINE))
        return;
    //printf("%s", buf);
    sscanf(buf, "%s %s %s", method, uri, version);
    if (strcasecmp(method, "GET")) {
        clienterror(fd, method, "501", "Not Implemented",
                "Proxy does not implement this method");
        return;
    }
    char* Uri = uri;
    if(strstr(Uri, "http://")){
        Uri = Uri+strlen("http://");
    }
    sscanf(Uri,"%[^/]%s", buf, Uri);
    if(strlen(buf) == 0){
        if (!Rio_readlineb(&rio_client, buf, MAXLINE))
            return;
        sscanf(buf, "Host:%[^:]:%s", host, port);
    }else{
        sscanf(buf,"%[^:]:%s", host, port);
        if(strlen(port) == 0){
            strcpy(port,"80");
        }
    }

    if (strlen(port) == 0 || strlen(host) == 0){
        clienterror(fd, method, "501", "Host or Port Error", "");
        return;
    }
    P(&rw.mutex);
    rw.readcnt++;
    if(rw.readcnt == 1){
    P(&rw.w);
    }
    V(&rw.mutex);
    cache_t* c = findcache(root, host, port, Uri);
    if (c){
        Rio_writen(fd_l, c->content, c->content_length);
        find_cache = 1;
    }
    P(&rw.mutex);
    rw.readcnt--;
    if(rw.readcnt == 0)
        V(&rw.w);
    V(&rw.mutex);
    
    if (find_cache){
        P(&rw.w);
        cache_t* c = findcache(root, host, port, Uri);
        if(c){
            movefirst(&root, c);
        }
        V(&rw.w);
    }else{
        fd_r = Open_clientfd(host, port);
        Rio_readinitb(&rio_server, fd_r);
        
        sprintf(buf,"GET %s HTTP/1.0 \r\n", Uri);
        Rio_writen(fd_r, buf, strlen(buf));
        sprintf(buf,"Host:%s:%s\r\n", host, port);
        Rio_writen(fd_r, buf, strlen(buf));
        Rio_writen(fd_r, user_agent_hdr, strlen(user_agent_hdr));
        sprintf(buf, "Connection: close\r\n");
        Rio_writen(fd_r, buf, strlen(buf));
        sprintf(buf, "Proxy-Connection: close\r\n");
        Rio_writen(fd_r, buf, strlen(buf));
        sprintf(buf, "\r\n");
        Rio_writen(fd_r, buf, strlen(buf));
        int rc = 0;
        int num = 0;
        char* content = (char*)malloc(MAX_OBJECT_SIZE);
        char* p = content;
        while(( rc = Rio_readlineb(&rio_server, buf, MAXLINE)) > 0){
            //printf("%s", buf);
            Rio_writen(fd_l, buf, rc);
            if (num + rc <= MAX_OBJECT_SIZE){
                memcpy(p, buf, rc);
                p = p + rc;
            }
            num += rc;
        }
        if (num <= MAX_OBJECT_SIZE) {
            char* Content = (char*)malloc(num);
            memcpy(Content, content, num);
            cache_t* cache = (cache_t*)malloc(sizeof(cache_t));
            setcache(cache, host, port, Uri, Content, num);
            P(&rw.w);
            if (rw.readcount + num <= MAX_CACHE_SIZE ){
                addcache(cache, &root);
                rw.readcount += num;
            }else{
                while(rw.readcount + num > MAX_CACHE_SIZE){
                    int len = removelast(&root);
                    rw.readcount -= len;
                }
                addcache(cache,&root);
                rw.readcount += num;
            }
            V(&rw.w);
        }
        Free(content);
    }

   /* fd_r = Open_clientfd(host, port);
    Rio_readinitb(&rio_server, fd_r);
    
    sprintf(buf,"GET %s HTTP/1.0 \r\n", Uri);
    Rio_writen(fd_r, buf, strlen(buf));
    sprintf(buf,"Host:%s:%s\r\n", host, port);
    Rio_writen(fd_r, buf, strlen(buf));
    Rio_writen(fd_r, user_agent_hdr, strlen(user_agent_hdr));
    sprintf(buf, "Connection: close\r\n");
    Rio_writen(fd_r, buf, strlen(buf));
    sprintf(buf, "Proxy-Connection: close\r\n");
    Rio_writen(fd_r, buf, strlen(buf));
    sprintf(buf, "\r\n");
    Rio_writen(fd_r, buf, strlen(buf));
    int rc = 0;
    int num = 0;
    char* content = (char*)malloc(MAX_OBJECT_SIZE);
    char* p = content;
    while(( rc = Rio_readlineb(&rio_server, buf, MAXLINE)) > 0){
        //printf("%s", buf);
        Rio_writen(fd_l, buf, rc);
        if (num + rc <= MAX_OBJECT_SIZE){
            memcpy(p, buf, rc);
            p = p + rc;
        }
        num += rc;
    }
    if (num <= MAX_OBJECT_SIZE) {
        char* Content = (char*)malloc(num);
        memcpy(Content, content, num);
        cache_t* cache = (cache_t*)malloc(sizeof(cache_t));
        setcache(cache, host, port, Uri, Content, num);
        P(&rw.w);
        if (rw.readcount + num <= MAX_CACHE_SIZE ){
            addcache(cache, root);
            rw.readcount += num;
        }else{
            while(rw.readcount + num > MAX_CACHE_SIZE){
                removelast(root);
                rw.readcount -= cache->content_length;
            }
            addcache(cache,root)
        }
        V(&rw.w)
    }
    Free(content)*/
    Close(fd_r);
    printf("close do it \n");
}

void* thread(void* vargp) {
    Pthread_detach(Pthread_self());
    while(1){
        int connfd = sbuf_remove(&sbuf);
        doit(connfd);
        Close(connfd);
    }
    return NULL;
}


/*
 *clienterror - returns an error message to the client 
 */
/* $begin client error */
void clienterror(int fd, char *cause, char* errnum,
        char* shortmsg, char* longmsg)
{
    char buf[MAXLINE];
    /* Print the HTTP response headers */
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n\r\n");
    Rio_writen(fd, buf, strlen(buf));

    /* Print the HTTP response body*/
    sprintf(buf, "<html><title> Proxy Error</title>");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<body bgcolor=""ffffff"">\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "%s: %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<p>%s: %s\r\n", longmsg, cause);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<hr><em> The Proxy Server</em>\r\n");
    Rio_writen(fd, buf, strlen(buf));
}
/* $end clienterror */
