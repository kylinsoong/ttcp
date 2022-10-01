#include <stdio.h>

int fd, fd_bancs, fd_to_card, fd_to_bancs; 

int main () {

   /* local variable definition */
   int a = 10;

   /* while loop execution */
   while( fd_to_bancs <= 0 ) {
      printf("value of a: %d, fd_to_bancs: %d\n", a, fd_to_bancs);
      sleep(1);
      a++;
   }
 
   return 0;
}
