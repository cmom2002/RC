#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>

#define BUF_SIZE 1024

void erro(char *msg);

void login(int server);



int main(int argc, char *argv[]) {
    if (argc != 3)
        erro("news_client {endereço do servidor} {PORTO_NOTICIAS}");

    char endServer[100];
    struct sockaddr_in addr;
    struct hostent *hostPtr;
    int fd;

    strcpy(endServer, argv[1]);
    if ((hostPtr = gethostbyname(endServer)) == 0)
        erro("Não consegui obter endereço.");

    bzero((void *) &addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = ((struct in_addr *) (hostPtr->h_addr))->s_addr;
    addr.sin_port = htons((int) atoi(argv[2]));

    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        erro("Socket.");
    if (connect(fd, (struct sockaddr *) &addr, sizeof(addr)) < 0)
        erro("Connect.");

    login(fd);
    

    return 0;
}

void login(int server){
    char buffer_login[BUF_SIZE];
    while(1){
        read(server, buffer_login, BUF_SIZE);
        printf("%s", buffer_login);
        memset(buffer_login, 0, BUF_SIZE); // limpa o buffer
        scanf("%s", buffer_login);
        write(server, buffer_login, BUF_SIZE);
        memset(buffer_login, 0, BUF_SIZE); // limpa o buffer
        read(server, buffer_login, BUF_SIZE);
        printf("%s", buffer_login);
        memset(buffer_login, 0, BUF_SIZE); // limpa o buffer
        scanf("%s", buffer_login);
        write(server, buffer_login, BUF_SIZE );
        memset(buffer_login, 0, BUF_SIZE); // limpa o buffer
        read(server, buffer_login, BUF_SIZE); //saber se o cliente existe
        printf("%s", buffer_login);
        memset(buffer_login, 0, BUF_SIZE); // limpa o buffer

        if(strcmp(buffer_login, "Welcome!!\n") == 0){
            break;
        }        
    }
}

void erro(char *msg) {
    printf("Erro: %s\n", msg);
    exit(-1);
}
