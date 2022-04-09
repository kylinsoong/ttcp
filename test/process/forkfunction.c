#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

int main()
{
    pid_t pid;
    int x = 1;
    
    pid = fork();
    if (pid == 0 ) {
        printf("child : x=%d\n", ++x);
        exit(0);
    }
    
    wait(NULL);
    printf("parent: x=%d\n", --x);
    exit(0);
}
