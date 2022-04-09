#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

int main()
{
    fork();
    printf("hello\n");
    wait(NULL);
}

