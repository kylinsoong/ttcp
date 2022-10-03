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

char* substring(char*, int, int);

int main(int argc, char **argv)
{


    char * msg = "-1562  000985000000000   **  0000000000003025002303818010002000000         2  1000005                                  0                RF000001>A202209221849094451000000300000000444394782C0000                                                                                                                                                                                                                                                                                                                                                                                                        0000000044439478201151150206198903180013                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              ";


    printf("length: %d, msg: %s\n", strlen(msg), msg);

    //char * gen = "-1562040064  5000000026212260200166874038   5002301214010002000000000100100312143622101003";
    //char * gen = "-1562040064  5000000026212260200166874038   5002301214010002000000000100100312143622101003000000000000000000030002     0                RF000001>A202209221849094451000000300000000444394782C0000                                                                       eefbedec412e000000000100030000000105000300000000000000000000100000000000000020000000000000003000478201151150206198903180013";
    char * gen = "-1562040064  5000000026212260200166874038   5002301214010002000000000100100312143622101003000000000000000000030002     0                RF000001>A202209221849094451000000300000000444394782C0000                                                                       eefbedec412e000000000100030000000105000300000000000000000000100000000000000020000000000000003000                                                          041050000000210031214360000000094300000010                                    D000000002000000082010121220                                                             0000000044439478201151150206198903180013                                                                                                                                                                                                                                                                                                       0000000000000000000000000001000000000000000000000000000000000000000000000000                                                                                                        C000000000                                                                                                                                                                                                        0000000000                                                                                                                                                                                                                                                0000000";

    printf("length: %d, msg: %s\n", strlen(gen), gen);

    printf("1. message length:\n");
    printf("%s\n", substring(msg, 1, 4));
    printf("%s\n", substring(gen, 1, 4));

    printf("2. serialnum:\n");
    printf("%s\n", substring(msg, 13, 9));
    printf("%s\n", substring(gen, 13, 9));
    
    printf("3. tradenum:\n");
    printf("%s\n", substring(msg, 54, 6));
    printf("%s\n", substring(gen, 54, 6));

    printf("4. trackingnum:\n");
    printf("%s\n", substring(msg, 90, 24));
    printf("%s\n", substring(gen, 90, 24));

    printf("5. RF:\n");
    printf("%s\n", substring(msg, 136, 8));
    printf("%s\n", substring(gen, 136, 8));

    printf("6. Transaction Times:\n");
    printf("%s\n", substring(msg, 280, 10));
    printf("%s\n", substring(gen, 280, 10));

    printf("7. REverse Times:\n");
    printf("%s\n", substring(msg, 290, 10));
    printf("%s\n", substring(gen, 290, 10));

    printf("8. Free Amount:\n");
    printf("%s\n", substring(msg, 300, 12));
    printf("%s\n", substring(gen, 300, 12));

    printf("9. Reverse Amount:\n");
    printf("%s\n", substring(msg, 344, 16));
    printf("%s\n", substring(gen, 344, 16));
 
    printf("10. Origin Organization:\n");
    printf("%s\n", substring(msg, 452, 8));
    printf("%s\n", substring(gen, 452, 8));

    printf("11. Birth:\n");
    printf("%s\n", substring(msg, 613, 12));
    printf("%s\n", substring(gen, 613, 12));

/*
    printf("%c%c%c\n", msg[13], msg[14], msg[15]);
    printf("%c%c%c\n", gen[13], gen[14], gen[15]);
   */ 

    return 1;
}

char *substring(char *string, int position, int length) {

    char *p;
    int c;
 
    p = malloc(length+1);
   
    if (p == NULL)
    {
       printf("Unable to allocate memory.\n");
       exit(1);
    }
 
    for (c = 0; c < length; c++)
    {
       *(p+c) = *(string+position);      
       string++;  
    }
 
    *(p+c) = '\0';
 
    return p;

}
