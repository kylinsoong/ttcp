#include <stdio.h>
#include <stdlib.h>

int main()
{
    pid_t pid;
    int x =1;

    pid = fork();
    if(pid ==0) {
        printf("%d\n", ++x);
        exit(1);
    }

    int wc = wait(NULL);
    printf("%d\n", --x);
}
