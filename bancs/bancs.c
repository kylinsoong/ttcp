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
 * |  ESB  | 
 * |       |
 *  -------  
 *     | request
 *     |
 *     |
 *  -------  request           _______
 * |       |------------------|       |
 * | BANCS |                  |  CARD |
 * |       |------------------|       |
 *  -------         response   -------
 *
 * 1. BANCS listen on 9805, 8805, CARD listen on 8806, BANCS connect to CARD via 8806(mark as connection 1), CARD connect to BANCS via 8805(mark as connection 2)
 * 2. A client simulate ESB send the request message to BANCS via 9902 which listened by BANCS
 * 3. BANCS forward request message to CARD
 * 4. CARD send response message to BANCS
 *  
 *
 * DESIGN PRINCIPLES:
 *
 * 1. Client ESB load the ISO8583 Message from '/etc/bancs.data', which the file keep bunch of Messages, one message per linem, and send to Bancs via 1 connection.
 * 
 * 2. All Handler contains a listened entrypoint, and started as a separate threads  
 *    
 *      Sub threads use pipe to write the message received from socket, main thread read message from pipe, and send the message via write sock fd.
 *      3 Handler are 3 separate threads, 2 Init be invoked by main thread. The BANCS has 2 Handler and 1 Init; the CARD has 1 Handler and 1 Init.
 *
 *      Inbound sequence(N/S from ESB, E/W from BANCS to CARD):
 *
 *          ESB  -  BANCS's InboundHandler  -  BANCS's BancsToCardInit  -  CARD's CardFromBancsHandler   
 *
 *      Outbound sequence(E/W from CARD to BANCS):
 *
 *          CARD's CardToBancsInit  -  BANCS's BancsFromCardHandler    
 *
 * 3. ISO8583 compatible, a tools used for generate ISO message
 *
 *
 * RUN ON SINGLE LINUX SERVER:
 *
 *   Start bancs
 *   ./a.out -b -d -l 1 127.0.0.1
 *
 *   Start Card
 *   ./a.out -c -d -l 1 127.0.0.1
 *  
 *   Start Client
 *   ./a.out -e 127.0.0.1     
 *
 *   Start Tools
 *   ./a.out -t -n 10 > /etc/bancs.data 
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

int lazy = 8;

short port = 8805;
short cport = 8806;               /* TCP port number */
short inport = 9805; 

char *host;                     /* ptr to name of host */
int server = 1;                 /* 0=esb, 1=bancs, 2=card, 3=tools */
int debug = 0 ;

int p[2];

FILE * fp;
char bufr[MAX_DATA_LINE];

extern int errno;
extern int optind;
extern char *optarg;

int num = 3;      /* how many messages to generate */
int kind = 1562;  /* how long a message should be, currently support 1562 */
int m = 6;        /* 0 - 0100, 1 - 0110, 2 - 0200, 3 - 0210, 4 - 0240, 5 - 0250, 6 - 0400, 7 - 0410, 8 - 0800, 9 - 0810 */


char Usage[] = "\
Usage: bancs -e [-options] <host of BANCS> \n\
       bancs -b [-options] <host of CARD>\n\
       bancs -c [-options] <host of BANCS>\n\
       bancs -t [-options]\n\
Common options:\n\
        -d      enable debug logging\n\
        -l ##   the length of lay time of Init wait Listener (default 8 seconds, which means once Listener init finished and 8 * 10 seconds later, the Init start)\n\
        -p ##   port number to send to or listen at (default 8805/8806 9805)\n\
Common options for -t:\n\
        -m ##   specify the version of the ISO8583 standard, allowed value are 0-9\n\
        -n ##   total number of message to generated\n\
        -k ##   specify message kinds, different kinds means different length, 1 - 1562\n\
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
void     BancsToCardInit(void);
void     CardFromBancsHandler(void);
void     CardToBancsInit(void);

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

void     WriteToSock(int);
void     ReadFromSock(int);

void     generate(int, int);
char     *leftpadding(int, int, char);

/* 1 - bancs to card 
 * 2 - card to bancs
 * 3 - bancs from card
 * 4 - card from bancs
 * */
int hint = 0 ;


/*
void     Str_puts(int);
*/

