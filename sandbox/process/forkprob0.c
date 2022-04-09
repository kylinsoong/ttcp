#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

int main()
{
    int x = 1;
   
    if(fork() == 0)
        printf("print1: x=%d (pid:%d)\n", ++x, (int)getpid()); 
    
    printf("print2: x=%d (pid:%d)\n", --x, (int)getpid());
    wait(NULL);
    exit(0);
}
