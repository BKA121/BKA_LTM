#include<stdio.h>
#include<stdlib.h>

int main(int argc, char *argv[])
{
    if(argc<5)
    {
        printf("Thieu tham so");
        return 1;
    }

    unsigned char header[20]; // mảng chứa mỗi phần tử 1 byte
    for(int i=0; i<20; i++)
    {
        // strtol đổi string sang số ở hệ 16, ép kiểu unsigned char để giữ 8 bit 
        header[i] = (unsigned char) strtol(argv[i+1], NULL, 16); 
    }

    int version = (header[0] >> 4) & 0x0F;
    int ihl = header[0] & 0x0F;
    int total_length = (header[2] << 8) | header[3];

    printf("version: %d, ihl: %d, total length: %d\n", version, ihl, total_length);

    char src[16], dest[16];

    sprintf(src, "%d.%d.%d.%d", header[12], header[13], header[14], header[15]);
    sprintf(dest, "%d.%d.%d.%d", header[16], header[17], header[18], header[19]);

    printf("src: %s\ndest: %s", src, dest);
    return 0;
}