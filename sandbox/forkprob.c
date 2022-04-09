#include <stdio.h>
#include <stdlib.h>

void doit()
{
  fork();
  fork();
  printf("hello\n");
}

int main()
{
  doit();
  printf("hello\n");
  wait(NULL);
}
