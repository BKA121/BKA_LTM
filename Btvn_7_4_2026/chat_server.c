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

struct client_info
{
    char client_id[30];
    int sock_client;
};

void removeClient(struct client_info *clients, int *numclients, int i)
{
    close(clients[i].sock_client);
    if(i<*numclients -1)
    {
        clients[i] = clients[*numclients-1];
    }
    *numclients -= 1;
}

bool CheckID(char* buf, char tmp[]) {
    char *colon_ptr = strchr(buf, ':');
    
    if (colon_ptr == NULL || colon_ptr == buf) {
        return false;
    }

    int id_len = colon_ptr - buf;
    strncpy(tmp, buf, id_len);
    tmp[id_len] = '\0'; 

    const char *name_ptr = colon_ptr + 1;

    while (*name_ptr == ' ') {
        name_ptr++;
    }

    if (*name_ptr == '\0' || *name_ptr == '\n' || *name_ptr == '\r') {
        return false;
    }

    int i = 0;
    while (name_ptr[i] != '\0' && name_ptr[i] != '\n' && name_ptr[i] != '\r') {
        if (isspace((unsigned char)name_ptr[i])) {
            return false;
        }
        i++;
    }
    return true;
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

    struct client_info clients[Max_client];
    int numclients = 0;

    fd_set fdread;
    struct timeval tv;
    char buf[256];
    char publicMsg[300];
    int len = 0;

    while(1)
    {
        FD_ZERO(&fdread);

        FD_SET(listener, &fdread);
        int maxdp = listener + 1;
        for(int i=0; i<numclients; i++)
        {
            FD_SET(clients[i].sock_client,  &fdread);
            if(clients[i].sock_client + 1 > maxdp) maxdp = clients[i].sock_client + 1;
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
                printf("New client connected: %d\n", client);
                clients[numclients].sock_client = client;
                numclients ++;
                clients[numclients].client_id[0] = '\0';
                char msg[] = "Please send using the following format 'client_id: client_name'\n";
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
            if(FD_ISSET(clients[i].sock_client, &fdread))
            {
                memset(buf, 0, sizeof(buf));
                res = recv(clients[i].sock_client, buf, sizeof(buf), 0);
                if(res <= 0)
                {
                    printf("Client %d disconnected\n", clients[i].sock_client);
                    removeClient(clients, &numclients, i);
                    i--;
                    continue;
                }

                if(strlen(clients[i].client_id) == 0)
                {
                    char tmp[30];
                    if(CheckID(buf, tmp))
                    {
                        strcpy(clients[i].client_id, tmp);
                        char msg[64];
                        sprintf(msg, "Welcome client: %s", clients[i].client_id);
                        send(clients[i].sock_client, msg, strlen(msg), 0);
                    }
                    else
                    {
                        char msg[] = "Please send using the following format 'client_id: client_name'\n";
                        send(clients[i].sock_client, msg, strlen(msg), 0);
                    }
                }
                else // da luu id roi, server gui msg cho cac client khac
                {
                    printf("Message from %s: %s\n", clients[i].client_id, buf);
                    memset(publicMsg, 0, sizeof(publicMsg));
                    sprintf(publicMsg, "%s: %s", clients[i].client_id, buf);
                    for(int j=0; j<numclients; j++)
                    {
                        if(i != j)
                        {
                            send(clients[j].sock_client, publicMsg, strlen(publicMsg), 0);
                        }
                    }
                }
            }
        }
    }
    close(listener);
    return 0;
}