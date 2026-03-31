#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

int main() {
    int server_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(9090);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(server_sock, 5);

    int client_sock = accept(server_sock, NULL, NULL);

    char buffer[2048];
    int n = recv(client_sock, buffer, sizeof(buffer), 0);
    int pos = 0;

    printf("%s\n", buffer + pos);
    pos += strlen(buffer + pos) + 1;

    int num_files;
    memcpy(&num_files, buffer + pos, sizeof(int));
    pos += sizeof(int);

    for (int i = 0; i < num_files; i++) {
        char *name = buffer + pos;
        pos += strlen(name) + 1;

        long size;
        memcpy(&size, buffer + pos, sizeof(long));
        pos += sizeof(long);

        printf("%s - %ld bytes\n", name, size);
    }

    close(client_sock);
    close(server_sock);
    return 0;
}