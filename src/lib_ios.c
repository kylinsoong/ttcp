/**
 *  * Author: Kylin Soong(kylinsoong.1214@gmail.com)
 *   * Date  : 2019-12-31
 *    */

#include "neth.h"

/**
 *  Process
 */
pid_t Fork(void)
{
    pid_t pid;

    if ((pid = fork()) == -1)
        err_sys("fork error");
    return (pid);
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

void Close(int fd)
{
    if (close(fd) == -1)
        err_sys("close error");
}

/**
 * Write
 */
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

/**
 * ReadLine
 */

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

