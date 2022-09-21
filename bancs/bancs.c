/*
 *        B A N C S . C
 *
 * Initial created in 2022.09.
 *             Kylin Soong
 *
 * This is sample program for simulate Core Bank System.
 * 
 *  _______
 * |       |
 * |ESB(MQ)| 
 * |       |
 *  -------  
 *     | request
 *     |
 *     |
 *  -------  request       _______
 * |       |--------------|       |
 * | BANCS |              |  CARD |
 * |       |--------------|       |
 *  -------     response   -------
 *
 * 1. BANCS listen on 9805, 8805, CARD listen on 8806, BANCS connect to CARD via 8806(mark as connection 1), CARD connect to BANCS via 8805(mark as connection 2)
 * 2. A client simulate ESB send the request message to BANCS via 9902 which listened by BANCS
 * 2. BANCS forward request message to CARD
 * 3. CARD send response message to BANCS
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

#define STR_DATTIME "daytime"
#define STR_TIME    "time"
#define STR_CHARGEN "chargen"
#define STR_ENTER   "\n"

#define ASCII_START 32
#define ASCII_END   126
#define MAXLINE     4096    /* max text line length */
#define BUFSIZE     1024

typedef void    Sigfunc(int);   /* for signal handlers */

struct sockaddr_storage sinme;
struct sockaddr_storage sinhim;
struct sockaddr_storage frominet;

int domain, fromlen;
int fd;                         /* fd of network socket */
int connfd;

short port = 8805;
short cport = 8806;               /* TCP port number */
short inport = 9805; 

char *host;                     /* ptr to name of host */
int server = 1;                 /* 0=client, 1=server */
int initiate = 0;

extern int errno;
extern int optind;
extern char *optarg;

char Usage[] = "\
Usage: bancs -e [-options] <host of BANCS> \n\
       bancs -b [-options] <host of CARD>\n\
       bancs -c [-options] <host of BANCS>\n\
Common options:\n\
        -p ##   port number to send to or listen at (default 8805/8806 9805)\n\
";

/* Socket */
int      Socket(int, int, int);
void     Bind(int, const struct sockaddr *, socklen_t);
void     Listen(int, int);
int      Accept(int, struct sockaddr *, socklen_t *);
void     Connect(int, const struct sockaddr *, socklen_t);
char     *Getpeername(int);

/* IO */
void     Close(int);
void     err_sys(const char *, ...);
void     out_sys(const char *, ...);
char     *Fgets(char *, int, FILE *);
void     Fputs(const char *, FILE *);
char     *concat(const char *s1, const char *s2);
char     *randstring(int size);

pid_t    Fork(void);
void     sig_chld(int);
Sigfunc  *Signal(int, Sigfunc *);

void     Writen(int, void *, size_t);
ssize_t  writen(int, const void *, size_t);

ssize_t  Readline(int, void *, size_t);
ssize_t  readline(int, void *, size_t);
ssize_t  my_read(int fd, char *ptr);

/* Core logic entries*/
void     Str_echo(int, const char *);
void     Str_cli(FILE *, int);
void     Str_puts(int);

