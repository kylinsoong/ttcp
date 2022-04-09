#include <unistd.h>
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

void handler1(int sig)
{
    pid_t pid;

    while ((pid = waitpid(-1, NULL, 0)) > 0)
        printf("waitpid error");

    printf("Handler reaped child %d\n", (int)pid);
    sleep(2);
    return;
}

int main()
{

    int i, n;
    char buf[1024];
    
    if (signal(SIGCHLD, handler1) == SIG_ERR)
        printf("signal error");

    for (i = 0; i < 3; i++) {
        if (fork() == 0) {
            printf("Hello from child %d\n", (int)getpid());
            sleep(1);
            exit(0);
        }
    }

    if ((n = read(STDIN_FILENO, buf, sizeof(buf))) < 0)
        printf("read error");

    printf("Parent processing input\n");

    while (1) ;

    exit(0);
}
