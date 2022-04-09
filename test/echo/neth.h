/**
 * Author: Kylin Soong(kylinsoong.1214@gmail.com)
 * Date  : 2019-12-31
 */

#ifndef __neth_h
#define __neth_h

#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <time.h>

#define STR_DATTIME "daytime"
#define STR_TIME    "time"
#define STR_CHARGEN "chargen"
#define STR_ENTER   "\n"

#define SA struct sockaddr

#define SERV_PORT   8877    /* TCP and UDP */
#define LISTENQ     1024    /* 2nd argument to listen() */
#define MAXLINE     4096    /* max text line length */
#define BUFSIZE     1024    

#define ASCII_START 32
#define ASCII_END   126

/* Socket */
int      Socket(int, int, int);
int      Accept(int, SA *, socklen_t *);
void     Bind(int, const SA *, socklen_t);
void     Listen(int, int);
void     Connect(int, const SA *, socklen_t);
void     Inet_pton(int, const char *, void *);

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

#endif //  __neth_h
