#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <memory.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    printf("Hello World!\n");

    int lsock = socket(AF_INET, SOCK_STREAM, 0);

#if 1
    int mss = 576;
    int result = setsockopt(lsock, IPPROTO_TCP, TCP_MAXSEG, &mss, sizeof(mss));
    if (result != 0)
    {
        perror(0);
        return 1;
    }
#endif

    int curr_mss;
    socklen_t curr_mss_len = sizeof(curr_mss);
    if (getsockopt(lsock, IPPROTO_TCP, TCP_MAXSEG, &curr_mss, &curr_mss_len) < 0) {
        perror(0);
        return 1;
    }

    printf("mss: %d\n", curr_mss);

    int optval = 1;
    setsockopt(lsock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    sockaddr_in bind_addr;
    memset(&bind_addr, 0, sizeof(bind_addr));
    bind_addr.sin_family = AF_INET;
    bind_addr.sin_port = htons(1145);

    if (bind(lsock, (const sockaddr *)&bind_addr, sizeof(bind_addr)) < 0) {
        perror(0);
        return 1;
    }

    if (listen(lsock, SOMAXCONN) < 0) {
        perror(0);
        return 1;
    }

    sockaddr peer_addr;
    socklen_t peer_addr_len = sizeof(peer_addr);
    int sock = accept(lsock,  (struct sockaddr*)&peer_addr, &peer_addr_len);
    if (sock < 0) {
        perror(0);
        return 1;
    }

    for (;;) {
        printf("Accepted.\n");
        int curr_mss;
        socklen_t curr_mss_len = sizeof(curr_mss);
        if (getsockopt(sock, IPPROTO_TCP, TCP_MAXSEG, &curr_mss, &curr_mss_len) < 0) {
            perror(0);
            return 1;
        }

        printf("mss: %d\n", curr_mss);
        sleep(30);
    }


  return 0;
}
