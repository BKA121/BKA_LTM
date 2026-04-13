#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/select.h>

int main()
{
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(listener == -1)
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
    
    fd_set fdread, fdtest;
    FD_ZERO(&fdread);
    FD_SET(listener, &fdread);

    struct timeval tv;
    char buf[256];

    while(1)
    {
        fdtest = fdread; // dau vong lap reset lai fdtest ban dau

        // reset thoi gian cho do time se tru dan qua moi vong lap
        tv.tv_sec = 5;
        tv.tv_usec = 0;

        int res = select(FD_SETSIZE, &fdtest, NULL, NULL, &tv);

        if(res == -1)
        {
            perror("select() failed");
            break;
        }
        if(res == 0)
        {
            printf("Time out\n");
            continue;
        }

        for(int i=0; i<FD_SETSIZE; i++)
        {
            if(FD_ISSET(i, &fdtest)) // neu con trong danh sach tra ve tuc la co su kien
            {
                if(i == listener)
                {
                    int client = accept(listener, NULL, NULL);
                    if(client < FD_SETSIZE)
                    {
                        printf("new client connected: %d\n", client);
                        FD_SET(client, &fdread);
                    }
                    else close(client);
                }
                else
                {
                    res = recv(i, buf, sizeof(buf), 0);
                    if(res <= 0)
                    {
                        printf("client %d disconnected\n", i);
                        FD_CLR(i, &fdread);
                    }
                    else
                    {
                        buf[res] = 0;
                        printf("Received from %d: %s\n", i, buf);
                    }
                }
            }
        }
    }
    close(listener);
    return 0;
}