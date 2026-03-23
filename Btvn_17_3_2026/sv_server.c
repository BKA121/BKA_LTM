#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

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

    char *sv_log = argv[2];


    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(atoi(argv[1]));
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

    struct sockaddr_in client_addr;
    int client_len = sizeof(client_addr);
    int client = accept(listener, (struct sockaddr *)&client_addr, &client_len);
    
    char msg[] = "Nhap du lieu de!\n";
    send(client, msg, strlen(msg), 0);

    FILE *log = fopen(sv_log, "a");

    struct svInfo sv;
    while(1)
    {
        int n = recv(client, &sv, sizeof(sv), 0);
        if(n <= 0) break;

        time_t t = time(NULL);
        struct tm *tm_info = localtime(&t);
        char time_str[20];
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);

        char *client_ip = inet_ntoa(client_addr.sin_addr);

        printf("%s %s %s %s %s %s\n", client_ip, time_str, sv.mssv, sv.hoten, sv.ngaysinh, sv.cpa);

        fprintf(log, "%s %s %s %s %s %s\n", 
                client_ip, 
                time_str, 
                sv.mssv, 
                sv.hoten, 
                sv.ngaysinh, 
                sv.cpa);
        
        fflush(log); // Đẩy dữ liệu vào file ngay lập tức
    }

    fclose(log);
    close(client);
    close(listener);
    return 0;
}