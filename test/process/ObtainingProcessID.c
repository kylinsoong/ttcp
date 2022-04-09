#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

int main()
{
    printf("pid: %d, ppid: %d \n", (int)getpid(), (int)getppid());
    fork();
    fork();
    printf("pid: %d, ppid: %d \n", (int)getpid(), (int)getppid());

    wait(NULL);
    wait(NULL);
    exit(0);
}
