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
 *
 * DESIGN PRINCIPLES:
 *
 * 1. Client ESB load the ISO8583 Message from '/etc/bancs.data', which the file keep bunch of Messages, one message per linem, and send to Bancs via 1 connection.
 * 2. All Handler contains a listened entrypoint, and started as a separate threads  
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
#include <signal.h>
#include <sys/mman.h>


#define MAX_DATA_LINE 4096

typedef void    Sigfunc(int);   /* for signal handlers */

struct sockaddr_storage esb_to_bancs, bancs_esb_ss, bancs_from_esb, bancs_card_ss, bancs_from_card, card_bancs_ss, card_from_bancs, bancs_to_card, card_to_bancs;

int domain, fromlen, bancs_from_esb_len, bancs_from_card_len, card_from_bancs_len;
int fd, fd_bancs, fd_to_card, fd_to_bancs;                 /* fd of network socket */
int connfd, connfd_bancs;

static int *glob_fd_to_card;

int lazy = 2;

short port = 8805;
short cport = 8806;               /* TCP port number */
short inport = 9805; 

char *host;                     /* ptr to name of host */
int server = 1;                 /* 0=esb, 1=bancs, 3=card */
int debug = 0 ;


FILE * fp;
char bufr[MAX_DATA_LINE];

extern int errno;
extern int optind;
extern char *optarg;

char Usage[] = "\
Usage: bancs -e [-options] <host of BANCS> \n\
       bancs -b [-options] <host of CARD>\n\
       bancs -c [-options] <host of BANCS>\n\
Common options:\n\
        -d      enable debug logging\n\
        -l ##   the length of lazy sock write/read time (default 2 seconds)\n\
        -p ##   port number to send to or listen at (default 8805/8806 9805)\n\
";

/* Socket */
int      Socket(int, int, int);
void     Bind(int, const struct sockaddr *, socklen_t);
void     Listen(int, int);
int      Accept(int, struct sockaddr *, socklen_t *);
void     Connect(int, const struct sockaddr *, socklen_t);
char     *Getpeername(int);

void     InboundHandler(void);
void     BancsFromCardHandler(void);

/* IO */
void     Close(int);
void     err_sys(const char *, ...);
void     out_sys(const char *, ...);
char     *Fgets(char *, int, FILE *);
void     Fputs(const char *, FILE *);
char     *concat(const char *s1, const char *s2);

pid_t    Fork(void);
void     sig_chld(int);
Sigfunc  *Signal(int, Sigfunc *);


void     Writen(int, void *, size_t);
ssize_t  Readn(int, void *, size_t);
ssize_t  writen(int, const void *, size_t);
ssize_t  readn(int, void *, size_t);
int      extlength(void *);


/*
void     Str_puts(int);
*/

