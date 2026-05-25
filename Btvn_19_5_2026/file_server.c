#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>

#define DIR_PATH "./files_folder"

void sigchld_handler(int sig) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

int main() {
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1) {
        perror("socket() failed");
        return 1;
    }
    
    if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int))) {
        perror("setsockopt() failed");
        close(listener);
        return 1;
    }
    
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(8080);
    
    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr))) {
        perror("bind() failed");
        close(listener);
        return 1;
    }
    
    if (listen(listener, 5)) {
        perror("listen() failed");
        close(listener);
        return 1;
    }

    printf("File Server đang lắng nghe trên cổng 8080...\n");

    signal(SIGCHLD, sigchld_handler);

    while (1) {
        int client = accept(listener, NULL, NULL);
        if (client < 0) continue;

        printf("Có client mới kết nối: %d\n", client);

        if (fork() == 0) {
            close(listener); 

            DIR *d;
            struct dirent *dir;
            int file_count = 0;
            char file_list[4096] = ""; 

            d = opendir(DIR_PATH);
            if (d) {
                while ((dir = readdir(d)) != NULL) {
                    if (dir->d_type == DT_REG) {
                        file_count++;
                        strcat(file_list, dir->d_name);
                        strcat(file_list, "\r\n"); 
                    }
                }
                closedir(d);
            }

            if (file_count == 0) {
                char *err_msg = "ERROR No files to download\r\n";
                send(client, err_msg, strlen(err_msg), 0);
                printf("Thư mục rỗng, ngắt kết nối client %d\n", client);
                close(client);
                exit(0);
            }

            char header[256];
            sprintf(header, "OK %d\r\n", file_count);
            send(client, header, strlen(header), 0);          
            send(client, file_list, strlen(file_list), 0);    
            send(client, "\r\n", 2, 0);                       

            char buf[256];
            while (1) {
                int ret = recv(client, buf, sizeof(buf) - 1, 0);
                if (ret <= 0) {
                    printf("Client %d đã ngắt kết nối\n", client);
                    break;
                }

                buf[ret] = 0;

                buf[strcspn(buf, "\r\n")] = 0; 

                char filepath[512];
                snprintf(filepath, sizeof(filepath), "%s/%s", DIR_PATH, buf);

                struct stat st;
                if (stat(filepath, &st) == 0 && S_ISREG(st.st_mode)) {

                    char file_header[256];
                    sprintf(file_header, "OK %ld\r\n", st.st_size);
                    send(client, file_header, strlen(file_header), 0);

                    int fd = open(filepath, O_RDONLY);
                    if (fd >= 0) {
                        char file_buf[1024];
                        int bytes_read;
                        while ((bytes_read = read(fd, file_buf, sizeof(file_buf))) > 0) {
                            send(client, file_buf, bytes_read, 0);
                        }
                        close(fd);
                    }
                    
                    printf("Đã gửi xong file '%s' cho client %d. Đóng kết nối.\n", buf, client);
                    break; 
                } else {
                    char *err = "ERROR File không tồn tại. Vui lòng gửi lại tên file:\r\n";
                    send(client, err, strlen(err), 0);
                }
            }

            close(client);
            exit(0); 
        }
        
        close(client); 
    }

    close(listener);
    return 0;
}