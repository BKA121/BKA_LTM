#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

struct svInfo
{
    char mssv[10];
    char hoten[30];
    char ngaysinh[12];
    char cpa[7];
};

int main(int argc, char *argv[])
{
    if(argc<3)
    {
        printf("Nhap thieu tham so"); return 1;
    }

    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(argv[1]);
    addr.sin_port = htons(atoi(argv[2]));

    int res = connect(client, (struct sockaddr *)&addr, sizeof(addr));
    if(res<0)
    {
        perror("Connect() failed"); exit(1);
    }

    char buf[64];
    int n = recv(client, buf, sizeof(buf), 0);
    buf[n] = 0;
    printf("Msg from server: %s\n", buf);

    struct svInfo sv;
    while(1)
    {
        printf("nhap mssv: ");
        fgets(sv.mssv, sizeof(sv.mssv), stdin); 
        sv.mssv[strcspn(sv.mssv, "\n")] = 0;

        printf("nhap hoten: ");
        fgets(sv.hoten, sizeof(sv.hoten), stdin); 
        sv.hoten[strcspn(sv.hoten, "\n")] = 0;

        printf("nhap ngay sinh: ");
        fgets(sv.ngaysinh, sizeof(sv.ngaysinh), stdin); 
        sv.ngaysinh[strcspn(sv.ngaysinh, "\n")] = 0;

        printf("nhap cpa: ");
        fgets(sv.cpa, sizeof(sv.cpa), stdin); 
        sv.cpa[strcspn(sv.cpa, "\n")] = 0;

        printf("\n");

        send(client, &sv, sizeof(sv), 0);
    }
    close(client);
}