#include <stdio.h>      // Cho printf
#include <stdlib.h>     // Cho EXIT_FAILURE, EXIT_SUCCESS
#include <string.h>     // Cho memcpy
#include <sys/types.h>  // Cho các định nghĩa kiểu dữ liệu hệ thống
#include <sys/socket.h> // Cho cấu trúc socket, AF_INET
#include <netdb.h>      // Cho getaddrinfo, struct addrinfo, freeaddrinfo
#include <arpa/inet.h>  // Cho inet_ntoa, struct sockaddr_in


int main(int argc, char *argv[])
{
    struct addrinfo *res, *p;
    int ret = getaddrinfo(argv[1], "http", NULL, &res);

    if(ret!=0)
    {
        printf("Failed to get ip");
        return 1;
    }

    p=res;
    while(p!=NULL)
    {
        if(p->ai_family == AF_INET)
        {
            printf("Ipv4\n");
            struct sockaddr_in addr;
            memcpy(&addr, p->ai_addr, p->ai_addrlen);
            printf("Ip: %s\n", inet_ntoa(addr.sin_addr));
        }
        else if(p->ai_family == AF_INET6)
        {
            printf("Ipv6\n");
            char buf[64];
            struct sockaddr_in6 addr6;
            memcpy(&addr6, p->ai_addr, p->ai_addrlen);
            printf("Ip: %s\n", inet_ntop(AF_INET6, &addr6.sin6_addr, buf, sizeof(addr6)));
        }
        p = p->ai_next;
    }
    freeaddrinfo(res);
    return 0;
}