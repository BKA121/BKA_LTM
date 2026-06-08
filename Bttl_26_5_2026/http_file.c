#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <signal.h>
#include <pthread.h>
#include <sys/wait.h>
#include <sys/stat.h>  
#include <dirent.h>

void decode_url(const char *src, char *dest) {
    while (*src) {
        if (*src == '%' && *(src + 1) && *(src + 2)) {
            int val;
            sscanf(src + 1, "%2x", &val);
            *dest++ = (char)val;
            src += 3;
        } else {
            *dest++ = *src++;
        }
    }
    *dest = '\0';
}

const char *get_content_type(const char *path) {
    const char *ext = strrchr(path, '.');
    if (!ext) return "application/octet-stream"; 

    if (strcmp(ext, ".html") == 0 || strcmp(ext, ".htm") == 0) return "text/html";
    if (strcmp(ext, ".txt") == 0 || strcmp(ext, ".c") == 0 || strcmp(ext, ".cpp") == 0) return "text/plain";
    if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0) return "image/jpeg";
    if (strcmp(ext, ".png") == 0) return "image/png";
    if (strcmp(ext, ".mp3") == 0) return "audio/mpeg";
    if (strcmp(ext, ".mp4") == 0) return "video/mp4";
    
    return "application/octet-stream";
}

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

    while(1)
    {
        int client = accept(listener, NULL, NULL);
        printf("New client connected: %d\n", client);

        pthread_t thread_id;
        pthread_create(&thread_id, NULL, client_thread, &client);
        pthread_detach(thread_id);
    }

    close(listener);
    return 0;
}

void *client_thread(void *params) 
{
    int client = *(int *)params;
    char buffer[2048];
    memset(buffer, 0, sizeof(buffer));

    int bytes_received = recv(client, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received <= 0) 
    {
        close(client);
        return NULL;
    }

    if (strncmp(buffer, "GET /favicon.ico", 16) == 0) 
    {
        close(client);
        return NULL;
    }

    char req_path[1024] = {0};
    if (sscanf(buffer, "GET %1023s HTTP", req_path) != 1) 
    {
        close(client);
        return NULL;
    }

    char decoded_req_path[1024] = {0};
    decode_url(req_path, decoded_req_path);

    char local_path[2048];
    sprintf(local_path, ".%s", decoded_req_path);
    struct stat st;
    if (stat(local_path, &st) == -1) 
    {
        char *not_found = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n\r\n<h1>404 File Not Found!</h1>";
        send(client, not_found, strlen(not_found), 0);
    }
    else if (S_ISDIR(st.st_mode)) 
    {
        DIR *dir = opendir(local_path);
        if (dir) 
        {
            char *header = "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\n\r\n"
                           "<html><body><h2>Danh sach thu muc</h2><hr>";
            send(client, header, strlen(header), 0);

            char back_btn[256];
            sprintf(back_btn, "<a href='..'><b>[Quay lai]</b></a><br><br>");
            send(client, back_btn, strlen(back_btn), 0);

            struct dirent *ent;
           char item_html[8192]; 
            char child_path[4096];

            while ((ent = readdir(dir)) != NULL) 
            {
                if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) continue;

                snprintf(child_path, sizeof(child_path), "%s/%s", local_path, ent->d_name);
                struct stat child_st;
                
                if (stat(child_path, &child_st) == 0) 
                {
                    char url_href[4096]; 
                    
                    if (strcmp(decoded_req_path, "/") == 0)
                        snprintf(url_href, sizeof(url_href), "/%s", ent->d_name);
                    else
                        snprintf(url_href, sizeof(url_href), "%s/%s", decoded_req_path, ent->d_name);
                    if (S_ISDIR(child_st.st_mode)) {
                        snprintf(item_html, sizeof(item_html), "<a href='%s'><b>%s/</b></a><br>", url_href, ent->d_name);
                    } else {
                        snprintf(item_html, sizeof(item_html), "<a href='%s'><i>%s</i></a><br>", url_href, ent->d_name);
                    }
                    
                    send(client, item_html, strlen(item_html), 0);
                }
            }
            closedir(dir);
            char *footer = "</body></html>";
            send(client, footer, strlen(footer), 0);
        }
    }
    else if (S_ISREG(st.st_mode)) 
    {
        FILE *fp = fopen(local_path, "rb"); 
        if (fp) 
        {
            char header[512];
            sprintf(header, "HTTP/1.1 200 OK\r\nContent-Type: %s\r\n\r\n", get_content_type(local_path));
            send(client, header, strlen(header), 0);

            char file_buf[4096];
            int bytes_read;
            while ((bytes_read = fread(file_buf, 1, sizeof(file_buf), fp)) > 0) 
            {
                send(client, file_buf, bytes_read, 0);
            }
            fclose(fp);
        }
    }
    close(client);
}