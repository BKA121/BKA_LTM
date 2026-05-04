#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/select.h>
#include <ctype.h>
#include <stdbool.h>

#define Max_client 1020

void removeClient(int *clients, int *numclients, int i)
{
    close(clients[i]);
    if(i<*numclients -1)
    {
        clients[i] = clients[*numclients-1];
    }
    *numclients -= 1;
}

void EMsg(char * buf)
{
    int len = strlen(buf);
    for(int i=0; i<len; i++)
    {
        if('0' <= buf[i] && buf[i] <= '9') buf[i] = (9 - (buf[i] - '0')) + '0';

    }
}

int main()
{
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(8080);

    int res = bind(listener, (struct sockaddr *)&addr, sizeof(addr));
    if(res == -1)
    {
        perror("bind() failed");
        return 1;
    }

    res = listen(listener, 5);
    if(res == -1)
    {
        perror("listen() failed");
        return 1;
    }

    printf("Server is listening on port 8080...\n");

    int clients[Max_client];
    int numclients = 0;
    fd_set fdread;
    struct timeval tv;
    char buf[256];

    while(1)
    {
        FD_ZERO(&fdread);
        FD_SET(listener, &fdread);
        int maxdp = listener + 1;
        for(int i=0; i<numclients; i++)
        {
            FD_SET(clients[i], &fdread);
            if(clients[i] + 1 > maxdp) maxdp = clients[i] + 1;
        }

        tv.tv_sec = 5;
        tv.tv_usec = 0;

        res = select(maxdp, &fdread, NULL, NULL, &tv);

        if(res<0)
        {
            perror("select() failed");
        }

        if(res == 0)
        {
            printf("Time out\n");
        }

        if(FD_ISSET(listener, &fdread))
        {
            int client = accept(listener, NULL, NULL);
            if(numclients < Max_client)
            {
                clients[numclients++] = client;

                char msg[100];
                sprintf(msg, "Xin chao, hien tai dang co %d client dang ket noi.\n", numclients);

                send(client, msg, strlen(msg), 0);
            }
            else
            {
                char msg[] = "Out of slot\n";
                send(client, msg, strlen(msg), 0);
                close(client);
            }
        }

        for(int i=0; i<numclients; i++)
        {
            if(FD_ISSET(clients[i], &fdread))
            {
                res = recv(clients[i], buf, sizeof(buf), 0);
                if(res<=0)
                {
                    printf("Client %d disconnected\n", clients[i]);
                    removeClient(clients, &numclients, i);
                    i--;
                    continue;
                }

                buf[res] = 0;
                if(strcmp(buf, "exit") == 0)
                {
                    printf("Client %d disconnected\n", clients[i]);
                    removeClient(clients, &numclients, i);
                    i--;
                    continue;
                }
                printf("Msg from client %d: %s\n", clients[i], buf);

                EMsg(buf);
                send(clients[i], buf, strlen(buf), 0);
            }
        }
    }
    close(listener);
}