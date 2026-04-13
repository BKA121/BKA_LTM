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
    fd_set fdread;
    char buf[256];

    while(1) {
        FD_ZERO(&fdread);
        FD_SET(listener, &fdread);
        int maxdp = listener;
        for(int i = 0; i < numclients; i++) {
            FD_SET(clients[i].sock_client, &fdread);
            if(clients[i].sock_client > maxdp) maxdp = clients[i].sock_client;
        }

        if(select(maxdp + 1, &fdread, NULL, NULL, NULL) == -1) {
            perror("select() failed");
            break;
        }

        if(FD_ISSET(listener, &fdread)) {
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
            if(FD_ISSET(clients[i].sock_client, &fdread)) {
                memset(buf, 0, sizeof(buf));
                int res = recv(clients[i].sock_client, buf, sizeof(buf) - 1, 0);
                if(res <= 0) {
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
                        while (fgets(buf, sizeof(buf), f) != NULL) {
                            send(clients[i].sock_client, buf, strlen(buf), 0);
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