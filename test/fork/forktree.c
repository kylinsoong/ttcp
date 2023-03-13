#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

void doit() {
    printf("%d %d hello\n", getppid(), getpid());
    fork();
    printf("%d %d hello 1st fork\n", getppid(), getpid());
    fork();
    printf("%d %d hello 2nd fork\n", getppid(), getpid());
}

int main() {
    printf("%d %d hello m\n", getppid(), getpid());
    printf("%d %d hello m\n", getppid(), getpid());
    doit();
    printf("%d %d hello m\n", getppid(), getpid());
    sleep(1);
    exit(0);
}
