/**
 * Author: Kylin Soong(kylinsoong.1214@gmail.com)
 * Date  : 2019-12-31
 */

#include "neth.h"

int Socket(int family, int type, int protocol)
{   
    int listen_sock;

    if ( (listen_sock = socket(family, type, protocol)) < 0) 
        err_sys("socket error");

    return (listen_sock);
}

void Bind(int fd, const struct sockaddr *sa, socklen_t salen)
{
    if (bind(fd, sa, salen) < 0)
        err_sys("bind error");
}

void Listen(int fd, int backlog)
{
    char *ptr;

    if ((ptr = getenv("LISTENQ")) != NULL) { /*4can override 2nd argument with environment variable */
        backlog = atoi(ptr);
    }

    if (listen(fd, backlog) < 0)
        err_sys("listen error");
}

int Accept(int fd, struct sockaddr *sa, socklen_t *salenptr)
{
    int sock;

    if ((sock = accept(fd, sa, salenptr)) < 0)
        err_sys("accept error");

    return (sock);
}

void Inet_pton(int family, const char *strptr, void *addrptr)
{
    int n;

    if ( (n = inet_pton(family, strptr, addrptr)) < 0)
        err_sys("inet_pton error for %s", strptr);
}

void Connect(int fd, const struct sockaddr *sa, socklen_t salen)
{
    if (connect(fd, sa, salen) < 0)
        err_sys("connect error");
}

