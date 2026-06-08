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
#include <time.h>

double process_http_request(const char *request, int *status) 
{
    *status = 0;
    char data[1024] = {0};

    if (strncmp(request, "GET", 3) == 0) 
    {
        char *start = strchr(request, '?'); 
        if (start != NULL) 
        {
            start++; 
            char *end = strstr(start, " HTTP"); 
            if (end != NULL) 
            {
                strncpy(data, start, end - start);
            }
        }
    } 
    else if (strncmp(request, "POST", 4) == 0) 
    {
        char *start = strstr(request, "\r\n\r\n"); 
        if (start != NULL) 
        {
            start += 4; 
            strcpy(data, start); 
        }
    }

    if (strlen(data) == 0) return 0; 

    double a = 0, b = 0;
    char op[10] = {0};

    char *ptr_a = strstr(data, "a=");
    char *ptr_b = strstr(data, "b=");
    char *ptr_op = strstr(data, "op=");

    if (ptr_a && ptr_b && ptr_op) 
    {
        sscanf(ptr_a, "a=%lf", &a);
        sscanf(ptr_b, "b=%lf", &b);
        
        sscanf(ptr_op, "op=%9[^& \r\n]", op); 
    } 
    else 
    {
        return 0; 
    }
    *status = 1; 

    if (strcmp(op, "cong") == 0) return a + b;
    if (strcmp(op, "tru") == 0) return a - b;
    if (strcmp(op, "nhan") == 0) return a * b;
    if (strcmp(op, "chia") == 0) 
    {
        if (b != 0) 
        {
            return a / b;
        } 
        else 
        {
            *status = -1; 
            return 0;
        }
    }

    *status = 0; 
    return 0;
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
    if (bytes_received <= 0) {
        close(client);
        return NULL;
    }

    if (strncmp(buffer, "GET /favicon.ico", 16) == 0) 
    {
        close(client);
        return NULL; 
    }

    printf("Nhan duoc:\n%s\n", buffer);

    char response[4096];
    if (strncmp(buffer, "GET / ", 6) == 0) 
    {
        sprintf(response, 
            "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"
            "<html><body>"
            
            "<h2>Tinh toan (Dung GET)</h2>"
            "<form action='/tinh' method='GET'>"
            "<input name='a'> <input name='b'> "
            "<select name='op'>"
                "<option value='cong'>+</option>"                
                "<option value='tru'>-</option>"
                "<option value='nhan'>*</option>"
                "<option value='chia'>/</option>"
            "</select> "
            "<button type='submit'>Gui GET</button>"
            "</form>"
            
            "<hr>" 
            
            "<h2>Tinh toan (Dung POST)</h2>"
            "<form action='/tinh' method='POST'>"
            "<input name='a'> <input name='b'> "
            "<select name='op'>"
                "<option value='cong'>+</option>"                
                "<option value='tru'>-</option>"
                "<option value='nhan'>*</option>"
                "<option value='chia'>/</option>"
            "</select> "
            "<button type='submit'>Gui POST</button>"
            "</form>"
            
            "</body></html>"
        );
    }
    else if (strncmp(buffer, "GET /tinh?", 10) == 0 || strncmp(buffer, "POST /tinh", 10) == 0)
    {
        int status;
        double ket_qua = process_http_request(buffer, &status);

        if (status == 1) 
        {
            sprintf(response, 
                "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"
                "<html><body><h2>Ket qua: %.2lf</h2>"
                "<a href='/'>Quay lai Form</a></body></html>", 
                ket_qua);
        } 
        else if (status == -1) 
        {
            sprintf(response, 
                "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"
                "<html><body><h2>Loi: Khong the chia cho 0!</h2>"
                "<a href='/'>Quay lai Form</a></body></html>");
        } 
        else 
        {
            sprintf(response, 
                "HTTP/1.1 400 Bad Request\r\nContent-Type: text/html\r\n\r\n"
                "<html><body><h2>Loi: Du lieu gui len khong hop le!</h2>"
                "<a href='/'>Quay lai Form</a></body></html>");
        }
    }
    send(client, response, strlen(response), 0);
    close(client);
}