int main(int argc, char **argv) 
{
    if (argc < 2) goto usage;
    
    pid_t              childpid;
    int c;

    memset(&sinme, 0, sizeof(&sinme));
    memset(&sinhim, 0, sizeof(&sinhim));
    memset(&frominet, 0, sizeof(&frominet));

    while ((c = getopt(argc, argv, "ebcp:")) != -1) {
        switch (c) {
        case 'e':
            server = 0;
            break;
        case 'b':
            server = 1;
            break;
        case 'c':
            server = 2;
            break;
        case 'p':
            port = atoi(optarg);
            cport = port + 1;
            inport = port + 1000; 
            break;
        default:
            goto usage;
        }
    }

    if (optind == argc) {
            goto usage;
    }
    host = argv[optind];

    if(server == 0) {

        out_sys("start");

    } else if(server == 1) {

        out_sys("start");

    } else if(server == 2) {

        out_sys("start");
    }

   printf("mode: %d, port: %d, cport: %d, inport: %d, host: %s\n", server, port, cport, inport,  host);

   exit(0);

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

    if (initiate) {

        Connect(fd, (struct sockaddr *)&sinhim, sizeof(sinhim));
        out_sys("connect");
        Str_cli(stdin, fd);

    } else {

        Listen(fd, 5);

        Signal(SIGCHLD, sig_chld);

        for(;;) {

            connfd = Accept(fd, (struct sockaddr *)&frominet, &fromlen);

            char *peer = Getpeername(connfd);
            out_sys(concat("accept from ", peer));

            if ((childpid = Fork()) == 0) {
                Close(fd); /* for address bad file descriptor */
                Str_echo(connfd, peer);
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

char *Getpeername(int connfd)
{
    char      buf[300];
    char      namebuf[256];
    in_port_t port;
 
    struct sockaddr_storage peer;
    int peerlen = sizeof(peer);

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

    snprintf(buf, sizeof(buf), "%s:%u", namebuf, port);
    char *result = malloc(strlen(buf) +1);
    strcpy(result, buf);

    return result;
}

void Close(int fd)
{
    if (close(fd) == -1)
        err_sys("close");
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

    char* role = "CLIENT";
    if(server == 0)
        role = "CLIENT";
    else if (server == 1)
        role = "BANCS";
    else if (server == 2)
        role = "CARD";

    char cur_time[128];
    struct tm*  ptm;
    time_t now = time(NULL);
    ptm = localtime(&now);
    strftime(cur_time, 128, "%d-%b-%Y %H:%M:%S", ptm);

    printf("%s (%d) %s: %s\n", cur_time, getpid(), role, fmt);
    va_end(ap);
}

char *Fgets(char *ptr, int n, FILE *stream)
{
    char    *rptr;

    if ( (rptr = fgets(ptr, n, stream)) == NULL && ferror(stream))
        err_sys("fgets");

    return (rptr);
}

void Fputs(const char *ptr, FILE *stream)
{
    if (fputs(ptr, stream) == EOF)
        err_sys("fputs");
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

void sig_chld(int signo)
{
    pid_t   pid;
    int     stat;

    while ( (pid = waitpid(-1, &stat, WNOHANG)) > 0) {
       char buf[30];
       snprintf(buf, sizeof(buf), "child %d terminated", pid);    
       out_sys(buf);
    }
    
    return;
}

pid_t Fork(void)
{
    pid_t pid;

    if ((pid = fork()) == -1)
        err_sys("fork");
    return (pid);
}

Sigfunc * Signal(int signo, Sigfunc *func)        /* for our signal() function */
{
    Sigfunc *sigfunc;

    if ( (sigfunc = signal(signo, func)) == SIG_ERR)
            err_sys("signal");
    return(sigfunc);
}

void Writen(int fd, void *ptr, size_t nbytes){

    if (writen(fd, ptr, nbytes) != nbytes)
        err_sys("writen");
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
        err_sys("readline");

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


void Str_echo(int connfd, const char *peer)
{
    ssize_t         n;
    char            line[MAXLINE], sendBuff[BUFSIZE];
    time_t          ticks;

    out_sys(concat("server ready, peer: ", peer));

    for(;;) {

        n = Readline(connfd, line, MAXLINE);

        if(n < 0) {
            err_sys("read");
        } else if (n == 0) {
            out_sys(concat("connection closed by ", peer));
            return;
        }

        if(strcmp(line, concat(STR_DATTIME, STR_ENTER)) == 0) {
            ticks = time(NULL);
            snprintf(sendBuff, sizeof(sendBuff), "%.24s\r\n", ctime(&ticks));
            write(connfd, sendBuff, strlen(sendBuff));
        } else if(strcmp(line, concat(STR_TIME, STR_ENTER)) == 0) {
            time_t now = time(0);
            sprintf (sendBuff, "%lu\n" , now);
            write(connfd, sendBuff, strlen(sendBuff));
        } else if (strcmp(line, concat(STR_CHARGEN, STR_ENTER)) == 0) {
            for(;;) {
                snprintf(sendBuff, sizeof(sendBuff), "%s\n", randstring(999));
                write(connfd, sendBuff, strlen(sendBuff));
                sleep(1);
            }
        } else{
            Writen(connfd, line, n);
        }
    }
}

void Str_cli(FILE *fp, int sockfd)
{
    char sendline[MAXLINE];

    while (Fgets(sendline, MAXLINE, fp) != NULL) {

        Writen(sockfd, sendline, strlen(sendline));

        if (strcmp(sendline, concat(STR_CHARGEN, STR_ENTER)) == 0) {
            for(;;) {
                Str_puts(sockfd);
            }
        }

        Str_puts(sockfd);
    }
}

void Str_puts(int sockfd)
{
    char recvline[MAXLINE];

    if (Readline(sockfd, recvline, MAXLINE) == 0)
        err_sys("server terminated prematurely");
    
    Fputs(recvline, stdout);
} 
