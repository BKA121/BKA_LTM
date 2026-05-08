#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <signal.h>
#include <sys/wait.h>
#include <time.h>

void get_formatted_date(char *output, int size, const char *format_type) 
{
    time_t t = time(NULL);
    struct tm *now = localtime(&t);

    strftime(output, size, format_type, now);
}

void signal_handler(int sig) 
{
    int pid = wait(NULL);
    while ((pid = waitpid(-1, NULL, WNOHANG)) > 0) 
    {
        printf("Child process terminated: %d\n", pid);
    }
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
    
    // Server is now listening for incoming connections
    printf("Server is listening on port 8080...\n");

    signal(SIGCHLD, signal_handler);

    while(1)
    {
        int client = accept(listener, NULL, NULL);
        printf("New client connected: %d\n", client);

        if(fork() == 0)
        {
            close(listener);

            char buf[50];
            while(1)
            {
                int ret = recv(client, buf, sizeof(buf), 0);
                if(ret < 0)
                {
                    perror("recv() failed");
                    break;
                }

                if(ret == 0)
                {
                    printf("Client %d disconnected\n", client);
                    break;
                }

                buf[ret] = 0;
                printf("Msg from client %d: %s\n", client, buf);

                char cmd[10], format[12];
                if(sscanf(buf, "%s %s", cmd, format) == 2)
                {
                    int flag = 1;
                    char date_str[64];
                    if(strcmp(cmd, "GET_TIME") == 0)
                    {
                        if(strcmp(format, "dd/mm/yyyy") == 0)
                        {
                            get_formatted_date(date_str, sizeof(date_str), "%d/%m/%Y");
                        }
                        else if(strcmp(format, "dd/mm/yy") == 0)
                        {
                            get_formatted_date(date_str, sizeof(date_str), "%d/%m/%y");
                        }
                        else if(strcmp(format, "mm/dd/yyyy") == 0)
                        {
                            get_formatted_date(date_str, sizeof(date_str), "%m/%d/%Y");
                        }
                        else if(strcmp(format, "mm/dd/yy") == 0)
                        {
                            get_formatted_date(date_str, sizeof(date_str), "%m/%d/%y");
                        }
                        else flag = 0;
                    }
                    else flag = 0;

                    if(flag == 0)
                    {
                        send(client, "Nhap lai lenh: ", 15, 0);
                    }
                    else
                    {
                        date_str[strlen(date_str)] = '\n';
                        send(client, date_str, strlen(date_str), 0);
                    }
                }
            }
            close(client);
            exit(0);
        }
        else close(client);
    }
}