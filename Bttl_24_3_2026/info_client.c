#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

int main() {
    int client_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(9090);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(client_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connect failed");
        return 1;
    }

    char buffer[2048];
    int pos = 0;

    getcwd(buffer + pos, 1024); 
    pos += strlen(buffer + pos) + 1; 

    int file_count_pos = pos;
    int num_files = 0;
    pos += sizeof(int);

    DIR *d = opendir(".");
    struct dirent *dir;
    struct stat st;

    while ((dir = readdir(d)) != NULL) {
        if (stat(dir->d_name, &st) == 0 && S_ISREG(st.st_mode)) {

            strcpy(buffer + pos, dir->d_name);
            pos += strlen(dir->d_name) + 1;

            memcpy(buffer + pos, &st.st_size, sizeof(long));
            pos += sizeof(long);
            num_files++;
        }
    }
    closedir(d);

    memcpy(buffer + file_count_pos, &num_files, sizeof(int));

    send(client_sock, buffer, pos, 0);

    close(client_sock);
    return 0;
}