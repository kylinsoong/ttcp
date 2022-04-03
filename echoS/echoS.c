/*
 *        E C H O S . C
 *
 * echoS - display a line of text over socket stream.
 *
 * Initial created in 2008 at CUG(China University of Geoscience). 
 *     Kylin Soong
 *
 * Modified in 2022 for IPv6 support.
 *     Kylin Soong
 *   Pv6 support
 *   Client source port binding ability.
 *  
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

#define STR_DATTIME "daytime"
#define STR_TIME    "time"
#define STR_CHARGEN "chargen"
#define STR_ENTER   "\n"

#define ASCII_START 32
#define ASCII_END   126
#define LISTENQ     1024    /* 2nd argument to listen() */
#define MAXLINE     4096    /* max text line length */
#define BUFSIZE     1024


struct sockaddr_storage sinme;
struct sockaddr_storage sinhim;
struct sockaddr_storage frominet;

int domain, fromlen;
int fd;                         /* fd of network socket */
int connfd;
short port = 8877;              /* TCP port number */
char *host;                     /* ptr to name of host */
short sPort = 0;                    /* TCP source port number */
int server = 1;                 /* 0=client, 1=server */
int initiate = 0;

extern int errno;
extern int optind;
extern char *optarg;

char Usage[] = "\
Usage: echoS -c [-options] host \n\
       ecohS -s [-options]\n\
Common options:\n\
        -p ##   port number to send to or listen at (default 8877)\n\
        -4      set IPv4 family for socket\n\
        -6      set IPv6 family for socket\n\
        -P ##   for -c, port number for client source port\n\
";

/* Socket */
int      Socket(int, int, int);
void     Bind(int, const struct sockaddr *, socklen_t);
void     Listen(int, int);
int      Accept(int, struct sockaddr *, socklen_t *);
void     Connect(int, const struct sockaddr *, socklen_t);

/* IO */
void     Close(int);
void     err_sys(const char *, ...);
void     out_sys(const char *, ...);
char     *Fgets(char *, int, FILE *);
void     Fputs(const char *, FILE *);
char     *concat(const char *s1, const char *s2);
char     *randstring(int size);

pid_t    Fork(void);

void     Writen(int, void *, size_t);
ssize_t  writen(int, const void *, size_t);

ssize_t  Readline(int, void *, size_t);
ssize_t  readline(int, void *, size_t);
ssize_t  my_read(int fd, char *ptr);

/* Core logic entries*/
void     Str_echo(int);
void     Str_cli(FILE *, int);


int main(int argc, char **argv) 
{
    if (argc < 2) goto usage;
    
    pid_t              childpid;
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
            err_sys("badhostname");
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

        if(sPort > 0) {
            switch(sinme.ss_family) {
            case AF_INET:
                ((struct sockaddr_in *)&sinme)->sin_port = htons(sPort);
                break;
            case AF_INET6:
                ((struct sockaddr_in6 *)&sinme)->sin6_port = htons(sPort);
                break;
            default:
                break;
            }
        }

    } else {
        switch(sinme.ss_family) {
        case AF_INET:
            ((struct sockaddr_in *)&sinme)->sin_port = htons(port);
            break;

        case AF_INET6:
            ((struct sockaddr_in6 *)&sinme)->sin6_port = htons(port);
            break;

        default:
            out_sys( "must specify address family");
            goto usage;
        }
    }

    fd = Socket(sinme.ss_family, SOCK_STREAM, 0);
    out_sys("socket");

    if(!initiate || sPort > 0) {

        Bind(fd, (struct sockaddr *)&sinme, sizeof(sinme));
        out_sys("bind");
    }

    if (initiate) {

        Connect(fd, (struct sockaddr *)&sinhim, sizeof(sinhim));
        out_sys("connect");
        Str_cli(stdin, fd);

    } else {

        Listen(fd, 5);

        for(;;) {

            connfd = Accept(fd, (struct sockaddr *)&frominet, &fromlen);

            {
                struct sockaddr_storage peer;
                int peerlen = sizeof(peer);
                char namebuf[256];
                uint16_t port;

                if (getpeername(connfd, (struct sockaddr *) &peer, &peerlen) < 0) {
                    err_sys("getpeername");
                }

                switch(peer.ss_family) {
                case AF_INET:
                    inet_ntop(peer.ss_family, &((struct sockaddr_in *)&peer)->sin_addr, namebuf, 256);
                    port = ntohs(((struct sockaddr_in *)&peer)->sin_port);
                    break;
                case AF_INET6:
                    inet_ntop(peer.ss_family, &((struct sockaddr_in6 *)&peer)->sin6_addr, namebuf, 256);
                    port = ntohs(((struct sockaddr_in6 *)&peer)->sin6_port);
                    break;
                }

                fprintf(stderr,"echoS: accept from %s:%u\n", namebuf, port);
            }

            if ((childpid = Fork()) == 0) {
                Close(fd); /* for address bad file descriptor */
                Str_echo(connfd);
                exit(0);
            }

            Close(connfd);
        }
    }

    return 1;

    usage:
        fprintf(stderr,Usage);
        exit(1);
}

int Socket(int family, int type, int protocol)
{
    int listen_sock;

    if ( (listen_sock = socket(family, type, protocol)) < 0)
        err_sys("socket");

    return (listen_sock);
}

void Bind(int fd, const struct sockaddr *sa, socklen_t salen)
{
    if (bind(fd, sa, salen) < 0)
        err_sys("bind");
}

