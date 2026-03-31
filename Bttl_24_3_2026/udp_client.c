#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SERVER_IP "127.0.0.1"
#define PORT 8888
#define BUF_SIZE 1024

int main() {
    int sockfd;
    char buffer[BUF_SIZE];
    char message[BUF_SIZE];
    struct sockaddr_in servaddr;

    sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    while (1) {
        printf("Nhập tin nhắn: ");
        fgets(message, BUF_SIZE, stdin);
        message[strcspn(message, "\n")] = 0; 

        if (strcmp(message, "exit") == 0) break;

        sendto(sockfd, message, strlen(message), 0, (const struct sockaddr *)&servaddr, sizeof(servaddr));

        int n = recvfrom(sockfd, buffer, BUF_SIZE, 0, NULL, NULL);
        buffer[n] = '\0';
        
        printf("Server phản hồi: %s\n", buffer);
    }

    close(sockfd);
    return 0;
}