#include <stdio.h>
#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

void doit(int fd);
void clienterror(int fd, char *cause, char *errnum,
        char * shortmsg, char *longmsg);

int main(int argc, char** argv)
{
    int listenfd, connfd;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    /* Check command line args */
    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n",argv[0]);
        exit(1);
    }
    
    listenfd = Open_listenfd(argv[1]);
    while(1) {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        Getnameinfo((SA *)&clientaddr, clientlen, hostname, 
                MAXLINE, port, MAXLINE, 0);
        printf("Accept connection from (%sm %s)\n", hostname, port);
        doit(connfd);
        Close(connfd);
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
    while(( rc = Rio_readlineb(&rio_server, buf, MAXLINE)) > 0){
        //printf("%s", buf);
        Rio_writen(fd_l, buf, rc);
    }
    Close(fd_r);
    //printf("close do it \n");
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
