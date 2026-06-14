#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int create_connection(const char *ip, int port) 
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return -1;

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &server_addr.sin_addr);

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        return -1;
    }
    return sock;
}

int enter_passive_mode(int control_sock, const char *server_ip) 
{
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));

    send(control_sock, "PASV\r\n", 6, 0);
    recv(control_sock, buffer, sizeof(buffer) - 1, 0);
    printf("Server (21): %s", buffer);

    int h1, h2, h3, h4, p1, p2;
    char *start = strchr(buffer, '(');
    if (start != NULL) {
        sscanf(start, "(%d,%d,%d,%d,%d,%d)", &h1, &h2, &h3, &h4, &p1, &p2);
        int data_port = p1 * 256 + p2;
        return create_connection(server_ip, data_port);
    }
    return -1;
}

void reverse_string(char *str) 
{
    int len = strlen(str);
    int i, j;
    char temp;
    while(len > 0 && (str[len-1] == '\n' || str[len-1] == '\r')) len--;
    
    for (i = 0, j = len - 1; i < j; i++, j--) {
        temp = str[i];
        str[i] = str[j];
        str[j] = temp;
    }
}

int main() 
{
    const char *server_ip = "103.101.163.226"; 
    const char *user_cmd = "USER user_20225246\r\n"; 
    const char *pass_cmd = "PASS 524612\r\n";  
    
    char buffer[1024];
    char cmd[512];

    int control_sock = create_connection(server_ip, 21);
    if (control_sock < 0) {
        printf("Khong the ket noi toi server!\n");
        return -1;
    }

    memset(buffer, 0, sizeof(buffer));
    recv(control_sock, buffer, sizeof(buffer) - 1, 0);
    printf("Server: %s", buffer);

    send(control_sock, user_cmd, strlen(user_cmd), 0);
    memset(buffer, 0, sizeof(buffer));
    recv(control_sock, buffer, sizeof(buffer) - 1, 0);
    printf("Server: %s", buffer);

    send(control_sock, pass_cmd, strlen(pass_cmd), 0);
    memset(buffer, 0, sizeof(buffer));
    recv(control_sock, buffer, sizeof(buffer) - 1, 0);
    printf("Server: %s", buffer);

    int data_sock = enter_passive_mode(control_sock, server_ip);
    
    send(control_sock, "NLST\r\n", 6, 0);
    memset(buffer, 0, sizeof(buffer));
    recv(control_sock, buffer, sizeof(buffer) - 1, 0); 
    
    char question_filename[256] = {0};
    char list_buffer[4096];
    memset(list_buffer, 0, sizeof(list_buffer));
    
    int bytes_read;
    while ((bytes_read = recv(data_sock, list_buffer, sizeof(list_buffer) - 1, 0)) > 0) {
        list_buffer[bytes_read] = '\0';
        char *match = strstr(list_buffer, "question_");
        if (match != NULL) {
            int i = 0;
            while (match[i] != '\r' && match[i] != '\n' && match[i] != '\0') {
                question_filename[i] = match[i];
                i++;
            }
            question_filename[i] = '\0';
        }
    }
    close(data_sock); 
    memset(buffer, 0, sizeof(buffer));
    recv(control_sock, buffer, sizeof(buffer) - 1, 0); 

    data_sock = enter_passive_mode(control_sock, server_ip);
    
    sprintf(cmd, "RETR %s\r\n", question_filename);
    send(control_sock, cmd, strlen(cmd), 0);
    
    memset(buffer, 0, sizeof(buffer));
    recv(control_sock, buffer, sizeof(buffer) - 1, 0); 
    
    char content[1024];
    memset(content, 0, sizeof(content));
    int total_bytes = 0;
    while ((bytes_read = recv(data_sock, content + total_bytes, sizeof(content) - 1 - total_bytes, 0)) > 0) {
        total_bytes += bytes_read;
    }
    content[total_bytes] = '\0';
    close(data_sock);
    
    memset(buffer, 0, sizeof(buffer));
    recv(control_sock, buffer, sizeof(buffer) - 1, 0); 
    printf("Noi dung goc (%d bytes): %s\n", total_bytes, content);

    reverse_string(content);
    printf("Noi dung sau khi dao nguoc: %s\n", content);
    
    char answer_filename[256];
    sprintf(answer_filename, "answer_%s", question_filename + 9);

    FILE *f = fopen(answer_filename, "w");
    if (f != NULL) {
        fprintf(f, "%s", content);
        fclose(f);
        printf("Da luu thanh file: %s\n", answer_filename);
    }

    data_sock = enter_passive_mode(control_sock, server_ip);
    
    sprintf(cmd, "STOR %s\r\n", answer_filename);
    send(control_sock, cmd, strlen(cmd), 0);
    
    memset(buffer, 0, sizeof(buffer));
    recv(control_sock, buffer, sizeof(buffer) - 1, 0); 
    
    f = fopen(answer_filename, "r");
    if (f != NULL) {
        char file_buffer[1024];
        int read_len = fread(file_buffer, 1, sizeof(file_buffer), f);
        send(data_sock, file_buffer, read_len, 0);
        fclose(f);
    }
    close(data_sock); 
    
    memset(buffer, 0, sizeof(buffer));
    recv(control_sock, buffer, sizeof(buffer) - 1, 0); 
    printf("Server: %s", buffer);

    send(control_sock, "QUIT\r\n", 6, 0);
    memset(buffer, 0, sizeof(buffer));
    recv(control_sock, buffer, sizeof(buffer) - 1, 0);
    close(control_sock);
    
    return 0;
}