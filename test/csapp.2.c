#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
  printf("hello world (pid:%d)\n", (int) getpid());

  if(rc < 0) {
    printf(stderr, "fork failed\n");
  } else if(rc == 0) {
    printf("hello, I am child (pid:%d)\n", (int) getpid());
  } else {
    int wc = wait(NULL);
    printf("hello, I am parent of (pid:%d), my pid: %d\n", wc, (int) getpid());
  }
}
