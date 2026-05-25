#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <ctype.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/wait.h>

void *client_thread(void *);

int main()
{
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1) 
    {
        perror("socket() failed");
        return 1;
    }
    
    if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int))) 
    {
        perror("setsockopt() failed");
        close(listener);
        return 1;
    }
    
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(8080);
    
    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr))) 
    {
        perror("bind() failed");
        close(listener);
        return 1;
    }
    
    if (listen(listener, 5)) 
    {
        perror("listen() failed");
        close(listener);
        return 1;
    }

    printf("Server is listening on port 8080...\n");

    int num_thread = 10;
    pthread_t threads[10]; 
    
    for(int i = 0; i < num_thread; i++)
    {
        int ret = pthread_create(&threads[i], NULL, client_thread, &listener);
        if(ret != 0)
        {
            printf("Could not create new thread\n");
            sched_yield();
        }
    }

    for(int i = 0; i < num_thread; i++)
    {
        pthread_join(threads[i], NULL);
    }

    close(listener);
    return 0;
}

void *client_thread(void *params)
{
    int listener = *(int *)params;
    char buf[256];

    while(1)
    {
        int client = accept(listener, NULL, NULL);
        if (client < 0) continue; 

        printf("New client %d connected\n", client);

        int ret = recv(client, buf, sizeof(buf) - 1, 0); 
        
        if(ret <= 0)
        {
            printf("Client %d disconnected\n", client);
            close(client);
            continue; 
        }

        buf[ret] = 0;
        printf("Msg from client %d: %s\n", client, buf);

        char *msg = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\nXin chao cac ban</h1></body></html>";
        send(client, msg, strlen(msg), 0);
        close(client);
    }
    return NULL;
}