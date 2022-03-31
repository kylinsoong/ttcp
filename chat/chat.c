/*
 *     C H A T . C
 *
 * The client server chat program. Makes a connection on 8878, 
 * chat messages from stdin.
 *
 * Created under Apache License Version 2.0 at Nov, 2021.
 *       Kylin Soong 
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
#define MAX 80

struct sockaddr_storage sinme;
struct sockaddr_storage sinhim;
struct sockaddr_storage frominet;

int domain, fromlen;
int fd;                         /* fd of network socket */
short port = 8878;              /* TCP port number */
char *host;			/* ptr to name of host */
short sPort;                    /* TCP source port number */
int server = 1;                 /* 0=client, 1=server */
int initiate = 0;

extern int errno;
extern int optind;
extern char *optarg;

char Usage[] = "\
Usage: chat -c [-options] host \n\
       chat -s [-options]\n\
Common options:\n\
        -p ##   port number to send to or listen at (default 8878)\n\
        -4      set IPv4 family for socket\n\
        -6      set IPv6 family for socket\n\
        -P ##   for -c, port number for client source port\n\
";

void err();
void mes();
void funcs();
void funcc();

int main(int argc, char **argv) {
  
    if (argc < 2) goto usage;
 
    int c;

    memset(&sinme, 0, sizeof(&sinme));
    memset(&sinhim, 0, sizeof(&sinhim));
    memset(&frominet, 0, sizeof(&frominet));

    while ((c = getopt(argc, argv, "46cp:P:s")) != -1) {
        switch (c) {
        case 'c':
            server = 0;
            break;
        case 's':
            server = 1;
            break;
        case '4':
            sinme.ss_family = AF_INET;
            break;
        case '6':
            sinme.ss_family = AF_INET6;
            break;
        case 'p':
            port = atoi(optarg);
            break;
        case 'P':
            sPort = atoi(optarg);
            break;
        default:
            goto usage;
        }
    }

    if(server == 0) {
        initiate = 1;
    }  

    if(initiate) {
        struct addrinfo *dest;

        if (optind == argc) {
            goto usage;
        }

        bzero((char *)&sinhim, sizeof(sinhim));
        host = argv[optind];

        if(getaddrinfo(host, NULL, NULL, &dest)!=0) {
            err("badhostname");
        }

        memcpy((void*)&sinhim,(void*)dest->ai_addr, dest->ai_addrlen);

        switch(sinhim.ss_family) {
        case AF_INET:
            ((struct sockaddr_in *)&sinhim)->sin_port = htons(port);
                  /* free choice */
            ((struct sockaddr_in *)&sinme)->sin_port = 0;
            break;

        case AF_INET6:
            ((struct sockaddr_in6 *)&sinhim)->sin6_port = htons(port);
                  /* free choice */
            ((struct sockaddr_in6 *)&sinme)->sin6_port = 0;
            break;
        }

        sinme.ss_family = sinhim.ss_family;

    } else {
        switch(sinme.ss_family) {
        case AF_INET:
            ((struct sockaddr_in *)&sinme)->sin_port = htons(port);
            break;

        case AF_INET6:
            ((struct sockaddr_in6 *)&sinme)->sin6_port = htons(port);
            break;

        default:
            fprintf(stderr, "chat server must specify address family: %u\n", sinme.ss_family);
            goto usage;
        }
    }

    if ((fd = socket(sinme.ss_family, SOCK_STREAM, 0)) < 0) {
        err("socket");
    }

    mes("socket");

    if(!initiate) {
        if (bind(fd, (struct sockaddr *)&sinme, sizeof(sinme)) < 0) {
            err("bind");
        }
    } 

    if (initiate) {

        if(connect(fd, (struct sockaddr *)&sinhim, sizeof(sinhim) ) < 0) {
            err("connect");
        }
                
        mes("connect");

        funcs(fd, host);
    } else {
        if ((listen(fd, 5)) != 0) {
            err("listen");
        }

        if((fd = accept(fd, (struct sockaddr *)&frominet, &fromlen) ) < 0) {
            err("accept");
        }

        {
            struct sockaddr_storage peer;
            int peerlen = sizeof(peer);
            char namebuf[256];
 
            if (getpeername(fd, (struct sockaddr *) &peer, &peerlen) < 0) {
                err("getpeername");
            }

            switch(peer.ss_family) {
            case AF_INET:
                inet_ntop(peer.ss_family, &((struct sockaddr_in *)&peer)->sin_addr, namebuf, 256);
                break;
            case AF_INET6:
                inet_ntop(peer.ss_family, &((struct sockaddr_in6 *)&peer)->sin6_addr, namebuf, 256);
                break;
            } 

            fprintf(stderr,"chat-s: accept from %s\n", namebuf);  

            funcs(fd, namebuf);
        }
    }

    usage:
        fprintf(stderr,Usage);
        exit(1);
}


void err(char *s)
{
    fprintf(stderr,"chat%s: ", server ? "-s" : "-c");
    perror(s);
    fprintf(stderr,"errno=%d\n",errno);
    exit(1);
}

void mes(char *s)
{
    fprintf(stderr, "chat%s: %s\n", server ? "-s" : "-c", s);
}

void funcs(int connfd, char *source)
{
    char buff[MAX];
    int n;

    for (;;) {

        bzero(buff, MAX);

        // read the message from client and copy it in buffer
        read(connfd, buff, sizeof(buff));
        fprintf(stderr,"%s: %s", source, buff);
        
        if (strncmp("exit", buff, 4) == 0) {
            exit(1);
        }

        bzero(buff, MAX);
        n = 0;

        printf("chat-s: ");
        while ((buff[n++] = getchar()) != '\n');

        // and send that buffer to client
        write(connfd, buff, sizeof(buff));
        
        if (strncmp("exit", buff, 4) == 0) {
            exit(1);
        }

    }    
}

void funcc(int sockfd, char *source)
{
    char buff[MAX];
    int n;

    for (;;) {

        bzero(buff, sizeof(buff));
        printf("chat-c: ");
        n = 0;
        while ((buff[n++] = getchar()) != '\n');

        write(sockfd, buff, sizeof(buff));
      
        if ((strncmp(buff, "exit", 4)) == 0) {
           exit(1);
        }

        bzero(buff, sizeof(buff));
        read(sockfd, buff, sizeof(buff));
        fprintf(stderr,"%s: %s", source, buff);

        if ((strncmp(buff, "exit", 4)) == 0) {
           exit(1);
        }
    }
}
