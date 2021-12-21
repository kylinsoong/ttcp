#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#define MAX 80
#define PORT 8878
#define SA struct sockaddr
void func(int sockfd)
{
	char buff[MAX];
	int n;
	for (;;) {
		bzero(buff, sizeof(buff));
		printf("Enter the string(%d) : ", sockfd);
		n = 0;
		while ((buff[n++] = getchar()) != '\n')
			;
                write(sockfd, buff, sizeof(buff));
               
                if ((strncmp(buff, "exit", 4)) == 0) {
                    printf("Send 'exit' signal to server..\n");
                    break;
                }

		bzero(buff, sizeof(buff));
		read(sockfd, buff, sizeof(buff));
		printf("From Server(%d) : %s", sockfd, buff);
		if ((strncmp(buff, "exit", 4)) == 0) {
			printf("Receive 'exit' signal from server..\n");
                        sleep(4);
                        exit(1);
		}
	}
}

int main(int argc, char **argv)
{
	int sockfd, connfd;
	struct sockaddr_in servaddr, cli;
        int port;

        if (argc != 3) {
            printf("%s\n", "usage: tcp-chat-client <SERVER_IP> <SERVER_PORT>");
            exit(1);
        }

	// socket create and varification
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		printf("socket creation failed...\n");
		exit(0);
	}
	else
		printf("Socket successfully created(%d)..\n", sockfd);
	bzero(&servaddr, sizeof(servaddr));

	// assign IP, PORT
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(argv[1]);
        port = atoi(argv[2]);
	servaddr.sin_port = htons(port);

	// connect the client socket to server socket
	if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) {
		printf("connection with the server failed...\n");
		exit(0);
	}
	else
		printf("connected to the server(%d)..\n", sockfd);

	// function for chat
	func(sockfd);

	// close the socket
	printf("Client close the socket(%d)...\n", sockfd);
        sleep(2);
	close(sockfd);
}
