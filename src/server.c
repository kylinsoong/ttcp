/**
 * Author: Kylin Soong(kylinsoong.1214@gmail.com)
 * Date  : 2019-12-31
 */

#include "neth.h"

int main(int argc, char **argv)
{
    int                listenfd, connfd;
    pid_t              childpid;
    socklen_t          clilen;
    struct sockaddr_in cliaddr, servaddr;

    listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);

    Bind(listenfd, (SA *) &servaddr, sizeof(servaddr)); 

    Listen(listenfd, LISTENQ);

    for(;;) {

        clilen = sizeof(cliaddr);
        connfd = Accept(listenfd, (SA *) &cliaddr, &clilen);

        if ((childpid = Fork()) == 0) {
            Close(listenfd);
            Str_echo(connfd); 
            exit(0);
        }
    
        Close(connfd);
    }

    return 1;
}

