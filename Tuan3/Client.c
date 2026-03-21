#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

int main()
{
    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(9000);

    int res = connect(client, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if(res == -1)
    {
        printf("Connect failed");
    }

    char msg[] = "hello server!\n";
    send(client, msg, strlen(msg), 0);

    char buf[2048];
    int len = recv(client, buf, sizeof(buf), 0);
    buf[len] = 0;
    printf("data received: %s", buf);

    close(client);
    return 0;
}