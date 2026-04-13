#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/select.h>

#define Max_client 1020

void removeClient(int *clients, int *numclients, int i)
{
    if(i<*numclients -1)
    {
        clients[i] = clients[*numclients-1];
    }
    *numclients -= 1;
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
    char buf[256];
    struct timeval tv;

    while(1)
    {
        FD_ZERO(&fdread);

        FD_SET(listener, &fdread);
        int maxdp = listener + 1;
        for(int i=0; i<numclients; i++)
        {
            FD_SET(clients[i], &fdread);
            if(maxdp < clients[i]+1) maxdp = clients[i]+1;
        }

        tv.tv_sec = 5;
        tv.tv_usec = 0;

        res = select(maxdp, &fdread, NULL, NULL, &tv);

        if(res == -1)
        {
            perror("select() failed");
            return 1;
        }
        if(res == 0)
        {
            printf("Time out\n");
            continue;
        }

        if(FD_ISSET(listener, &fdread))
        {
            int client = accept(listener, NULL, NULL);
            if(numclients < Max_client)
            {
                clients[numclients] = client;
                numclients ++;
                printf("New client connected: %d\n", client);
                char msg[] = "Welcome\n";
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

                if(res <= 0)
                {
                    printf("Client %d disconnected\n", clients[i]);
                    removeClient(clients, &numclients, i);
                    i--;
                    continue;
                }

                buf[res] = 0;
                printf("Msg from client %d: %s\n", clients[i], buf);
            }
        }
    }
    close(listener);
    return 0;
}