#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <ctype.h>

#define PORT 8080
#define MAX_CLIENTS 64

#include <stdio.h>
#include <string.h>
#include <ctype.h>

void format_email(char *input, char *output) {
    char name_part[128] = {0};
    char mssv_part[16] = {0};
    char *words[20]; 
    int word_count = 0;

    char *token = strtok(input, ",");
    if (token) {
        strncpy(name_part, token, sizeof(name_part) - 1);
        token = strtok(NULL, ",");
        if (token) {
            while (isspace(*token)) token++;
            strncpy(mssv_part, token, sizeof(mssv_part) - 1);
            mssv_part[strcspn(mssv_part, "\r\n")] = 0;
        }
    }

    char *name_token = strtok(name_part, " ");
    while (name_token != NULL && word_count < 20) {
        words[word_count++] = name_token;
        name_token = strtok(NULL, " ");
    }

    if (word_count > 0 && strlen(mssv_part) > 0) {
        char processed_name[128] = {0};
        int p = 0;

        char *last_name = words[word_count - 1];
        for (int i = 0; last_name[i]; i++) 
            processed_name[p++] = tolower(last_name[i]);

        for (int i = 0; i < word_count - 1; i++) {
            processed_name[p++] = tolower(words[i][0]);
        }
        processed_name[p] = '\0';
        sprintf(output, "%s%s@sis.hust.edu.vn\n", processed_name, mssv_part);
    }
}

int main() {
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1) { perror("socket() failed"); return 1; }

    unsigned long ul = 1;
    ioctl(listener, FIONBIO, &ul);

    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(PORT);

    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr))) {
        perror("bind() failed"); return 1;
    }

    listen(listener, 5);

    int clients[MAX_CLIENTS];
    int nclients = 0;
    char buf[256];

    while (1) {
        int client = accept(listener, NULL, NULL);
        if (client != -1) {
            char *welcome = "Hay nhap 'Ho ten, MSSV': ";
            send(client, welcome, strlen(welcome), 0);

            ul = 1;
            ioctl(client, FIONBIO, &ul);
            clients[nclients++] = client;
        }

        for (int i = 0; i < nclients; i++) {
            int len = recv(clients[i], buf, sizeof(buf) - 1, 0);
            
            if (len == -1) {
                if (errno != EWOULDBLOCK) {
                    close(clients[i]);
                    clients[i] = clients[--nclients]; 
                }
            } else if (len == 0) {
                printf("Client %d da ngat ket noi.\n", clients[i]);
                close(clients[i]);
                clients[i] = clients[--nclients];
                i--;
            } else {
                buf[len] = 0;
                printf("Nhan tu %d: %s", clients[i], buf);

                char response[256];
                format_email(buf, response);
                send(clients[i], response, strlen(response), 0);
            }
        }
        
        usleep(10000); 
    }

    close(listener);
    return 0;
}