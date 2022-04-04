#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>           /* struct timeval */
#include <sys/resource.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct addrinfo hints;

void test_addressinfo();


int main(int argc, char **argv)
{
    test_addressinfo();

    return 0;
}

void test_addressinfo()
{
    printf("testAddressinfo\n");
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    printf("AF_INET: %d, SOCK_STREAM: %d\n", hints.ai_family, hints.ai_socktype);
}

