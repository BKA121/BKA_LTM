#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

int main() {
    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9090);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(client, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("Connect failed");
        return 1;
    }

    char input[1024];
    printf("Nhap chuoi:\n");

    while (1) {
        if (fgets(input, sizeof(input), stdin) == NULL) break;
        
        input[strcspn(input, "\n")] = 0;

        if (strcmp(input, "exit") == 0) break;

        if (send(client, input, strlen(input), 0) < 0) {
            perror("Send failed");
            break;
        }
    }

    close(client);
    return 0;
}