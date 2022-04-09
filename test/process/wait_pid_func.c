#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int main()
{
    int status, i;
    pid_t pid;

    for (i = 0; i < 10; i++)
        if ((pid = fork()) == 0)
            exit(100 + i);

    while ((pid = waitpid(-1, &status, 0)) > 0) {
        if (WIFEXITED(status))
            printf("child %d terminated normally with exit status=%d\n", pid, WEXITSTATUS(status));
        else
            printf("child %d terminated abnormally\n", pid);
    }  

    if (errno != ECHILD)
        printf("waitpid error");

    exit(0);
}
