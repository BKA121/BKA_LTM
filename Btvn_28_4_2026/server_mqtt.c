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
#define Max_topics 10
#define Max_topic_len 20
#define Buf_size 256

struct client_info
{
    int fd;
    char topic[Max_topics][Max_topic_len];
    int numTopic;
};

void RemoveClient(struct pollfd *fds, struct client_info *clients, int *nfds, int i)
{
    close(fds[i].fd);
    fds[i] = fds[*nfds-1];
    clients[i-1] = clients[*nfds-2];
    (*nfds) --;
}

int main()
{
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9000);

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

    printf("Server is listening on port 9000...\n");

    struct pollfd fds[Max_client];
    struct client_info clients[Max_client];
    int nfds = 1;
    fds[0].fd = listener;
    fds[0].events = POLLIN;

    while(1)
    {
        int ret = poll(fds, nfds, -1);
        if (ret < 0) {
            perror("poll() failed");
            break;
        }

        if(fds[0].revents & POLLIN)
        {
            int client = accept(listener, NULL, NULL);
            if(nfds < Max_client)
            {
                printf("New client connected: %d\n", client);
                fds[nfds].fd = client;
                fds[nfds].events = POLLIN;

                clients[nfds-1].fd = client;
                clients[nfds-1].numTopic = 0;

                nfds ++;
            }
            else close(client);
        }

        for(int i=1; i<nfds; i++)
        {
            if(fds[i].revents & POLLIN)
            {
                char buf[Buf_size];
                int nbit = recv(fds[i].fd, buf, sizeof(buf), 0);
                if(nbit <= 0)
                {
                    RemoveClient(fds, clients, &nfds, i);
                    i--;
                    continue;
                }

                buf[nbit] = 0;
                char cmd[8], topic[Max_topic_len], msg[Buf_size];
                if(sscanf(buf, "%s %s %[^\n]", cmd, topic, msg)>=2)
                {
                    if(strcmp(cmd, "SUB") == 0)
                    {
                        printf("Message from client %d: %s %s\n", fds[i].fd , cmd, topic);
                        int found = 0;
                        for(int t=0; t < clients[i-1].numTopic; t++)
                            if(strcmp(clients[i-1].topic[t], topic) == 0) found = 1;
                        
                        if (!found && clients[i-1].numTopic < Max_topics) 
                        {
                            strcpy(clients[i-1].topic[clients[i-1].numTopic++], topic);
                            send(fds[i].fd, "Subscribed OK\n", 14, 0);
                        }
                    }

                    else if(strcmp(cmd, "UNSUB") == 0)
                    {
                        printf("Message from client %d: %s %s\n", fds[i].fd , cmd, topic);
                        for(int t=0; t < clients[i-1].numTopic; t++)
                        {
                            if(strcmp(clients[i-1].topic[t], topic) == 0)
                            {
                                strcpy(clients[i-1].topic[t], clients[i-1].topic[clients[i-1].numTopic-1]);
                                clients[i-1].numTopic --;
                                send(fds[i].fd, "Unsubscribed OK\n", 16, 0);
                                break;
                            }
                        }
                    }

                    else if(strcmp(cmd, "PUB") == 0)
                    {
                        printf("Message from client %d: %s %s %s\n", fds[i].fd , cmd, topic, msg);
                        char formatted_msg[280];
                        snprintf(formatted_msg, sizeof(formatted_msg), "[%s]: %s\n", topic, msg);
                        for(int j = 0; j<nfds-1; j++)
                        {
                            for(int k=0; k<clients[j].numTopic; k++)
                            {
                                if(strcmp(clients[j].topic[k], topic) == 0)
                                {
                                    send(clients[j].fd, formatted_msg, strlen(formatted_msg), 0);
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    close(listener);
    return 0;
}