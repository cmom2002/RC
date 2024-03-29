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

void login(int server, char *id_client);

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
    login(fd, argv[1]);
    close(fd);
    return 0;
}

void login(int server, char *id_client){
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
        
        char type[BUF_SIZE];
        strcpy(type, buffer_login);
        
        if(strcmp(type, "writer") == 0 || strcmp(type, "reader") == 0){
            memset(buffer_login, 0, BUF_SIZE);
            printf("Welcome!\n\n");
            write(server, id_client, strlen(id_client));
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
                
                if(strcmp(type, "writer") == 0){
                   printf("1 - List Topics\n2 - Subscribe Topics\n3 - Create Topic\n4 - Send News\n5 - Sair\n"); 
                }
                else{
                    printf("1 - List Topics\n2 - Subscribe Topics\n3 - Sair\n");
                }
                
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
                else if(strcmp(buffer_login, "3") == 0 && strcmp(type, "writer") != 0){
                    memset(buffer_login, 0, BUF_SIZE); 
                    read(server, buffer_login, BUF_SIZE);
                    printf("%s", buffer_login);
                    exit(0);
                } 
                else if(strcmp(buffer_login, "3") == 0 && strcmp(type, "writer") == 0){
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
                else if(strcmp(buffer_login, "4") == 0 && strcmp(type, "writer") == 0){
                    //send_news
                }
                else if(strcmp(buffer_login, "5") == 0 && strcmp(type, "writer") == 0){
                    memset(buffer_login, 0, BUF_SIZE); 
                    read(server, buffer_login, BUF_SIZE);
                    printf("%s", buffer_login);
                    exit(0);
                }
                else{
                    memset(buffer_login, 0, BUF_SIZE); 
                    read(server, buffer_login, BUF_SIZE);
                    printf("%s", buffer_login);
                    memset(buffer_login, 0, BUF_SIZE);
                }
            }       
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
