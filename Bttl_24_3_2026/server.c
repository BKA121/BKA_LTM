#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define TARGET "0123456789"
#define TARGET_LEN 10

int main() {
    int server_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9090);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int opt = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    bind(server_sock, (struct sockaddr *)&addr, sizeof(addr));
    listen(server_sock, 5);

    int client_sock = accept(server_sock, NULL, NULL);

    char recv_buf[1024];
    char leftover[TARGET_LEN] = {0}; 
    int total_occurrences = 0;

    while (1) {
        int n = recv(client_sock, recv_buf, sizeof(recv_buf) - 1, 0);
        if (n <= 0) {
            break;
        }
        recv_buf[n] = '\0';

        char combined[2048] = {0};
        snprintf(combined, sizeof(combined), "%s%s", leftover, recv_buf);

        char *ptr = combined;
        while ((ptr = strstr(ptr, TARGET)) != NULL) {
            total_occurrences++;
            ptr += 1; 
        }

        printf("Dữ liệu nhận được: [%s] | Tổng số lần thấy '%s': %d\n", 
                recv_buf, TARGET, total_occurrences);

        int combined_len = strlen(combined);
        int to_copy = (combined_len < TARGET_LEN - 1) ? combined_len : (TARGET_LEN - 1);
        
        memset(leftover, 0, sizeof(leftover));
        strncpy(leftover, combined + combined_len - to_copy, to_copy);
    }

    close(client_sock);
    close(server_sock);
    return 0;
}