int main(int argc, char **argv) 
{
    if (argc < 2) goto usage;

    // parse the main argv
    int c;
    while ((c = getopt(argc, argv, "ebcdl:p:")) != -1) {
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
        case 'd':
            debug = 1;
            break;
        case 'l':
            lazy = atoi(optarg);
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

    struct addrinfo *dest;
    if(getaddrinfo(host, NULL, NULL, &dest)!=0) 
      err_sys("badhostname");
   
    memset(&esb_to_bancs, 0, sizeof(&esb_to_bancs));
    memcpy((void*)&esb_to_bancs,(void*)dest->ai_addr, dest->ai_addrlen);
    ((struct sockaddr_in *)&esb_to_bancs)->sin_port = htons(inport);

    fd = Socket(AF_INET, SOCK_STREAM, 0);
    out_sys("socket");

    Connect(fd, (struct sockaddr *)&esb_to_bancs, sizeof(esb_to_bancs));
    out_sys(concat("connect to bancs ", host));

    if((fp = fopen("/etc/bancs.data","r")) != NULL) {
        while(fgets(bufr, MAX_DATA_LINE, fp) != NULL) {
    
            if (strncmp("#", bufr, strlen("#")) == 0 || strlen(bufr) < 6)
                continue;

            strtok(bufr, "\n");

            char strmsglen[5];
            strncpy(strmsglen, bufr, 5);
            int len = extlength(strmsglen) + 5;

            if(strlen(bufr) != len) {
                out_sys(concat("Invalid message, message length is not equals header defined length, message: ", bufr));
                continue;             
            } 
            Writen(fd, bufr, len);
            out_sys(concat("send message to bancs, message: ", bufr));
            sleep(lazy);
        }        
    } else {
        err_sys("file 'bancs.data' not exist");
    }

    out_sys("exit"); 

  } else if(server == 1) {

    out_sys("start");

    glob_fd_to_card = mmap(NULL, sizeof *glob_fd_to_card, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    Signal(SIGCHLD, sig_chld);
    int rc = fork();
    if (rc < 0 ) {
      err_sys("Error:unable to create thread");
    } else if ( rc == 0) {
      //InboundHandler();
      BancsFromCardHandler();
    } else {

        Signal(SIGCHLD, sig_chld);
        int src = fork();
        if(src < 0) {
            err_sys("Error:unable to create thread");
        } else if (src == 0) {
            //BancsFromCardHandler();
            InboundHandler();
        } else {


            sleep(lazy * 5);

            struct addrinfo *dest;
            if(getaddrinfo(host, NULL, NULL, &dest)!=0)
                err_sys("badhostname");

            memset(&bancs_to_card, 0, sizeof(&bancs_to_card));
            memcpy((void*)&bancs_to_card,(void*)dest->ai_addr, dest->ai_addrlen);

            ((struct sockaddr_in *)&bancs_to_card)->sin_port = htons(cport);

            fd_to_card = Socket(AF_INET, SOCK_STREAM, 0);
            if(debug) out_sys("socket");

            Connect(fd_to_card, (struct sockaddr *)&bancs_to_card, sizeof(bancs_to_card));
            out_sys(concat("connect to card ", host));
           
            *glob_fd_to_card = fd_to_card ;

            int subpid, status;
            while((subpid = waitpid(-1, &status, 0)) > 0) {
                char str[80];
                sprintf(str, "process %d terminated, errno: %d", subpid, errno);
                out_sys(str);
            }

        }           


     } 

  } else if(server == 2) {

    out_sys("start");

    memset(&card_bancs_ss, 0, sizeof(&card_bancs_ss));
    memset(&card_from_bancs, 0, sizeof(&card_from_bancs));

    ((struct sockaddr_in *)&card_bancs_ss)->sin_port = htons(cport);
    fd = Socket(AF_INET, SOCK_STREAM, 0);
    out_sys("socket");

    Bind(fd, (struct sockaddr *)&card_bancs_ss, sizeof(card_bancs_ss));
    out_sys("bind");

    Listen(fd, 5);
    out_sys("listen");

    connfd = Accept(fd, (struct sockaddr *)&card_from_bancs, &card_from_bancs_len);
    char *peer = Getpeername(connfd);
    out_sys(concat("conn from bancs ", peer));

    sleep(lazy);   

    // connect to bancs
    struct addrinfo *dest;
    if(getaddrinfo(host, NULL, NULL, &dest)!=0)
      err_sys("badhostname");

    memset(&card_to_bancs, 0, sizeof(&card_to_bancs));
    memcpy((void*)&card_to_bancs,(void*)dest->ai_addr, dest->ai_addrlen);

    ((struct sockaddr_in *)&card_to_bancs)->sin_port = htons(port);

    fd_to_bancs = Socket(AF_INET, SOCK_STREAM, 0);
    out_sys("socket");

    Connect(fd_to_bancs, (struct sockaddr *)&card_to_bancs, sizeof(card_to_bancs));
    out_sys(concat("connect to bancs ", host));

    // handle bancs request message
    for(;;) {

      char header[5];
      Readn(connfd, header, 5);
      header[5] = '\0';
      int datalen = extlength(header);

/*
      if(datalen <= 0) {
         out_sys("bancs closed the connection"); 
         break;
      }
*/
      char message[datalen];
      Readn(connfd, message, datalen);
      message[datalen] = '\0';

      char *total = concat(header, message);
      out_sys(concat("receive request message from bancs: ", total));
     
     // TODO --
     // add card process logic

      Writen(fd_to_bancs, total, datalen + 5);
      out_sys(concat("response message to bancs, message: ", total));   
      memset(header, 0, 5);
      memset(message, 0, datalen);
    }

  }

  return 1;

  usage:
    fprintf(stderr,Usage);
    exit(1);
}

/**
 * Handle the client request 
 */
void InboundHandler() {

  out_sys("inbound handler start");
/*  
  struct addrinfo *dest;
  if(getaddrinfo(host, NULL, NULL, &dest)!=0)
    err_sys("badhostname");

  memset(&bancs_to_card, 0, sizeof(&bancs_to_card));
  memcpy((void*)&bancs_to_card,(void*)dest->ai_addr, dest->ai_addrlen);

  ((struct sockaddr_in *)&bancs_to_card)->sin_port = htons(cport);

  fd_to_card = Socket(AF_INET, SOCK_STREAM, 0);
  if(debug) out_sys("socket");

  Connect(fd_to_card, (struct sockaddr *)&bancs_to_card, sizeof(bancs_to_card));
  out_sys(concat("connect to card ", host));

*/

  memset(&bancs_esb_ss, 0, sizeof(&bancs_esb_ss));
  memset(&bancs_from_esb, 0, sizeof(&bancs_from_esb));

  ((struct sockaddr_in *)&bancs_esb_ss)->sin_port = htons(inport);
  fd = Socket(AF_INET, SOCK_STREAM, 0);
  if(debug) out_sys("inbound handler socket");

  Bind(fd, (struct sockaddr *)&bancs_esb_ss, sizeof(bancs_esb_ss));
  if(debug) out_sys("inbound handler bind");

  Listen(fd, 5);
  char str[80];
  sprintf(str, "inbound handler listen on 0.0.0.0:%d", inport);
  out_sys(str);

  for(;;) {
    connfd = Accept(fd, (struct sockaddr *)&bancs_from_esb, &bancs_from_esb_len);

    char *peer = Getpeername(connfd);

    while(*glob_fd_to_card <= 0) {
        out_sys("BANCS TO CARD Connection is not initialized\n");
        sleep(lazy * 3);
    }

/*
  Snnipts: open socket fd in sub threads.
    char fd_path[64];
    snprintf(fd_path, sizeof(fd_path), "/proc/%d/fd/%d", getppid(), *glob_fd_to_card);
    int new_fd = open(fd_path, O_RDWR);
    char strs[80];
    sprintf(strs, "open new sock fd: %d", new_fd);
    out_sys(strs);   
*/
    for(;;) {
        char header[5];
        Readn(connfd, header, 5);
        header[5] = '\0';
        int datalen = extlength(header);

        if(datalen <= 0) {
            char str[80];
            sprintf(str, "CLIENT %s exit", peer);
            out_sys(str);
            break;
        }

        char message[datalen];
        Readn(connfd, message, datalen);
        message[datalen] = '\0';

        char *total = concat(header, message);

/*
        while(*glob_fd_to_card <= 0) {
            out_sys("BANCS TO CARD Connection is not initialized\n");
            sleep(lazy * 3);
        }

printf("fd_to_card: %d, message: %s\n", fd_to_card, total);
sleep(3);
memset(header, 0, 5);
memset(message, 0, datalen);
continue;
*/
        int sock_fd_to_card = *glob_fd_to_card;
        char str[80];
        sprintf(str, "inbound message from %s, message length: %d, ppid: %d, sock fd: %d", peer, datalen, getppid(), *glob_fd_to_card);
        out_sys(str);
        Writen(sock_fd_to_card, total, datalen + 5);
        out_sys(concat("request message to card, message: ", total));
        
        memset(header, 0, 5);
        memset(message, 0, datalen);
    }

    Close(connfd);
  }
}

/*
 * Listen for Card to connect, handle card's message.
 * Standard Sock wait for CARD to connect, and handle CARD's response message.
 * Handle CARD system's response message; Repeatedly extract the message. Current handling mechanism is only output the response message to log.
 *
 */
void BancsFromCardHandler() {
  
    out_sys("card handler start");

    memset(&bancs_card_ss, 0, sizeof(&bancs_card_ss));
    memset(&bancs_from_card, 0, sizeof(&bancs_from_card));

    ((struct sockaddr_in *)&bancs_card_ss)->sin_port = htons(port);
    fd_bancs = Socket(AF_INET, SOCK_STREAM, 0);
    if(debug)out_sys("socket");

    Bind(fd_bancs, (struct sockaddr *)&bancs_card_ss, sizeof(bancs_card_ss));
    if(debug)out_sys("bind");

    Listen(fd_bancs, 5);
    char str[80];
    sprintf(str, "card handler listen on 0.0.0.0:%d", port);
    out_sys(str);

    for(;;) {

        connfd_bancs = Accept(fd_bancs, (struct sockaddr *)&bancs_from_card, &bancs_from_card_len);
        char *peer = Getpeername(connfd_bancs);
        out_sys(concat("connection from card: ", peer));

        for(;;) {
            char header[5];
            Readn(connfd_bancs, header, 5);
            header[5] = '\0';
            int datalen = extlength(header);

            if(datalen <= 0) {
                char str[80];
                sprintf(str, "CARD %s exit", peer);
                out_sys(str);
                break;
            }

            char message[datalen];
            Readn(connfd_bancs, message, datalen);
            message[datalen] = '\0';
            char *total = concat(header, message);
            out_sys(concat("response message from card, message: ", total)); // TODO- add to parse ISO8583 to extract specific bit position.  
        }
    }

/*
    connfd_bancs = Accept(fd_bancs, (struct sockaddr *)&bancs_from_card, &bancs_from_card_len);
    char *peer = Getpeername(connfd_bancs);
    out_sys(concat("conn from card: ", peer));
    for(;;) {
        char header[5];
        Readn(connfd_bancs, header, 5);
        header[5] = '\0';
        int datalen = extlength(header);
        char message[datalen];
        Readn(connfd_bancs, message, datalen);
        message[datalen] = '\0';
        char *total = concat(header, message);
        out_sys(concat("response message from card, message: ", total));
    }
*/
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

    char* role = "CLIENT";
    if(server == 0)
        role = "CLIENT";
    else if (server == 1)
        role = "BANCS";
    else if (server == 2)
        role = "CARD";

    //perror(fmt);
    fprintf(stderr,"%s error, errno=%d, msg=%s\n", role, errno, fmt);
    va_end(ap);

    exit(0);
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



/*
 *
 * Write "n" bytes to a descriptor. 
 *
 * */
void Writen(int fd, void *ptr, size_t nbytes)
{

    if (writen(fd, ptr, nbytes) != nbytes)
        err_sys("writen");
}

/*
 *
 * Read "n" bytes from a descriptor.
 *
 * */
ssize_t Readn(int fd, void *ptr, size_t nbytes)
{
    ssize_t         n;

    if ( (n = readn(fd, ptr, nbytes)) < 0)
        err_sys("readn error");

    return(n);
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
            break;                 

                nleft -= nread;
                ptr   += nread;
    }
    return(n - nleft);              
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

