#include<stdio.h>
#include<string.h>

int main()
{
    char str[20];
    char cmd[10]; 
    float x, y;

    while(1)
    {
        printf("Nhap lenh: ");
        fgets(str, sizeof(str), stdin); // đọc từ bàn phím vào mảng str (lưu ý c không có kiểu string)

        int n = sscanf(str, "%s %f %f", cmd, &x, &y); //đọc dữ liệu có định dạng vào các biến 
        if(n == 3)
        {
            if(strcmp(cmd, "ADD") == 0 || strcmp(cmd, "SUB") == 0 || 
               strcmp(cmd, "MUL") == 0 || strcmp(cmd, "DIV") == 0)
            {
                printf("Lenh dung, cmd: %s, x: %.2f, y: %.2f", cmd, x, y);
            }
            else printf("Sai cu phap\n");
        }
        else printf("Thua thieu tham so\n");
    }
    return 0;
}