#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

int main()
{
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    // Tạo cấu trúc địa chỉ server
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY); // Chấp nhận bất kì địa chỉ nào
    addr.sin_port = htons(9000); // Đổi 9000 từ little -> big để phù hợp với kiểu mạng 

    int opt = 1; 
    // Thiết lập SO_REUSEADDR để giải phóng cổng ngay khi server tắt 
    if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) 
    { 
        perror("setsockopt failed"); 
        exit(EXIT_FAILURE); 
    }

    int res = bind(listener, (struct sockaddr *)&addr, sizeof(addr));
    if(res<0)
    {
        perror("bind() failed");
        exit(1);
    }

    res = listen(listener, 5); // 5 là tối đa 5 client chờ để accept 
    if(res<0)
    {
        perror("listen() failed");
        exit(1);
    }

    printf("Waiting for client\n");

    // Cấu trúc client 
    struct sockaddr_in client_addr;
    int client_addr_len = sizeof(client_addr);
    int client = accept(listener, (struct sockaddr *)&client_addr, &client_addr_len);
    if(client<0)
    {
        perror("accept() failed");
        exit(1);
    }
    printf("Client connected: %d\n", client);
    printf("IP: %s\n", inet_ntoa(client_addr.sin_addr));
    printf("Port: %d\n", ntohs(client_addr.sin_port));

    char msg[] = "Hello client from server!\n";
    send(client, msg, strlen(msg), 0);

    char buf[256];
    while(1)
    {
        int n = recv(client, buf, sizeof(buf), 0);
        if (n <= 0) {
            printf("Disconnected\n");
            break;
        }

        buf[n] = 0; // Thêm kí tự kết thúc xâu 
        printf("%d bytes received: %s\n", n, buf);
    }

    close(client);
    close(listener);
}