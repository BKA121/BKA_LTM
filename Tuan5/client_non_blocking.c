#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <errno.h>

int main()
{
    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(8080);

    int res = connect(client, (struct sockaddr *)&addr, sizeof(addr));
    if(res == -1)
    {
        perror("connect() failed");
        return 1;
    }

    unsigned long ul = 1;
    ioctl(client, FIONBIO, &ul);
    ioctl(STDIN_FILENO, FIONBIO, &ul);

    printf("Connected to server on port 8080...\n");

    char send_buf[64];
    char recv_buf[64];
    int len;
    while(1)
    {
        if(fgets(send_buf, sizeof(send_buf), stdin)==NULL)
        {
            if(errno == EWOULDBLOCK)
            {
                // cho du lieu nhap
            }
            else break;
        }
        else
        {
            send(client, send_buf, strlen(send_buf), 0);
            if (strcmp(send_buf, "exit\n") == 0) break;
        }

        len = recv(client, recv_buf, sizeof(recv_buf), 0);
        if(len == -1)
        {
            if(errno != EWOULDBLOCK)
            {
                perror("recv() failed");
                break;
            }
            else 
            {

            }
        }
        else
        {
            if(len == 0) break;

            recv_buf[len] = 0;
            printf("Received: %s\n", recv_buf);
        }
    }
    close(client);
    return 0;
}