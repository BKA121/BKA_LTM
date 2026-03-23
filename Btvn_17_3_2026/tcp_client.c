#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

int main(int argc, char * argv[])
{
    if(argc<3)
    {
        printf("Nhap thieu tham so, nhap lai");
        return 1;
    }

    int port = atoi(argv[2]);

    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(argv[1]);
    addr.sin_port = htons(port);

    int res = connect(client, (struct sockaddr *)&addr, sizeof(addr));
    if(res<0)
    {
        perror("Connect() failed"); exit(1);
    }

    char buf[64];
    int n = recv(client, buf, sizeof(buf), 0);
    buf[n] = 0;
    printf("Msg from server: %s\n", buf);

    char msg[64];
    printf("Nhap message:\n");
    while(1)
    {
        fgets(msg, sizeof(msg), stdin);
        send(client, msg, strlen(msg), 0);

        if(strncmp(msg, "exit", 4) == 0) break;
    }

    close(client);
}