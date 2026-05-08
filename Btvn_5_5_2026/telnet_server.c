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

int Check_login(char *user, char *pass) 
{
    FILE *f = fopen("database.txt", "r");
    if (f == NULL) return 0;

    char f_user[25], f_pass[25];
    while (fscanf(f, "%s %s", f_user, f_pass) != EOF) {
        if (strcmp(user, f_user) == 0 && strcmp(pass, f_pass) == 0) 
        {
            fclose(f);
            return 1;
        }
    }
    fclose(f);
    return 0;
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

    while(1)
    {
        int client = accept(listener, NULL, NULL);

        if(fork() == 0)
        {
            close(listener);
            int authenticated = 0;

            char msg[] = "Nhap user pass (vidu: admin admin): \n";
            send(client, msg, strlen(msg), 0);

            char buf[64];
            while(1)
            {
                memset(buf, 0, sizeof(buf));
                int ret = recv(client, buf, sizeof(buf), 0);
                if(ret <= 0)
                {
                    printf("Client %d disconnected\n", client);
                    close(client);
                    exit(0);
                }

                buf[strcspn(buf, "\r\n")] = 0;
                printf("Msg from client %d: %s\n", client, buf);

                if(authenticated == 0)
                {
                    char msg[35];
                    char user[25], pass[25];
                    if (sscanf(buf, "%s %s", user, pass) == 2 && Check_login(user, pass)) 
                    {
                        authenticated = 1;
                        strcpy(msg, "Login Success! Nhap lenh:");
                    }
                    else
                    {
                        strcpy(msg, "Login Failed!\n");
                    }
                    send(client, msg, strlen(msg), 0);
                }
                else
                {
                    char cmd[256];
                    char out_file[20];
                    sprintf(out_file, "out_%d.txt", getpid()); 
                    snprintf(cmd, sizeof(cmd), "%s > %s 2>&1", buf, out_file);
                    system(cmd);

                    FILE *f = fopen(out_file, "r");
                    if (f) 
                    {
                        char file_buf[256];
                        while (fgets(file_buf, sizeof(file_buf), f) != NULL) 
                        {
                            send(client, file_buf, strlen(file_buf), 0);
                        }
                        fclose(f);
                    }
                    send(client, "\nNhap lenh: ", 12, 0);
                }
            }
        }
        close(client);
    }
    close(listener);
    return 0;
}