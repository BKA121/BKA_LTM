#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <signal.h>

int main()
{
    int client = socket(AF_INET, SOCK_STREAM, IPROTO_TCP);

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(8080);

    int res = connect(client, (struct sockaddr *)&addr, sizeof(addr));
    if(res == -1)
    {
        perror("connect() failed");
        close(client);
        return 1;
    }

    printf("Connected to server on port 8080...\n");

    if(fork() == 0)
    {
        char buf[256];
        while(1)
        {
            fgets(buf, sizeof(buf), stdin);
            send(client, buf, strlen(buf), 0);
            if (strcmp(buf, "exit\n") == 0)
                break;
        }
    }
    else
    {
        char buf[256];
        while (1)
        {
            int len = recv(client, buf, sizeof(buf), 0);
            if (len <= 0)
                break;
            printf("Received: %s\n", buf);
        }
    }

    kill(0, SIGKILL);

    close(client);
    return 0;
}