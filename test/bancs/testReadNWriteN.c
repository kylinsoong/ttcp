/*
 *
 * How to run?
 *
 * Start the server via:
 *       
 *     ./a.out -s
 *
 * Start the client via:
 *
 *   ./a.out -c 000101234567890 
 *   ./a.out -c 00026abcdefghijklmnopqrstuvwxyz
 *   ./a.out -c 00036abcdefghijklmnopqrstuvwxyz1234567890
 *   ./a.out -c 00072abcdefghijklmnopqrstuvwxyz1234567890abcdefghijklmnopqrstuvwxyz1234567890
 *
 */

#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdarg.h>
#include <time.h>
#include <errno.h>
#include <signal.h>

int server = 0 ;
int msglen = 0 , len = 0;  /* message length*/

int sockfd;
int connfd;
short port = 8802;
struct sockaddr_storage ss_me, ss_from_conn, ss_him;
int connlen;

extern int optind;

char *data;

ssize_t  writen(int, const void *, size_t);
ssize_t  readn(int, void *, size_t);
int      extlength(void *);

char     *concat(const char *s1, const char *s2);

char Usage[] = "\
Usage: testReadNWriteN -c <message(length should large than 6, the first 5 is the data payload's length, the other is the real payload) write to server> \n\
       testReadNWriteN -s \n\
";

void     err_sys(const char *, ...);
void     out_sys(const char *, ...);

int main(int argc, char **argv) {

    if (argc < 2) goto usage;

    int c;
    while ((c = getopt(argc, argv, "cs")) != -1) {
        switch (c) {
        case 'c':
            server = 0;
            break;
        case 's':
            server = 1;
            break;
        default:
            goto usage;
        }
    }

    if(server) {

        if ( (sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)       
            err_sys("socket");
        
        memset(&ss_me, 0, sizeof(&ss_me));
        memset(&ss_from_conn, 0, sizeof(&ss_from_conn));

        ((struct sockaddr_in *)&ss_me)->sin_port = htons(port);

        if (bind(sockfd, (struct sockaddr *)&ss_me, sizeof(ss_me)) < 0)
            err_sys("bind");      

        if (listen(sockfd, 5) < 0)
            err_sys("listen");

        for(;;) {
            if ((connfd = accept(sockfd, (struct sockaddr *)&ss_from_conn, &connlen)) < 0)
                err_sys("accept");

            char header[5];
            readn(connfd, header, 5);

            int datalen = extlength(header);
            char message[datalen];
            readn(connfd, message, datalen);
            message[datalen] = '\0';

            char *total = concat(header, message);
            printf("receive message, header: %s, message: %s, total: %s, total length: %d\n", header, message, total, strlen(total));   
        
            if (close(connfd) == -1)
                err_sys("close");   

        }
    } else {
        data = malloc(4096);
        strcpy(data, argv[optind]);
        if(strlen(data) < 6) goto usage;
        
        char strmsglen[5];
        strncpy(strmsglen, data, 5);

        msglen = extlength(strmsglen);
        len = msglen + 5 ;

        if(strlen(data) != len) goto usage;

        struct addrinfo *dest;
        if(getaddrinfo("127.0.0.1", NULL, NULL, &dest)!=0)
            err_sys("badhostname");

        memset(&ss_him, 0, sizeof(&ss_him));
        memcpy((void*)&ss_him,(void*)dest->ai_addr, dest->ai_addrlen);

        ((struct sockaddr_in *)&ss_him)->sin_port = htons(port);

        if ( (sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
            err_sys("socket");

        if (connect(sockfd, (struct sockaddr *)&ss_him, sizeof(ss_him)) < 0)
            err_sys("connect");
        
        writen(sockfd, data, len);

        printf("send message: %s, msglen: %d, len: %d\n", data, msglen, len);   
    }


    return 1;

    usage:
        fprintf(stderr, Usage);
        exit(1);
}

void err_sys(const char *fmt, ...) {
   
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr,"error, errno=%d, msg=%s\n",  errno, fmt);
    va_end(ap);

    exit(0);
} 

void out_sys(const char *fmt, ...) {

    va_list ap;
    va_start(ap, fmt);
    printf("%s\n",  fmt);
    va_end(ap);
}

ssize_t writen(int fd, const void *vptr, size_t n)
{
        size_t          nleft;
        ssize_t         nwritten;
        const char      *ptr;

        ptr = vptr;
        nleft = n;
        while (nleft > 0) {
                if ( (nwritten = write(fd, ptr, nleft)) <= 0) {
                        if (nwritten < 0 && errno == EINTR)
                                nwritten = 0;           /* and call write() again */
                        else
                                return(-1);                     /* error */
                }

                nleft -= nwritten;
                ptr   += nwritten;
        }
        return(n);
}

ssize_t readn(int fd, void *vptr, size_t n)
{
        size_t  nleft;
        ssize_t nread;
        char    *ptr;

        ptr = vptr;
        nleft = n;
        while (nleft > 0) {
                if ( (nread = read(fd, ptr, nleft)) < 0) {
                        if (errno == EINTR)
                                nread = 0;              /* and call read() again */
                        else
                                return(-1);
                } else if (nread == 0)
                        break;                          /* EOF */

                nleft -= nread;
                ptr   += nread;
        }
        return(n - nleft);              /* return >= 0 */
}

int extlength(void *vptr)
{

    int i;
    int cur ;
    char    *strmsglen;

    strmsglen = vptr;
    cur = 0;
    for (i = 0 ; i < 5 ; i ++) {
        if(strmsglen[i] == '0' || strmsglen[i] == '-') {
            cur++ ;
        } else {
            break;
        }
    }
    char substrmsglen[5-cur];
    int index = 0;
    for(cur ; cur < 5 ; cur++)
        substrmsglen[index++] = strmsglen[cur];

    int msglen = atoi(substrmsglen);

    return msglen;
}

char *concat(const char *s1, const char *s2)
{   
    char *result = malloc(strlen(s1) + strlen(s2) + 1);
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

