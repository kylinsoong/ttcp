#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

int main()
{
    int i = 1;
    
    for(i = 0 ; i < 2 ; i++)
        fork();   
    
    printf("hello (pid:%d)\n", (int)getpid());
    wait(NULL);
    wait(NULL);
    exit(0);
}
