#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

int main(int argc, char * argv[])
{
    if(argc<2)
    {
        printf("Nhap thieu tham so, nhap lai");
        return 1;
    }

    int port = atoi(argv[1]);
    char *hello_file = argv[2];
    char *recv_file = argv[3];

    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);
    int res = bind(listener, (struct sockaddr *)&addr, sizeof(addr));
    if(res<0)
    {
        perror("bind() failed"); exit(1);
    }

    int opt = 1; 
    // Thiết lập SO_REUSEADDR để giải phóng cổng ngay khi server tắt 
    if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) { 
        perror("setsockopt failed"); 
        exit(EXIT_FAILURE); 
    }

    printf("Dang cho ket noi...\n");
    listen(listener, 5);

    struct sockaddr_in client_addr;
    int client_len = sizeof(client_addr);
    int client = accept(listener, (struct sockaddr *)&client_addr, &client_len);

    FILE *f_hello = fopen(hello_file, "r");
    char hello_buf[256];
    while (fgets(hello_buf, sizeof(hello_buf), f_hello) != NULL) {
        send(client, hello_buf, strlen(hello_buf), 0);
    }
    fclose(f_hello);

    FILE *f_output = fopen(recv_file, "wb"); 
    char recv_buf[1024];
    int n;

    printf("Msg from client: \n");
    while ((n = recv(client, recv_buf, sizeof(recv_buf-1), 0)) > 0) {
        fwrite(recv_buf, 1, n, f_output);
        recv_buf[n] = 0;
        printf("%s", recv_buf);
    }

    fclose(f_output);
    close(client);
    close(listener);
    return 0;
}