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
#define PORT 8878
#define SA struct sockaddr

struct sockaddr_storage sinme;

int fd;                         /* fd of network socket */
short port = 8878;              /* TCP port number */
short sPort;                    /* TCP source port number */
int server = 1;                 /* 0=client, 1=server */
int initiate = 0;

extern int errno;
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

int main(int argc, char **argv) {
  
    if (argc < 2) goto usage;
 
    int c;

    memset(&sinme, 0, sizeof(&sinme));

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
        printf("chat client connect()\n");
    } else {

        printf("chat server accept()\n");
 
        switch(sinme.ss_family) {
        case AF_INET:
            ((struct sockaddr_in *)&sinme)->sin_port = htons(port);
            break;

        case AF_INET6:
            ((struct sockaddr_in6 *)&sinme)->sin6_port = htons(port);
            break;

        default:
            fprintf(stderr, "chat server must specify address family: %u\n", sinme.ss_family);
        }
    }

    if ((fd = socket(sinme.ss_family, SOCK_STREAM, 0)) < 0) {
        err("socket");
    }

    mes("socket");

    printf("%d, %d, %d, %d \n", server, port, sPort, initiate);
    exit(0);

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
