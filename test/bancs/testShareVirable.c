#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static int *glob_fd_to_card;

int main(void)
{
    glob_fd_to_card = mmap(NULL, sizeof *glob_fd_to_card, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    *glob_fd_to_card = 1;

    if (fork() == 0) {
        *glob_fd_to_card = 5;
        exit(EXIT_SUCCESS);
    } else {
        wait(NULL);
        printf("%d\n", *glob_fd_to_card);
        munmap(glob_fd_to_card, sizeof *glob_fd_to_card);
    }
    return 0;
}