void Connect(int fd, const struct sockaddr *sa, socklen_t salen)
{
    if (connect(fd, sa, salen) < 0)
        err_sys("connect");
}

void Listen(int fd, int backlog)
{
    char *ptr;

    if ((ptr = getenv("LISTENQ")) != NULL) { /*4can override 2nd argument with environment variable */
        backlog = atoi(ptr);
    }

    if (listen(fd, backlog) < 0)
        err_sys("listen");
}

int Accept(int fd, struct sockaddr *sa, socklen_t *salenptr)
{
    int sock;

    if ((sock = accept(fd, sa, salenptr)) < 0)
        err_sys("accept");

    return (sock);
}

void Close(int fd)
{
    if (close(fd) == -1)
        err_sys("close error");
}

void err_sys(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    fprintf(stderr,"echoS: ");
    perror(fmt);
    fprintf(stderr,"errno=%d\n",errno);
    va_end(ap);

    exit(1);
}

void out_sys(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    printf("echoS: %s\n", fmt);
    va_end(ap);
}

char *Fgets(char *ptr, int n, FILE *stream)
{
    char    *rptr;

    if ( (rptr = fgets(ptr, n, stream)) == NULL && ferror(stream))
        err_sys("fgets error");

    return (rptr);
}

void Fputs(const char *ptr, FILE *stream)
{
    if (fputs(ptr, stream) == EOF)
        err_sys("fputs error");
}

char *concat(const char *s1, const char *s2)
{
    char *result = malloc(strlen(s1) + strlen(s2) + 1);
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

char *randstring(int size)
{
    int i;
    char *res = malloc(size + 1);
    for(i = 0; i < size; i++) {
        res[i] = (char) (rand() % (ASCII_END - ASCII_START)) + ASCII_START;
    }
    res[i] = '\0';
    return res;
}

pid_t Fork(void)
{
    pid_t pid;

    if ((pid = fork()) == -1)
        err_sys("fork error");
    return (pid);
}

void Writen(int fd, void *ptr, size_t nbytes){

    if (writen(fd, ptr, nbytes) != nbytes)
        err_sys("writen error");
}

ssize_t writen(int fd, const void *vptr, size_t n)
{
    size_t     nleft;
    ssize_t    nwritten;
    const char *ptr;

    ptr = vptr;
    nleft = n;

    while (nleft > 0) {
        if ((nwritten = write(fd, ptr, nleft)) <= 0) {
            if (nwritten < 0 && errno == EINTR)
                nwritten = 0;
            else
                return (-1);
        }
        nleft -= nwritten;
        ptr   += nwritten;
    }

    return (n);
}

ssize_t Readline(int fd, void *ptr, size_t maxlen)
{
    ssize_t n;

    if ( (n = readline(fd, ptr, maxlen)) < 0)
        err_sys("readline error");

    return (n);
}

ssize_t readline(int fd, void *vptr, size_t maxlen)
{
    ssize_t n, rc;
    char    c, *ptr;
    char    read_buf[MAXLINE];

    ptr = vptr;
    for (n = 1; n < maxlen; n++) {
        if ( (rc = my_read(fd, &c)) == 1) {
            *ptr++ = c;
            if (c == '\n')
                break;
        } else if (rc == 0) {
            *ptr = 0;
            return(n - 1);
        } else {
            return(-1);
        }
    }

    *ptr = 0;
    return(n);
}

ssize_t my_read(int fd, char *ptr)
{

    int      read_cnt;
    char     *read_ptr;
    char     read_buf[MAXLINE];

    if (read_cnt <= 0) {
        again:
        if ( (read_cnt = read(fd, read_buf, sizeof(read_buf))) < 0) {
            if (errno == EINTR)
                goto again;
        } else if (read_cnt == 0) {
            return(0);
        }

        read_ptr = read_buf;
    }

    read_cnt--;
    *ptr = *read_ptr++;
    return(1);
}


void Str_echo(int sockfd)
{
    ssize_t         n;
    char            line[MAXLINE], sendBuff[BUFSIZE];
    time_t          ticks;

    for(;;) {

        n = Readline(sockfd, line, MAXLINE);

        if(n < 0) {
            err_sys("read error");
        } else if (n == 0) {
            out_sys("connection closed by other end");
            return;
        }

        if(strcmp(line, concat(STR_DATTIME, STR_ENTER)) == 0) {
            ticks = time(NULL);
            snprintf(sendBuff, sizeof(sendBuff), "%.24s\r\n", ctime(&ticks));
            write(sockfd, sendBuff, strlen(sendBuff));
        } else if(strcmp(line, concat(STR_TIME, STR_ENTER)) == 0) {
            time_t now = time(0);
            sprintf (sendBuff, "%lu\n" , now);
            write(sockfd, sendBuff, strlen(sendBuff));
        } else if (strcmp(line, concat(STR_CHARGEN, STR_ENTER)) == 0) {
            snprintf(sendBuff, sizeof(sendBuff), "%s\n", randstring(48));
            write(sockfd, sendBuff, strlen(sendBuff));
        } else{
            Writen(sockfd, line, n);
        }
    }
}

void Str_cli(FILE *fp, int sockfd)
{
    char sendline[MAXLINE], recvline[MAXLINE];

    while (Fgets(sendline, MAXLINE, fp) != NULL) {

        Writen(sockfd, sendline, strlen(sendline));

        if (Readline(sockfd, recvline, MAXLINE) == 0)
            err_sys("server terminated prematurely");

        Fputs(recvline, stdout);
    }
}
