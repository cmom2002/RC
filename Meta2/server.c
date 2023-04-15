#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/unistd.h>
#include <stdbool.h>
#include <sys/wait.h>

#define BUF_SIZE 1024

struct client {
    char username[BUF_SIZE];
    char password[BUF_SIZE];
    char type[BUF_SIZE];
    struct client *next;
};



struct sockaddr_in server_sock, client_sock;
socklen_t slen = sizeof(client_sock);
int sock;

void start_connection(int client, struct client *head);

bool verifica_login(char *username, char *password, struct client *head);

void add_node_client(struct client **head, char *user, char *pass, char *type);

void delete(struct client **head, char *user);

void list(struct client *head);

bool verify_user(struct client *head, char *user);

bool verify_admin_user(struct client *head, char *user);

bool verify_admin_pass(struct client *head, char *pass, char *user);

bool verify_type(char *type);

void read_file(struct client **head, char *file_name);

void write_file(struct client *head, char *file_name);

void send_to_client(char buf[]);

void erro(char *s);

int main(int argc, char *argv[]) {
    if (argc != 4)
        erro("new_server {PORTO_NOTICIAS} {PORTO_CONFIG} {ficheiro configuração}");

    struct client *head = NULL;
    read_file(&head, "Ficheiro.txt");


    if (fork() == 0){
        int fd, client;
        struct sockaddr_in addr, client_addr;
        int client_addr_size;

        bzero((void *) &addr, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        addr.sin_port = htons((int) atoi(argv[1]));

        if ( (fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
            erro("na funcao socket");
        if ( bind(fd,(struct sockaddr*)&addr,sizeof(addr)) < 0)
            erro("na funcao bind");
        if( listen(fd, 5) < 0)
            erro("na funcao listen");
        client_addr_size = sizeof(client_addr);
        

        //clean finished child processes, avoiding zombies
        //must use WNOHANG or would block whenever a child process was working
        while(waitpid(-1,NULL,WNOHANG)>0);
        while (1) {
            client = accept(fd,(struct sockaddr *)&client_addr,(socklen_t *)&client_addr_size);
            if (client > 0) {
                if (fork() == 0) {                    
                    start_connection(client, head);
                    exit(0);
                }
            close(client);
            }
        }
    }
    else{
        char line[BUF_SIZE];
        int recv_len;

        // Cria um socket para recepção de pacotes UDP
        if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
            erro("Failed to create socket.");

        // Preenchimento da socket address structure
        server_sock.sin_family = AF_INET;
        server_sock.sin_port = htons((int) atoi(argv[2]));
        server_sock.sin_addr.s_addr = ntohl(INADDR_ANY);

        // Associa o socket à informação de endereço
        if (bind(sock, (struct sockaddr *) &server_sock, sizeof(server_sock)) == -1)
            erro("Failed to bind");

        char instructions[4][BUF_SIZE];
        char *token;
        bool login = false, start_login = true, valid_user = false;

        char username[BUF_SIZE];
        //list(head);

        while (1) {
            // Espera recepção de mensagem (a chamada é bloqueante)
            if ((recv_len = (int) recvfrom(sock, line, BUF_SIZE, 0, (struct sockaddr *) &client_sock,
                                            (socklen_t *) &slen)) == -1)
                erro("Fail in recvfrom");
            // Para ignorar o restante conteúdo (anterior do buffer)
            line[recv_len] = '\0';

            if (start_login) {
                send_to_client("===============\n= Login admin =\n===============\nUsername: ");
                start_login = false;
            }

            if (strcmp(line, "X") != 0 && strlen(line) != 1) {
                int pos = 0;

                token = strtok(line, " \n");
                while (token != NULL) {
                    strcpy(instructions[pos++], token);
                    token = strtok(NULL, " \n");
                }

                if (!login) {
                    if(!valid_user && verify_admin_user(head, instructions[0])){
                        valid_user = true;
                        strcpy(username, instructions[0]);
                        send_to_client("Password: ");
                    }
                    else if(!valid_user && !verify_admin_user(head, instructions[0])){
                        send_to_client("Wrong username.\nUsername: ");
                    }
                    else if(valid_user && verify_admin_pass(head, instructions[0], username)){
                        login = true;
                        send_to_client("Welcome!\n");
                    }
                    else if (valid_user || !verify_admin_pass(head, instructions[0], username)){
                        send_to_client("Wrong password.\nPassword: ");
                    }
                    
                } else {
                    if (strcmp(instructions[0], "ADD_USER") == 0) {
                        if (!verify_user(head, instructions[1])) {
                            if(verify_type(instructions[3])){
                                add_node_client(&head, instructions[1], instructions[2], instructions[3]);
                                send_to_client("Client successfully registered.\n");
                            }
                            else{
                                send_to_client("Undefined user type.\n");
                            }
                        }
                        else{
                            send_to_client("Client already registered.\n");
                        }
                    } else if (strcmp(instructions[0], "DEL") == 0) {
                        if(strcmp(username, instructions[1]) == 0){
                            send_to_client("You can't delete yourself!\n");
                        }
                        else{
                            delete(&head, instructions[1]);   
                        }
                    } else if (strcmp(instructions[0], "LIST") == 0) {

                        list(head);
                    } else if (strcmp(instructions[0], "QUIT") == 0) {
                        break;
                    } else if (strcmp(instructions[0], "QUIT_SERVER") == 0) {
                        close(sock);
                        break;
                    } else {
                        send_to_client("Invalid argument.\n");
                    }
                }
            }
        }
        write_file(head, argv[3]);
    }
    return 0;
}

//

void start_connection(int client, struct client *head){
    //list(head);
    while(1){
        char username[BUF_SIZE], password[BUF_SIZE];
        write(client, "===============\n= Login =\n===============\nUsername: ", strlen("===============\n= Login =\n===============\nUsername: "));
        read(client, username, BUF_SIZE);
        write(client, "Password: ", strlen("Password: "));
        read(client, password, BUF_SIZE);
        printf("[%s] - [%s]\n", username, password);
        if (verifica_login(username, password, head)){
            write(client, "Welcome!!\n", strlen("Welcome!!\n"));
            break;
        }
        else{
            write(client, "Wrong Login\n", strlen("Wrong Login\n"));
        }
    }
}

bool verifica_login(char *username, char *password, struct client *head){
    struct client *aux = head;
    while (aux != NULL) {
        if(strcmp(aux->username, username) == 0 && strcmp(aux->password, password) == 0){
            return true;
        }
        aux = aux->next;
    }
    return false;
}

//------------------- Criar Lista Ligada de Clientes -------------------------

struct client* create_node(char *user, char *pass, char *type) {
    struct client* new_node = (struct client*)malloc(sizeof(struct client));
    strcpy(new_node->username, user);
    strcpy(new_node->password, pass);
    strcpy(new_node->type, type);
    new_node->next = NULL;
    return new_node;
}

void add_node_client(struct client **head, char *user, char *pass, char *type){
    struct client *aux = create_node(user, pass, type);
    if (*head == NULL) {
        *head = aux;
    } else {
        struct client *current = *head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = aux;
    }
}

void delete(struct client **head, char *user){
    // se a lista está vazia, não há nada a ser feito
    if (*head == NULL) {
        return;
    }
    
    // se o nó a ser excluído é o primeiro da lista
    if (strcmp((*head)->username, user) == 0) {
        if (strcmp((*head)->type, "administrador") == 0){
            send_to_client("User is an admnistrator! You can't delete it.\n");
        }
        else{
            *head = (*head)->next;
            send_to_client("User successfully deleted.\n");   
        }
        return;
    }

    // percorre a lista procurando o nó a ser excluído
    struct client* current = *head;
    while (current->next != NULL) {
        if (strcmp(current->next->username, user) == 0) {
            if (strcmp(current->next->type, "administrador") == 0){
                send_to_client("User is an admnistrator! You can't delete it.\n");
            }
            else{
              struct client* temp = current->next;
            current->next = current->next->next;
            free(temp);
            send_to_client("User successfully deleted.\n"); 
            }
            return;
            
        }
        current = current->next;
    }
    send_to_client("User doesn't exists.\n");
}

void list(struct client *head){
    struct client *aux = head;
    while (aux != NULL) {
        char buffer[BUF_SIZE];
        strcpy(buffer, aux->username);
        send_to_client(strcat(buffer, "\n"));
        aux = aux->next;
    }
}

//------------------- Funções de Verificação para os Admins -------------------------

bool verify_admin_user(struct client *head, char *user){
    struct client *aux = (struct client*)malloc(sizeof(struct client));
    aux = head;
    while (aux != NULL){
        if(strcmp(aux->username, user) == 0 && strcmp(aux->type, "administrador") == 0)
                return true;
        aux = aux->next;
    }
    return false;
    
}

bool verify_admin_pass(struct client *head, char *pass, char *username){
    struct client *aux = (struct client*)malloc(sizeof(struct client));
    aux = head;
    while (aux != NULL){
        if(strcmp(aux->password, pass) == 0 && strcmp(aux->username, username) == 0)
                return true;
        aux = aux->next;
    }
    return false;
    
}

bool verify_user(struct client *head, char *user){
    struct client *aux = (struct client*)malloc(sizeof(struct client));
    aux = head;
    while (aux != NULL){
        if(strcmp(aux->username, user) == 0)
                return true;
        aux = aux->next;
    }
    return false;
}

bool verify_type(char *type){
    int count = 0;
    if (strcmp(type, "administrador") == 0)
        count++;
    else if (strcmp(type, "leitor") == 0)
        count++;
    else if (strcmp(type, "jornalista") == 0)
        count++;
    if(count != 0)
        return true;
    return false;
}

//------------------- Funções para Ler/Escrever nos Ficheiros -------------------------

void write_file(struct client *head, char *file_name){
    FILE *file;
    if ((file = fopen(file_name, "w")) == NULL) {
        printf("Failed to open.\n");
        exit(1);
    }
    struct client *aux = (struct client*)malloc(sizeof(struct client));
    aux = head;
    while (aux != NULL){
        char buffer[BUF_SIZE] = "";
        strcat(buffer, aux->username);
        strcat(buffer, ";");
        strcat(buffer, aux->password);
        strcat(buffer, ";");
        strcat(buffer, aux->type);
        strcat(buffer, "\n");
        fprintf(file, "%s", buffer);
        aux = aux->next;
    }

    if (fclose(file) != 0) {
        printf("Failed to close file.\n");
        exit(1);
    }
}

void read_file(struct client **head, char *file_name) {
    FILE *file;
    char line[BUF_SIZE], data[3][BUF_SIZE];
    int n_users, m_count = 0, s_count = 0;

    if ((file = fopen(file_name, "r")) == NULL) {
        printf("Failed to open.\n");
        exit(1);
    }
    
    for (int i = 0; fgets(line, sizeof(line), file) != NULL; i++) {
        int pos = 0;
        char *token = strtok(line, ";\n\r");
        while (token != NULL) {
            strcpy(data[pos], token);
            token = strtok(NULL, ";\n\r");
            pos++;
        }
        add_node_client(head, data[0], data[1], data[2]);
    }

    if (fclose(file) != 0) {
        printf("Failed to close file.\n");
        exit(1);
    }
}


void erro(char *s) {
    perror(s);
    exit(1);
}

void send_to_client(char buf[]) {
    if ((sendto(sock, buf, strlen(buf), 0, (struct sockaddr *) &client_sock, slen) == -1))
        erro("Failed to send message to client.");
}
