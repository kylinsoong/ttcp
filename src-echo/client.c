/**
 *  * Author: Kylin Soong(kylinsoong.1214@gmail.com)
 *   * Date  : 2019-12-31
 *    */

#include "neth.h"

int main(int argc, char **argv)
{
    int                sockfd;
    struct sockaddr_in servaddr;
    int port;

    if (argc != 3)
        err_sys("usage: tcp-echo-client <IP> <PORT>");

    sockfd = Socket(AF_INET, SOCK_STREAM, 0);

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    port = atoi(argv[2]);
    servaddr.sin_port = htons(port);
    Inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

    Connect(sockfd, (SA *) &servaddr, sizeof(servaddr));

    Str_cli(stdin, sockfd);

    exit(0);
}

