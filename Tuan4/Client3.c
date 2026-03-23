#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

struct sinhvien
{
    int mssv;
    char hoten[64];
    int tuoi;
};

int main()
{
    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(9090);
    int res = connect(client, (struct sockaddr *)&addr, sizeof(addr));
    if(res<0)
    {
        perror("connect() failed"); exit(1);
    }

    char buf[30];
    int n = recv(client, buf, sizeof(buf), 0);
    buf[n] = 0;
    printf("%s\n", buf);

    struct sinhvien sv;
    char temp[48];
    while(1)
    {
        printf("nhap mssv: ");
        fgets(temp, sizeof(temp), stdin); 
        sv.mssv = atoi(temp);

        printf("nhap hoten: ");
        fgets(sv.hoten, sizeof(sv.hoten), stdin); 
        sv.hoten[strcspn(sv.hoten, "\n")] = 0;

        printf("nhap tuoi: ");
        fgets(temp, sizeof(temp), stdin); 
        sv.tuoi = atoi(temp);

        if(sv.tuoi == 0) break;

        send(client, &sv, sizeof(sv), 0);
    }
    close(client);
    return 0;
}