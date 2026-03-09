#include<stdio.h>
#include <string.h>
#include <stdlib.h>

int main()
{
    char pasv[] = "227 Entering Passive Mode (213,229,112,130,216,4)";

    char * p = strtok(pasv, "(),"); // tách chuỗi 
    p = strtok(NULL, "(),");
    int i1 = atoi(p); // chuyển từ chuỗi sang số hệ 10 
    p = strtok(NULL, "(),");
    int i2 = atoi(p);
    p = strtok(NULL, "(),");
    int i3 = atoi(p);
    p = strtok(NULL, "(),");
    int i4 = atoi(p);

    p = strtok(NULL, "(),");
    int p1 = atoi(p);
    p = strtok(NULL, "(),");
    int p2 = atoi(p);
    printf("ip: %d.%d.%d.%d\n", i1, i2, i3, i4);
    printf("port: %d", p1*256+p2);
}