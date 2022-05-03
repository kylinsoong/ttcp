#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netinet/in.h>

int main(int argc, char **argv)
{

    int mss ;
    int fdsocket = socket(AF_INET, SOCK_STREAM, 0);
    socklen_t len = sizeof(mss);
    getsockopt(fdsocket, IPPROTO_TCP, TCP_MAXSEG, (void *)&mss, &len);
    printf("default mss=%d\n", mss);
    
    int mss_n = 576;
    int result = setsockopt(fdsocket, IPPROTO_TCP, TCP_MAXSEG, &mss_n, sizeof(mss_n));
    if (result != 0)
    {
        perror(0);
        return 1;
    }

    int mss_n_g;
    socklen_t len_n;
    getsockopt(fdsocket, IPPROTO_TCP, TCP_MAXSEG, (void *)&mss_n_g, &len_n);
    printf("new   mss=%d\n", mss_n_g);
    
    return 0;
}

