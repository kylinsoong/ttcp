/**
 * Author: Kylin Soong(kylinsoong.1214@gmail.com)
 * Date  : 2019-12-31
 */

#include "neth.h"

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
