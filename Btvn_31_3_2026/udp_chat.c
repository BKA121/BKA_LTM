#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>

int main(int argc, char *argv[]) {
    if (argc < 4) {
        printf("Thieu tham so");
        return 1;
    }

    int port_s = atoi(argv[1]);
    char *ip_d = argv[2];
    int port_d = atoi(argv[3]);

    int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd == -1) {
        perror("socket() failed");
        return 1;
    }

    unsigned long ul = 1;
    ioctl(sockfd, FIONBIO, &ul);

    ioctl(STDIN_FILENO, FIONBIO, &ul);

    struct sockaddr_in addr_s = {0};
    addr_s.sin_family = AF_INET;
    addr_s.sin_addr.s_addr = htonl(INADDR_ANY);
    addr_s.sin_port = htons(port_s);

    if (bind(sockfd, (struct sockaddr *)&addr_s, sizeof(addr_s))) {
        perror("bind() failed");
        close(sockfd);
        return 1;
    }

    struct sockaddr_in addr_d = {0};
    addr_d.sin_family = AF_INET;
    addr_d.sin_addr.s_addr = inet_addr(ip_d);
    addr_d.sin_port = htons(port_d);

    printf("Nhap tin nhan de:\n");

    char buf[1024];
    struct sockaddr_in sender_addr;
    socklen_t sender_len = sizeof(sender_addr);

    while (1) {
        int len = recvfrom(sockfd, buf, sizeof(buf) - 1, 0, 
                           (struct sockaddr *)&sender_addr, &sender_len);
        
        if (len > 0) {
            buf[len] = 0;
            printf("\n[NHẬN]: %s", buf); 
            printf("Bạn: "); fflush(stdout); 
        } else if (len == -1 && errno != EWOULDBLOCK) {
            perror("recvfrom() error");
        }

        int n = read(STDIN_FILENO, buf, sizeof(buf) - 1);
        if (n > 0) {
            buf[n] = 0;
            sendto(sockfd, buf, n, 0, (struct sockaddr *)&addr_d, sizeof(addr_d));
            printf("Bạn: "); fflush(stdout);
        }

        usleep(10000); 
    }

    close(sockfd);
    return 0;
}