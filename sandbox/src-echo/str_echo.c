/**
 * Author: Kylin Soong(kylinsoong.1214@gmail.com)
 * Date  : 2019-12-31
 */

#include "neth.h"

void Str_echo(int sockfd)
{
    ssize_t         n;
    char            line[MAXLINE], sendBuff[BUFSIZE];
    time_t          ticks;

    for(;;) {

        n = Readline(sockfd, line, MAXLINE);

        if(n < 0) {
            err_sys("str_echo: read error");
        } else if (n == 0) {
            out_sys("connection closed by other end");
            return;
        }

        //printf("%s", line);

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


