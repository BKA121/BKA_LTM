#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

int main()
{
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in addr; // Khối này lưu địa chỉ đích 
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9000);
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

    listen(listener, 5);

    int client = accept(listener, NULL, NULL);

    char buf[256];
    while(1)
    {
        int n = recv(client, buf, sizeof(buf), 0);
        if(n<=0)
        {
            break;
        }
        buf[n] = 0;
        printf("Msg from client: %s\n", buf);
    }

    close(client);
    close(listener);

    return 0;
}