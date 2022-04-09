#include <stdio.h>
#include <stdlib.h>


int main()
{

    int status, i;   
    pid_t pid;

    for (i = 0; i < 2; i++)
        if ((pid = Fork()) == 0) 
            exit(100+i);

}
