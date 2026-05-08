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
#include <signal.h>
#include <sys/wait.h>

void signalHandler(int sig) 
{
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

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

    signal(SIGCHLD, signalHandler);

    int num_process = 10;
    for(int i=0; i<num_process; i++)
    {
        if(fork() == 0)
        {
            char buf[256];

            while(1)
            {
                int client = accept(listener, NULL, NULL);
                printf("New client %d connected\n", client);

                int ret = recv(client, buf, sizeof(buf), 0);
                if(ret <= 0)
                {
                    printf("Client %d disconnected\n", client);
                    close(client);
                    exit(0);
                }

                buf[ret] = 0;
                printf("Msg from client %d: %s\n", client, buf);

                char *msg =  "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\nXin chao cac ban</h1></body></html>";
                send(client, msg, strlen(msg), 0);
                close(client);
                exit(0);
            }
        }
    }
    while(wait(NULL) > 0);
}