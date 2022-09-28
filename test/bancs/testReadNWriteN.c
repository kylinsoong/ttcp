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

int server = 0 ;
int msglen = 0 , len = 0;  /* message length*/

extern int optind;

char *data;

char Usage[] = "\
Usage: testReadNWriteN -c <message(length should large than 6, the first 5 is the data payload's length, the other is the real payload) write to server> \n\
       testReadNWriteN -s \n\
";

int main(int argc, char **argv) {

    if (argc < 2) goto usage;

    int c;
    while ((c = getopt(argc, argv, "cs")) != -1) {
        switch (c) {
        case 'c':
            server = 0;
            break;
        case 's':
            server = 1;
            break;
        default:
            goto usage;
        }
    }

    if(server) {

    } else {
        data = malloc(4096);
        strcpy(data, argv[optind]);
        if(strlen(data) < 6) goto usage;
        
        char strmsglen[5];
        strncpy(strmsglen, data, 5);

        int i;
        int cur = 0 ;
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
       
        msglen = atoi(substrmsglen);
        len = msglen + 5 ;

        if(strlen(data) != len) goto usage;

        printf("length: %d, message: %s, msglen: %d, len: %d\n", strlen(data), data, msglen, len);   
    }


    return 1;

    usage:
        fprintf(stderr, Usage);
        exit(1);
}
