#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <pthread.h> 
#include <ctype.h>
#include <stdbool.h>

#define Max_client 1020

struct client_info
{
    char client_id[30];
    int sock_client;
};

struct client_info clients[Max_client];
int numclients = 0;

bool CheckID(char* buf, char tmp[]) 
{
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

void removeClient(struct client_info *clients, int i) {
    close(clients[i].sock_client);
    clients[i].sock_client = 0; 
    clients[i].client_id[0] = '\0'; 
}

void *client_thread(void *);

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

    while(1)
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

        pthread_t thread_id;
        pthread_create(&thread_id, NULL, client_thread, &client);
        pthread_detach(thread_id);
    }

    close(listener);
    return 0;
}

void *client_thread(void *params) 
{
    int client = *(int *)params;
    char buf[64];
    char publicMsg[300];
    int i = -1;

    while (1) 
    {
        int len = recv(client, buf, sizeof(buf), 0);
        if (len <= 0)
        {
            printf("Client %d disconnected\n", client);
            break;
        }

        buf[len] = 0;
        printf("Received from %d: %s", client, buf);

        if(i == -1)
        {
            for(int j=0; j<numclients; j++)
            {
                if(clients[j].sock_client == client)
                {
                    i = j;
                    break;
                }
            }
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
            memset(publicMsg, 0, sizeof(publicMsg));
            sprintf(publicMsg, "%s: %s", clients[i].client_id, buf);
            for(int k = 0; k < Max_client; k++) 
            {
                if(clients[k].sock_client > 0 && k != i) 
                    send(clients[k].sock_client, publicMsg, strlen(publicMsg), 0);
            }
        }

    }
    
    removeClient(clients, i);
}