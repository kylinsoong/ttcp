#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>

void handler(int sig) 
{
    printf("Caught SIGINT %d\n", sig);
    exit(1);
}

int main()
{
    if (signal(SIGINT, handler) == SIG_ERR)
        printf("signal error");

    pause();

    exit(0);
}
