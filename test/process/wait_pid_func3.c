#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#define N 10

void end (void) {
     printf("2"); 
     fflush(stdout);
}


int main()
{
    if(fork() == 0) {
        atexit(end);
    }

    if(fork() == 0) {
        printf("0");
        fflush(stdout);
    } else {
        printf("1");
        fflush(stdout);
    }

    wait(NULL);
    wait(NULL);
    exit(0);
}
