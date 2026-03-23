#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

int main()
{
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9090);
    int res = bind(listener, (struct sockaddr *)&addr, sizeof(addr));
    if(res<0)
    {
        perror("bind() failed"); exit(1);
    }

    listen(listener, 5);

    char buf[2048];
    int client = accept(listener, NULL, NULL);
    int n = recv(client, buf, sizeof(buf), 0);
    buf[n] = 0;
    printf("%s\n", buf);

    char msg[] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body><h1>Hello World</h1></body></html>";
    send(client, msg, strlen(msg), 0);
    close(client);
    close(listener);
}