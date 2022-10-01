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
int msglen = 0 , len = 0; 

int fd, fd_bancs, fd_to_card, fd_to_bancs;  

int sockfd;
int connfd;
short port = 9805;
struct sockaddr_storage ss_me, ss_from_conn, ss_him;
int connlen;

void     err_sys(const char *, ...);
void     out_sys(const char *, ...);

extern int errno;
extern int optind;

int main(int argc, char **argv) {


  sleep(3);

 printf("start\n");

    while( 1 ) {
      printf("fd_to_bancs\n");
      sleep(1);
      if(fd_to_bancs > 0) {
          break;
      }
   }

    while(1) {
        printf("test/n");
        sleep(1);
    }

  printf("end\n");
 
    char *host = argv[optind];

    struct addrinfo *dest;
    if(getaddrinfo(host, NULL, NULL, &dest)!=0)
        err_sys("badhostname");

    memset(&ss_him, 0, sizeof(&ss_him));
    memcpy((void*)&ss_him,(void*)dest->ai_addr, dest->ai_addrlen);

    ((struct sockaddr_in *)&ss_him)->sin_port = htons(port);

    if ( (sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        err_sys("socket");

   printf("start connect\n");


    int conn = 1;
    while (conn) {
        if (connect(sockfd, (struct sockaddr *)&ss_him, sizeof(ss_him)) < 0) {
            sleep(5);
            printf("errno: %d\n", errno); 
        } else {
            break;
        }
        printf("re connect, connfd: %d\n", connfd);
    }

     printf("EINPROGRESS: %d, ETIMEDOUT: %d, ECONNREFUSED: %d, EHOSTUNREACH: %d, ENETUNREACH: %d\n", errno, EINPROGRESS, ETIMEDOUT, ECONNREFUSED, EHOSTUNREACH, ENETUNREACH);
    


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