int main(int argc, char **argv) 
{
    if (argc < 2) goto usage;

    // parse the main argv
    int c;
    while ((c = getopt(argc, argv, "ebctdl:p:m:n:k:")) != -1) {
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
        case 't':
            server = 3;
            break;
        case 'p':
            port = atoi(optarg);
            cport = port + 1;
            inport = port + 1000; 
            break;
        case 'd':
            debug = 1;
            break;
        case 'n':
            num = atoi(optarg);
            break;
        case 'l':
            lazy = atoi(optarg);
            break;
        case 'k':
            kind = atoi(optarg);
            if(kind == 1) {
                kind = 1562;
            } else if (kind = 2) {
                kind = 864;
            }
            break;
        case 'm':
            m = atoi(optarg);
            break;
        default:
            goto usage;
        }
    }

    if( server == 0 || server == 1 || server == 2) {

        if (optind == argc) {
            goto usage;
        }
  
        host = argv[optind];

        if (pipe(p) < 0)
            err_sys("pipe");
    }

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

    Signal(SIGCHLD, sig_chld);
    int rc = fork();
    if (rc < 0 ) {
      err_sys("Error:unable to create thread");
    } else if ( rc == 0) {
      InboundHandler();
    } else {

        Signal(SIGCHLD, sig_chld);
        int src = fork();
        if(src < 0) {
            err_sys("Error:unable to create thread");
        } else if (src == 0) {
            BancsFromCardHandler();
        } else {

            hint = 1;

            sleep(lazy * 10);

            BancsToCardInit();

            WriteToSock(fd_to_card);

        }           
     } 
  } else if(server == 2) {

    out_sys("start");

    Signal(SIGCHLD, sig_chld);
    int rc = fork();
    if(rc < 0) {
        err_sys("Error:unable to create thread");
    } else if (rc == 0) {
        CardFromBancsHandler();
    } else {

        hint = 2;

        sleep(lazy * 10); 

        CardToBancsInit();

        WriteToSock(fd_to_bancs);
    }

  } else if(server == 3) {
      
      if(debug) out_sys("generate messages");

      generate(num, kind);

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

  memset(&bancs_esb_ss, 0, sizeof(&bancs_esb_ss));
  memset(&bancs_from_esb, 0, sizeof(&bancs_from_esb));

  ((struct sockaddr_in *)&bancs_esb_ss)->sin_port = htons(inport);
  fd = Socket(AF_INET, SOCK_STREAM, 0);
  if(debug) {
        char str[80];
        sprintf(str, "inbound handler socket, sock fd: %d", fd);
        out_sys(str);
    }

  Bind(fd, (struct sockaddr *)&bancs_esb_ss, sizeof(bancs_esb_ss));
  if(debug) out_sys("inbound handler bind");

  Listen(fd, 5);
  char str[80];
  sprintf(str, "inbound handler listen on 0.0.0.0:%d", inport);
  out_sys(str);

  for(;;) {

    connfd = Accept(fd, (struct sockaddr *)&bancs_from_esb, &bancs_from_esb_len);

    ReadFromSock(connfd);

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
    if(debug) {
        char str[80];
        sprintf(str, "from card socket, sock fd: %d", fd_bancs);
        out_sys(str);
    }

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
                sprintf(str, "CARD %s response illegal message, header: %s, header length: %d, data length: %d", peer, header, strlen(header), datalen);
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

}

void BancsToCardInit() {

    struct addrinfo *dest;
    if(getaddrinfo(host, NULL, NULL, &dest)!=0)
        err_sys("badhostname");

    memset(&bancs_to_card, 0, sizeof(&bancs_to_card));
    memcpy((void*)&bancs_to_card,(void*)dest->ai_addr, dest->ai_addrlen);

    ((struct sockaddr_in *)&bancs_to_card)->sin_port = htons(cport);

    fd_to_card = Socket(AF_INET, SOCK_STREAM, 0);
    if(debug) {
        char str[80];
        sprintf(str, "to card socket, sock fd: %d", fd_to_card);
        out_sys(str);
    } 

    Connect(fd_to_card, (struct sockaddr *)&bancs_to_card, sizeof(bancs_to_card));
    out_sys(concat("connect to card ", host));

}

void CardFromBancsHandler() {

    memset(&card_bancs_ss, 0, sizeof(&card_bancs_ss));
    memset(&card_from_bancs, 0, sizeof(&card_from_bancs));

    ((struct sockaddr_in *)&card_bancs_ss)->sin_port = htons(cport);
    fd = Socket(AF_INET, SOCK_STREAM, 0);
    if(debug) {
        char str[80];
        sprintf(str, "from bancs socket, sock fd: %d", fd);
        out_sys(str);
    }

    Bind(fd, (struct sockaddr *)&card_bancs_ss, sizeof(card_bancs_ss));
    if(debug) out_sys("bind");

    Listen(fd, 5);
    char str[80];
    sprintf(str, "bancs handler listen on 0.0.0.0:%d", cport);
    out_sys(str);

    for(;;) {

        connfd = Accept(fd, (struct sockaddr *)&card_from_bancs, &card_from_bancs_len);

        ReadFromSock(connfd);

        Close(connfd);
    }
    
}

void CardToBancsInit() {

    struct addrinfo *dest;
    if(getaddrinfo(host, NULL, NULL, &dest)!=0)
      err_sys("badhostname");

    memset(&card_to_bancs, 0, sizeof(&card_to_bancs));
    memcpy((void*)&card_to_bancs,(void*)dest->ai_addr, dest->ai_addrlen);

    ((struct sockaddr_in *)&card_to_bancs)->sin_port = htons(port);

    fd_to_bancs = Socket(AF_INET, SOCK_STREAM, 0);
    if(debug) {
        char str[80];
        sprintf(str, "to bancs socket, sock fd: %d", fd_to_bancs);
        out_sys(str);
    }

    Connect(fd_to_bancs, (struct sockaddr *)&card_to_bancs, sizeof(card_to_bancs));
    out_sys(concat("connect to bancs ", host));

}

/*
 * The WriteToSock methods can only be invoked by main thread.
 *
 * Firstly, read the message from pipe and write to sock fd.
 *
 * Secondly, if read pipe error,  the main thrads will wait for all son thrads to finish, then exit.
 *
 */
void  WriteToSock(int fd) {

    int nbytes;
    char inbuf[MAX_DATA_LINE];
    while ((nbytes = read(p[0], inbuf, MAX_DATA_LINE)) > 0) {
        char header[5];
        strncpy(header, inbuf, 5);
        int datalen = extlength(header);

        inbuf[datalen + 5] = '\0';

        if(debug) {
            char extMessage[MAX_DATA_LINE];
            sprintf(extMessage, "extract message from pipe, message length: %d, header: %s, total length: %d, message: %s", datalen, header, strlen(inbuf), inbuf);
            out_sys(extMessage);
        }

        inbuf[datalen + 5] = '\0';

        Writen(fd, inbuf, strlen(inbuf));

        if(hint == 1) {
            out_sys(concat("request message to card, message: ", inbuf));
        } else if (hint == 2) {
            out_sys(concat("response message to bancs, message: ", inbuf));
        }

        memset(inbuf, 0, strlen(inbuf));
    }

    int subpid, status;
    while((subpid = waitpid(-1, &status, 0)) > 0) {
        char str[80];
        sprintf(str, "process %d terminated", subpid);
        out_sys(str);
    }

}

/*
 * The ReadFromSock method be used by listener, read the message from connection sock fd, and add message to a pipe.
 *
 * The ReadFromSock only for one specific connection sock fd, if connfd is closed ot broken, this methods will throw exception.
 * 
 * The read from connfd will repeatedly, till the connfd be broken. 
 *
 */
void ReadFromSock(int connfd) {

    char *peer = Getpeername(connfd);
        
    out_sys(concat("connection from ", peer));

    for(;;) {

        char header[5];
        Readn(connfd, header, 5);
        header[5] = '\0';
        int datalen = extlength(header);

        if(datalen <= 0) {
            char str[80];
            sprintf(str, "peer %s exit", peer);
            out_sys(str);
            break;
        }

        char message[datalen];
        Readn(connfd, message, datalen);
        message[datalen] = '\0';

        char *total = concat(header, message);
        total[datalen + 5] = '\0';

        char str[80];
        sprintf(str, "receive message from %s, message length: %d, total length: %d", peer, datalen, strlen(total));
        out_sys(str);

        write(p[1], total, datalen + 5);
        if(debug) {
            out_sys(concat("add message to pipe, message: ", total));
        }

        memset(header, 0, 5);
        memset(message, 0, datalen); write(p[1], total, datalen + 5);
    }

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
    else if (server == 3)
        role = "TOOLS";

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
    else if (server == 3)
        role = "TOOLS";

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

void generate(int num, int kind) {

    int i, j;
    for(i = 0 ; i < num ; i ++) {
       
        char *message = malloc(kind +5);

        // header: length: 5
        char *header = leftpadding(5, kind, '-');
        message = concat(header, "        ");

        // mit: four numeric digits that specify the version of the ISO8583 standard.
        char *mit = malloc(4);
        switch (m) {
            case 0 :
                mit = "0100";  // Authorization Request
                break;
            case 1 :
                mit = "0110";  // Request Response
                break; 
            case 2 :
                mit = "0200";  // Acquirer Financial Request
                break;
            case 3 :
                mit = "0210";  // Issuer Response to Financial Request
                break;
            case 4 :
                mit = "0240";  // Query Transaction
                break;
            case 5 :
                mit = "0250";  // Query Transaction Reply
                break;
            case 6 :
                mit = "0400";  // Reversed Transaction 
                break;
            case 7 :
                mit = "0410";  // Reversed Transaction Reply
                break;
            case 8 :
                mit = "0800";  // Management Transaction
                break;
            case 9 :
                mit = "0810";  // Management Transaction Reply
                break;
            default:
                mit = "0400";  // Reversed Transaction 
                break;      
        }

        message = concat(header, mit);
       
        // bitmap: 64 or 128
        char *bm = "64  ";
        message = concat(message, bm); 

        // serialnum: length: 9
        int serialnum = 500000000 + i ;
        char serial[9];
        sprintf(serial, "%d", serialnum);
        message = concat(message, serial);

        // Primary Account Number: length 22
        char pan[22] = "6212260200166874038   ";
        message = concat(message, pan);

        // Process Code (length 6) 
        // Local Transaction( length 4, format: MMDD)
        message = concat(message, "5002301214");

        // tradenum: length 6
        int tradenum = 10000 + i ; 
        char *trade = leftpadding(6, tradenum, '0');
        message = concat(message, trade);        

        // Transaction: length: 12, 
        // Transmission Date and Time: length: 10, format: MMDDhhmmss 
        // Expiration, length: 4, foramt: YYMM
        // Settlement, length: 4. format: MMDD
        message = concat(message, "000000000100");        // represent 1.00 local currency,
        message = concat(message, "100312143622101003");  // Transmission Date and Time + Expiration + Settlement
     
        // trackingnum, length: 24
        int tracknum = 30000 + i ;
        char *track = leftpadding(24, tracknum, '0');
        message = concat(message, track);      
        
        // Padding (24) + RF (8) 
        message = concat(message, "     0                RF000001");      

        // Date related
        message = concat(message, ">A202209221849094451000000300000000444394782C0000       ");      

        // Place Holder, length: 64
        message = concat(message, "                                                                ");

        // MAC(16) + Transaction Number(10) + Reversal Number(10)
        message = concat(message, "eefbedec412e0000");
        message = concat(message, "0000010003");
        message = concat(message, "0000000105");

        // FeeAmount (12) + Amount(16) + Debits Amount (16) + Reversal Amount(16)
        message = concat(message, "000300000000");        // represent 3 m local currency,
        message = concat(message, "0000000000001000");
        message = concat(message, "0000000000002000");
        message = concat(message, "0000000000003000");

        // Padding(58)
        message = concat(message, "                                                          ");

        // Original Data Elements (42 = 4 +9 + 10 + 11 + 8)
        message = concat(message, "041050000000210031214360000000094300000010"); 

        // Padding (36) + Net Settlement Amount(17) + Settlement Institution Identification(11)
        message = concat(message, "                                    ");
        message = concat(message, "D0000000020000000");
        message = concat(message, "82010121220");

        // Padding 
        message = concat(message, "                                                             0000000044439");     

        // Account (15) + Birth (12)
        message = concat(message, "478201151150206198903180013");        

        // Padding (275)
        message = concat(message, "                                                                                                    ");
        message = concat(message, "                                                                                                    ");
        message = concat(message, "                                                                                               ");

        // Account Identification 1 (28) + Account Identiication 2 (48)
        message = concat(message, "0000000000000000000000000001000000000000000000000000000000000000000000000000");

        // Padding (104)
        message = concat(message, "                                                                                                        ");
        message = concat(message, "C000000000");
        message = concat(message, "                                                                                                    ");
        message = concat(message, "                                                                                                    ");
        message = concat(message, "0000000000");
        message = concat(message, "                                                                                                                        ");
        message = concat(message, "                                                                                                                        ");
        message = concat(message, "0000000");

        // holder
        //message = concat(message, "        "); 
        printf("%s\n", message);
    }

}

char *leftpadding(int length, int value, char c) {

    char *result = malloc(length);

    char text[30];
    sprintf(text, "%d", value);
    int len = strlen(text);
    text[len] = '\0';

    int pad = length - len;
    if(pad > 0) {
        char content[pad];
        content[pad] = '\0';
        int i ;
        for (i = 0 ; i < pad ; i ++) {
            content[i] = c ;
        }
        result = concat(content, text);
    } else {
        result = text;
    }

    return result;
}
