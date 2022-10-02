#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdarg.h>
#include <time.h>
#include <errno.h>
#include <signal.h>

void     InboundHandler(void);
void     BancsFromCardHandler(void);

int p[2];


int main(int argc, char **argv) {

    printf("%d start\n", getpid());

    if (pipe(p) < 0)
        exit(1);

    int pid_inbound = fork();
    if (pid_inbound < 0 ) {
        printf("fork err\n");
    } else if ( pid_inbound == 0){
        InboundHandler();
    } else {
        
        int pid_card = fork();
        if(pid_card < 0) {
            printf("fork err\n");
        } else if ( pid_card == 0){
            BancsFromCardHandler();
        } else {
            printf("main: pid_inbound: %d, pid_card: %d, pid: %d, ppid: %d\n", pid_inbound, pid_card, getpid(), getppid());

            printf("p[0]: %d, p[1]: %d\n", p[0], p[1]);

            int nbytes;
            char inbuf[4096];
            while ((nbytes = read(p[0], inbuf, 4096)) > 0) {
                printf("length: %d, msg: %s\n", strlen(inbuf), inbuf);
            }
            
            int subpid, status;
            while((subpid = waitpid(-1, &status, 0)) > 0) {
                printf("process %d exit\n", subpid);
            }
        } 
       
    }

   


}

void InboundHandler() {

    printf("%d start\n", getpid());
 
    printf("p[0]: %d, p[1]: %d\n", p[0], p[1]);

    while(1) {
        sleep(3);
        write(p[1], "1234567890", 10);
    }

}

void BancsFromCardHandler() {

    printf("%d start\n", getpid());

}
