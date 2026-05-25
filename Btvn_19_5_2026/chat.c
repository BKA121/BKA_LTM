#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

typedef struct {
    int my_sock;
    int partner_sock;
} PairInfo;

void *chat_thread(void *arg) 
{
    PairInfo *info = (PairInfo *)arg;
    int my_sock = info->my_sock;
    int partner_sock = info->partner_sock;
    free(info); 

    char buf[1024];

    while (1) 
    {
        int ret = recv(my_sock, buf, sizeof(buf) - 1, 0);

        if (ret <= 0) 
        {
            printf("Client %d da ngat ket noi. Ket thuc phien chat cap.\n", my_sock);

            shutdown(partner_sock, SHUT_RDWR);

            close(my_sock);
            break;
        }

        buf[ret] = '\0';
        printf("[Chuyen tiep] %d -> %d: %s", my_sock, partner_sock, buf);

        send(partner_sock, buf, ret, 0);
    }
    return NULL;
}

int main() 
{
    signal(SIGPIPE, SIG_IGN);

    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1) {
        perror("socket() failed");
        return 1;
    }
    
    if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int))) {
        perror("setsockopt() failed");
        return 1;
    }
    
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(8080);
    
    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr))) {
        perror("bind() failed");
        return 1;
    }
    
    if (listen(listener, 5)) {
        perror("listen() failed");
        return 1;
    }

    printf("Chat Server dang lang nghe tren cong 8080...\n");

    int waiting_client = -1;

    while (1) 
    {
        int client = accept(listener, NULL, NULL);
        if (client < 0) continue;

        printf("Co client moi ket noi: %d\n", client);

        if (waiting_client == -1) 
        {
            waiting_client = client;
            char *msg = "Ban da vao hang doi. Dang tim kiem doi tac...\n";
            send(client, msg, strlen(msg), 0);
        } 
        else 
        {
            int client1 = waiting_client;
            int client2 = client;
            
            waiting_client = -1; 

            char *msg = "Da tim thay doi tac! Bat dau tro chuyen.\n";
            send(client1, msg, strlen(msg), 0);
            send(client2, msg, strlen(msg), 0);

            printf("Da ghep cap thanh cong: %d va %d\n", client1, client2);

            PairInfo *info1 = malloc(sizeof(PairInfo));
            info1->my_sock = client1;
            info1->partner_sock = client2;

            PairInfo *info2 = malloc(sizeof(PairInfo));
            info2->my_sock = client2;
            info2->partner_sock = client1;

            pthread_t t1, t2;
  
            pthread_create(&t1, NULL, chat_thread, info1);
            pthread_detach(t1);

            pthread_create(&t2, NULL, chat_thread, info2);
            pthread_detach(t2);
        }
    }

    close(listener);
    return 0;
}