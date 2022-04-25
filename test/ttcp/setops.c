#include <stdio.h>
#include <sys/socket.h>

int main(int argc, char **argv)
{
#if defined(SO_SNDBUF) || defined(SO_RCVBUF)
    printf("YES\n");
#else
    printf("NO\n");
#endif
}
