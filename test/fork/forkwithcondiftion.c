#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

void doit() {
    printf("%d %d hello\n", getppid(), getpid());
    fork();
    if(fork() ==0) {
       printf("%d %d hello 2st fork\n", getppid(), getpid());
    } 
    fork();
    printf("%d %d hello 3nd fork\n", getppid(), getpid());
}

int main() {
    printf("%d %d hello m\n", getppid(), getpid());
    doit();
    sleep(1);
    exit(0);
}
