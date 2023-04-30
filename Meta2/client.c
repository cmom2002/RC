#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>

#define BUF_SIZE 1024

void erro(char *msg);

void list_topics(char *buffer);

void login(int server);

int main(int argc, char *argv[]) {
    if (argc != 3)
        erro("new_client {endereço do servidor} {PORTO_NOTICIAS}");

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
    read(server, buffer_login, BUF_SIZE);
    printf("%s", buffer_login);
    memset(buffer_login, 0, BUF_SIZE); 
    while(1){
        
        scanf("%s", buffer_login);
        write(server, buffer_login, BUF_SIZE);
        memset(buffer_login, 0, BUF_SIZE); 
        read(server, buffer_login, BUF_SIZE);
        printf("%s", buffer_login);
        memset(buffer_login, 0, BUF_SIZE); 
        scanf("%s", buffer_login);
        write(server, buffer_login, BUF_SIZE );
        memset(buffer_login, 0, BUF_SIZE); 
        read(server, buffer_login, BUF_SIZE);
        
        if(strcmp(buffer_login, "reader!!\n") == 0){
            printf("Welcome!\n\n");
            write(server, "sou leitor", strlen("sou leitor") );
            memset(buffer_login, 0, BUF_SIZE); 
            read(server, buffer_login, BUF_SIZE);
            if(strcmp(buffer_login, "User doesn't have topics subscribed\n") == 0){
                printf("%s", buffer_login);
            }
            else{
                printf("You are subscribed in this topics\n");
                list_topics(buffer_login);
            }
            
            
            memset(buffer_login, 0, BUF_SIZE); 
            while(1){
                printf("1 - List Topics\n2 - Subscribe Topics\n3 - Sair\n");
                scanf("%s", buffer_login);  
                write(server, buffer_login, BUF_SIZE );
                if(strcmp(buffer_login, "1") == 0){
                    memset(buffer_login, 0, BUF_SIZE); 
                    read(server, buffer_login, BUF_SIZE);
                    list_topics(buffer_login);
                } 
                else if(strcmp(buffer_login, "2") == 0){
                    memset(buffer_login, 0, BUF_SIZE); 
                    read(server, buffer_login, BUF_SIZE);
                    printf("%s", buffer_login);
                    memset(buffer_login, 0, BUF_SIZE); 
                    scanf("%s", buffer_login);
                    write(server, buffer_login, BUF_SIZE);
                    memset(buffer_login, 0, BUF_SIZE); 
                    read(server, buffer_login, BUF_SIZE);
                    printf("%s", buffer_login);
                    memset(buffer_login, 0, BUF_SIZE); 
                }   
                else if(strcmp(buffer_login, "3") == 0){
                    memset(buffer_login, 0, BUF_SIZE); 
                    read(server, buffer_login, BUF_SIZE);
                    printf("%s", buffer_login);
                    exit(0);
                } 
            }       
        }   
        else if(strcmp(buffer_login, "writter!!\n") == 0){
            write(server, "sou jornalista", strlen("sou jornalista") );
            printf("Welcome!\n1 - Create Topic\n2 - Send News\n");
            memset(buffer_login, 0, BUF_SIZE); 
            break;
        } 
        else{
            printf("%s", buffer_login);
            memset(buffer_login, 0, BUF_SIZE);           
        }
    }
}


void list_topics(char *buffer){
    char *token = strtok(buffer, "|\n\r");
    while(token != NULL){
        printf("\t- %s\n", token);
        token = strtok(NULL, "|\n\r");
    }
}


void erro(char *msg) {
    printf("Erro: %s\n", msg);
    exit(-1);
}
