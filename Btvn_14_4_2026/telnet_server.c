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

struct client_info {
    int is_authenticated;
    int sock_client;
};

int check_login(char *user, char *pass) {
    FILE *f = fopen("database.txt", "r");
    if (f == NULL) return 0;

    char f_user[50], f_pass[50];
    while (fscanf(f, "%s %s", f_user, f_pass) != EOF) {
        if (strcmp(user, f_user) == 0 && strcmp(pass, f_pass) == 0) {
            fclose(f);
            return 1;
        }
    }
    fclose(f);
    return 0;
}

void removeClient(struct client_info *clients, int *numclients, int i) {
    close(clients[i].sock_client);
    if(i < *numclients - 1) {
        clients[i] = clients[*numclients - 1];
    }
    *numclients -= 1;
}

int main() {
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(8080);

    if(bind(listener, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("bind() failed");
        return 1;
    }

    if(listen(listener, 5) == -1) {
        perror("listen() failed");
        return 1;
    }

    printf("Server is listening on port 8080...\n");

    struct client_info clients[Max_client];
    int numclients = 0;

    struct pollfd fds[Max_client + 1];
    char buf[256];

    while(1) {
        fds[0].fd = listener;
        fds[0].events = POLLIN;

        for(int i = 0; i < numclients; i++) {
            fds[i + 1].fd = clients[i].sock_client;
            fds[i + 1].events = POLLIN;
        }

        int res = poll(fds, numclients + 1, -1);

        if(res == -1) {
            perror("poll() failed");
            break;
        }

        if(fds[0].revents & POLLIN) {
            int client = accept(listener, NULL, NULL);
            if(numclients < Max_client) {
                printf("New client connected: %d\n", client);
                clients[numclients].sock_client = client;
                clients[numclients].is_authenticated = 0; 
                numclients++;
                char msg[] = "Nhap user pass (vidu: admin admin): \n";
                send(client, msg, strlen(msg), 0);
            } else {
                close(client);
            }
        }

        for(int i = 0; i < numclients; i++) {
            if(fds[i + 1].revents & POLLIN) {
                memset(buf, 0, sizeof(buf));
                int res_recv = recv(clients[i].sock_client, buf, sizeof(buf) - 1, 0);
                
                if(res_recv <= 0) {
                    printf("Client %d disconnected\n", clients[i].sock_client);
                    removeClient(clients, &numclients, i);
                    i--; 
                    continue;
                }

                buf[strcspn(buf, "\r\n")] = 0;

                if(!clients[i].is_authenticated) {
                    char user[50], pass[50];
                    if (sscanf(buf, "%s %s", user, pass) == 2 && check_login(user, pass)) {
                        clients[i].is_authenticated = 1;
                        send(clients[i].sock_client, "Login Success! Nhap lenh: ", 26, 0);
                    } else {
                        send(clients[i].sock_client, "Login Fail. Nhap lai: ", 22, 0);
                    }
                } else {
                    char cmd[512];
                    snprintf(cmd, sizeof(cmd), "%s > out.txt 2>&1", buf); 
                    system(cmd);

                    FILE *f = fopen("out.txt", "r");
                    if (f) {
                        char file_buf[256];
                        while (fgets(file_buf, sizeof(file_buf), f) != NULL) {
                            send(clients[i].sock_client, file_buf, strlen(file_buf), 0);
                        }
                        fclose(f);
                    }
                    send(clients[i].sock_client, "\nNhap lenh: ", 12, 0);
                }
            }
        }
    }
    close(listener);
    return 0;
}