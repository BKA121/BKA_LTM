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
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(listener == -1)
    {
        perror("socket() failed");
        return 1;
    }

    unsigned long ul = 1;
    ioctl(listener, FIONBIO, &ul);

    // Giai phong port
    if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int))) {
        perror("setsockopt() failed");
        close(listener);
        return 1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(8080);

    int res = bind(listener, (struct sockaddr *)&addr, sizeof(addr));
    if(res<0)
    {
        perror("bind() failed");
        return 1;
    }

    res = listen(listener, 5);
    if(res<0)
    {
        perror("listen() failed");
        return 1;
    }

    printf("Server is listening on port 8080...\n");

    int clients[64];
    int nclients = 0;
    int len;
    char buf[256];

    while(1)
    {
        int client = accept(listener, NULL, NULL);

        if(client == -1) // accept loi
        {
            if(errno != EWOULDBLOCK)
            {
                perror("accept() failed");
                return 1;
            }
            else
            {
                // Loi do vao ra, lat quay lai xu ly sau
            }
        }
        else
        {
            printf("new client connected: %d\n", client);
            clients[nclients] = client;
            nclients ++;
            ul = 1;
            ioctl(client, FIONBIO, &ul);
        }

        for(int i=0; i<nclients; i++)
        {
            len = recv(clients[i], buf, sizeof(buf), 0);

            if(len == -1) // ma loi cua recv
            {
                if(errno != EWOULDBLOCK)
                {
                    perror("recv() failed");
                    continue;
                }
                else{}
            }
            else if(len == 0)
            {
                printf("Client disconnected\n");
                close(clients[i]);
                clients[i] = clients[nclients-1];
                nclients --;
                i--;
                continue;
            }
            else
            {
                buf[len] = 0;
                printf("Msg from client %d: %s\n", clients[i], buf);
            }
        }
    }

    close(listener);
    return 0;
}