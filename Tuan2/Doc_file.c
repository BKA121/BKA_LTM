#include<stdio.h>
#include <stdlib.h>
#include <memory.h>

int main(int argc, char *argv[])
{
    FILE *f = fopen(argv[1], "rb"); // con trỏ f trỏ tới struct chứa thông tin file 
    char buf[2048];
    int size = 0;
    char * p = NULL;

    while(1)
    {
        int n = fread(buf, 1, sizeof(buf), f); // đọc vào toàn bộ buf, trả ra số byte đọc được
        if(n<=0) break;

        size += n;
        p = realloc(p, size); // nới rộng vùng p thêm size 
        memcpy(p+size-n, buf, n); // ghép vùng buf vào vị trí đầu của vùng mới mở rộng
    }

    if(strstr(p, "Ky thuat May tinh") != NULL) printf("Da thay");
    else printf("Khong thay");
    return 0;
}
