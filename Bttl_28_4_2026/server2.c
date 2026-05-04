#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/select.h> 
#include <errno.h>

int main(int argc, char *argv[]) {
    if (argc < 4) {
        printf("Sử dụng: %s <port_nguon> <ip_dich> <port_dich>\n", argv[0]);
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

    struct sockaddr_in addr_s = {0};
    addr_s.sin_family = AF_INET;
    addr_s.sin_addr.s_addr = htonl(INADDR_ANY);
    addr_s.sin_port = htons(port_s);

    if (bind(sockfd, (struct sockaddr *)&addr_s, sizeof(addr_s))) {
        perror("bind() failed");
        close(sockfd);
        return 1;
    }

    // Địa chỉ đích để gửi tin nhắn tới
    struct sockaddr_in addr_d = {0};
    addr_d.sin_family = AF_INET;
    addr_d.sin_addr.s_addr = inet_addr(ip_d);
    addr_d.sin_port = htons(port_d);

    printf("Chat UDP Ready! Nhập tin nhắn và nhấn Enter.\n");
    printf("Bạn: "); fflush(stdout);

    char buf[1024];
    fd_set readfds;

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds); 
        FD_SET(sockfd, &readfds);       

        int maxfd = (sockfd > STDIN_FILENO) ? sockfd : STDIN_FILENO;

        if (select(maxfd + 1, &readfds, NULL, NULL, NULL) < 0) {
            perror("select() failed");
            break;
        }

        if (FD_ISSET(sockfd, &readfds)) {
            struct sockaddr_in sender_addr;
            socklen_t sender_len = sizeof(sender_addr);
            
            int len = recvfrom(sockfd, buf, sizeof(buf) - 1, 0, 
                               (struct sockaddr *)&sender_addr, &sender_len);
            if (len > 0) {
                buf[len] = 0;
                printf("\r[NHẬN từ %s]: %s", inet_ntoa(sender_addr.sin_addr), buf);
                printf("Bạn: "); fflush(stdout);
            }
        }

        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            if (fgets(buf, sizeof(buf), stdin) != NULL) {
                sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *)&addr_d, sizeof(addr_d));
                
                if (strncmp(buf, "exit", 4) == 0) break;
                
                printf("Bạn: "); fflush(stdout);
            }
        }
    }

    close(sockfd);
    printf("\nĐã đóng kết nối.\n");
    return 0;
}