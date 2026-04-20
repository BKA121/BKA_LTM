#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <poll.h> 
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
    if(i < *numclients - 1)
    {
        clients[i] = clients[*numclients - 1];
    }
    *numclients -= 1;
}

bool CheckID(char* buf, char tmp[]) {
    char *colon_ptr = strchr(buf, ':');
    if (colon_ptr == NULL || colon_ptr == buf) return false;

    int id_len = colon_ptr - buf;
    if (id_len >= 30) id_len = 29; 
    strncpy(tmp, buf, id_len);
    tmp[id_len] = '\0'; 

    const char *name_ptr = colon_ptr + 1;
    while (*name_ptr == ' ') name_ptr++;
    if (*name_ptr == '\0' || *name_ptr == '\n' || *name_ptr == '\r') return false;

    int i = 0;
    while (name_ptr[i] != '\0' && name_ptr[i] != '\n' && name_ptr[i] != '\r') {
        if (isspace((unsigned char)name_ptr[i])) return false;
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

    if(bind(listener, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        perror("bind() failed");
        return 1;
    }

    if(listen(listener, 5) == -1)
    {
        perror("listen() failed");
        return 1;
    }

    printf("Server is listening on port 8080...\n");

    struct client_info clients[Max_client];
    int numclients = 0;

    struct pollfd fds[Max_client + 1]; 
    char buf[256];
    char publicMsg[300];

    while(1)
    {
        fds[0].fd = listener;
        fds[0].events = POLLIN; 
        
        for(int i = 0; i < numclients; i++)
        {
            fds[i+1].fd = clients[i].sock_client;
            fds[i+1].events = POLLIN; 
        }

        int res = poll(fds, numclients + 1, 5000);
        
        if(res == -1)
        {
            perror("poll() failed");
            return 1;
        }
        if(res == 0)
        {
            printf("Time out\n");
            continue;
        }

        if(fds[0].revents & POLLIN)
        {
            int client = accept(listener, NULL, NULL);
            if(numclients < Max_client)
            {
                printf("New client connected: %d\n", client);
                clients[numclients].sock_client = client;
                clients[numclients].client_id[0] = '\0';
                numclients++;

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

        for(int i = 0; i < numclients; i++)
        {
            if(fds[i+1].revents & POLLIN)
            {
                memset(buf, 0, sizeof(buf));
                int nbit = recv(clients[i].sock_client, buf, sizeof(buf), 0);
                
                if(nbit <= 0)
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
                        sprintf(msg, "Welcome client: %s\n", clients[i].client_id);
                        send(clients[i].sock_client, msg, strlen(msg), 0);
                    }
                    else
                    {
                        char msg[] = "Please send using the following format 'client_id: client_name'\n";
                        send(clients[i].sock_client, msg, strlen(msg), 0);
                    }
                }
                else 
                {
                    printf("Message from %s: %s\n", clients[i].client_id, buf);
                    memset(publicMsg, 0, sizeof(publicMsg));
                    sprintf(publicMsg, "%s: %s", clients[i].client_id, buf);
                    for(int j = 0; j < numclients; j++)
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