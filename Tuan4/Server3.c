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
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9090);
    int res = bind(listener, (struct sockaddr *)&addr, sizeof(addr));
    if(res<0)
    {
        perror("bind() failed"); exit(1);
    }

    listen(listener, 5);

    int client = accept(listener, NULL, NULL);
    
    char msg[] = "Nhap du lieu de!\n";
    send(client, msg, strlen(msg), 0);

    struct sinhvien sv;
    while(1)
    {
        int n = recv(client, &sv, sizeof(sv), 0);
        if(n<=0) break;

        printf("Mssv: %d\nHoten: %s\nTuoi: %d\n", sv.mssv, sv.hoten, sv.tuoi);
    }

    close(client);
    close(listener);

    return 0;
}