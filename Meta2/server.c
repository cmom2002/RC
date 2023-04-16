#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/unistd.h>
#include <stdbool.h>
#include <sys/wait.h>

#define BUF_SIZE 1024
#define MAX_TOPICS 10

struct node_client {
    char username[BUF_SIZE];
    char password[BUF_SIZE];
    char type[BUF_SIZE];
    struct node_client *next;
};

struct node_client_topics{
    char username[BUF_SIZE];
    char topics[MAX_TOPICS][BUF_SIZE];
}

struct node_news{
    char title[BUF_SIZE];
    char text[BUF_SIZE];
    char author[BUF_SIZE];
    struct node_news *next;
};

struct node_topics{
    char topic[BUF_SIZE];
    struct node_topics *next;
    struct node_news *news;
};


struct sockaddr_in server_sock, client_sock;
socklen_t slen = sizeof(client_sock);
int sock;

void start_connection(int client, struct node_client *head);

bool verifica_login(char *username, char *password, struct node_client *head);

void read_file_topics(struct node_topics **head, char *file_name);

void add_node_topics(struct node_topics **head, char *topic, char *title, char *text, char *author);

void add_node_news(struct node_news **news, char *topic, char *title, char *text, char *author);

struct node_topics* create_node_topics(char *topic, char *title, char *text, char *author);

struct node_news* create_node_news(char *topic, char *title, char *text, char *author);

void list(struct node_topics *head);

struct node_topics *search_topic(struct node_topics **head, char *topic);

void add_node_client(struct node_client **head, char *user, char *pass, char *type);

void delete(struct node_client **head, char *user);

void list(struct node_client *head);

bool verify_user(struct node_client *head, char *user);

bool verify_admin_user(struct node_client *head, char *user);

bool verify_admin_pass(struct node_client *head, char *pass, char *user);

bool verify_type(char *type);

void read_file_users(struct node_client **head, char *file_name);

void write_file(struct node_client *head, char *file_name);

void send_to_client(char *buf);

void erro(char *s);

int main(int argc, char *argv[]) {
    if (argc != 4)
        erro("new_server {PORTO_NOTICIAS} {PORTO_CONFIG} {ficheiro configuração}");

    struct node_client *head = NULL;
    read_file_users(&head, "Utilizadores.txt");
    struct node_topics *news = NULL;
    read_file_topics(&news, "Topicos.txt");

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

//------------------- Conexão TCP -------------------------

void start_connection(int client, struct node_client *head){
    //list(head);
    char username[BUF_SIZE], password[BUF_SIZE];
    while(1){
        char username[BUF_SIZE], password[BUF_SIZE];
        write(client, "===============\n= Login =\n===============\nUsername: ", strlen("===============\n= Login =\n===============\nUsername: "));
        read(client, username, BUF_SIZE);
        write(client, "Password: ", strlen("Password: "));
        read(client, password, BUF_SIZE);
        if (verifica_login(username, password, head)){
            write(client, "Welcome!!\n", strlen("Welcome!!\n"));
            break;
        }
        else{
            write(client, "Wrong Login\n", strlen("Wrong Login\n"));
            memset(username, 0, BUF_SIZE);
            memset(password, 0, BUF_SIZE);
        }
    }
    
}

bool verifica_login(char *username, char *password, struct node_client *head){
    struct node_client *aux = head;
    while (aux != NULL) {
        if(strcmp(aux->username, username) == 0 && strcmp(aux->password, password) == 0){
            return true;
        }
        aux = aux->next;
    }
    return false;
}

//------------------- Criar Lista Ligada dos Topicos de Utilizadores --------------------------

void read_file_topics(struct node_topics **head, char *file_name) {
    FILE *file;
    char line[BUF_SIZE], data[MAX_TOPICS][BUF_SIZE];

    if ((file = fopen(file_name, "r")) == NULL) {
        printf("Failed to open.\n");
        exit(1);
    }

    while (fgets(line, sizeof(line), file) != NULL) {
        int pos = 0;
        char *token = strtok(line, "|\r\n");
        char username[BUF_SIZE];
        strcpy(username, token);
        while (token != NULL) {
            token = strtok(NULL, ";\r\n");
            strcpy(data[pos], token);
            pos++;
        }
        add_node_topics(head, data[0], data[1], data[2], data[3]);
    }

    if (fclose(file) != 0) {
        printf("Failed to close file.\n");
        exit(1);
    }
}
//------------------- Criar Lista Ligada de Tópicos --------------------------

void read_file_topics(struct node_topics **head, char *file_name) {
    FILE *file;
    char line[BUF_SIZE], data[4][BUF_SIZE];

    if ((file = fopen(file_name, "r")) == NULL) {
        printf("Failed to open.\n");
        exit(1);
    }

    while (fgets(line, sizeof(line), file) != NULL) {
        int pos = 0;
        char *token = strtok(line, "|\r\n");
        while (token != NULL) {
            strcpy(data[pos], token);
            token = strtok(NULL, "|\r\n");
            pos++;
        }
        add_node_topics(head, data[0], data[1], data[2], data[3]);
    }

    if (fclose(file) != 0) {
        printf("Failed to close file.\n");
        exit(1);
    }
}

struct node_topics *create_node_topics(char *topic, char *title, char *text, char *author) {
    struct node_topics* new_node = (struct node_topics*)malloc(sizeof(struct node_topics));
    strcpy(new_node->topic, topic);

    new_node->next = NULL;
    new_node->news = NULL;
    add_node_news(&new_node->news, topic, title, text, author);

    return new_node;
}

void add_node_topics(struct node_topics **head, char *topic, char *title, char *text, char *author){
    struct node_topics *topic_node = search_topic(head, topic);
    if (topic_node != NULL) {
        add_node_news(&topic_node->news, topic, title, text, author);
    } else {
        topic_node = create_node_topics(topic, title, text, author);
        if (*head == NULL) {
            *head = topic_node;
        } else {
            struct node_topics *current = *head;
            while (current->next != NULL) {
                current = current->next;
            }
            current->next = topic_node;
        }
    }
}

struct node_news* create_node_news(char *topic, char *title, char *text, char *author) {

    struct node_news* new_node = (struct node_news*)malloc(sizeof(struct node_news));
    strcpy(new_node->title, title);
    strcpy(new_node->text, text);
    strcpy(new_node->author, author);
    new_node->next = NULL;
    return new_node;
}

void add_node_news(struct node_news **news, char *topic, char *title, char *text, char *author){

    struct node_news *aux = create_node_news(topic, title, text, author);
    if (*news == NULL) {
        *news = aux;
    } else {
        struct node_news *current = *news;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = aux;
    }
}

void list(struct node_topics *head) {

    struct node_topics *aux_topic = head;
    while (aux_topic != NULL) {
        printf("[%s]\n", aux_topic->topic);
        struct node_news *aux_news = aux_topic->news;
        while (aux_news != NULL) {
            printf("[%s] [%s] [%s]\n", aux_news->title, aux_news->text, aux_news->author);
            aux_news = aux_news->next;
        }
        aux_topic = aux_topic->next;
    }
}

struct node_topics *search_topic(struct node_topics **head, char *topic) {

    struct node_topics *aux = *head;
    while (aux != NULL) {
        if (strcmp(aux->topic, topic) == 0) {
            return aux;
        }
        aux = aux->next;
    }
    return NULL;
}


//------------------- Criar Lista Ligada de Clientes -------------------------

struct node_client* create_node(char *user, char *pass, char *type) {
    struct node_client* new_node = (struct node_client*)malloc(sizeof(struct node_client));
    strcpy(new_node->username, user);
    strcpy(new_node->password, pass);
    strcpy(new_node->type, type);
    new_node->next = NULL;
    return new_node;
}

void add_node_client(struct node_client **head, char *user, char *pass, char *type){
    struct node_client *aux = create_node(user, pass, type);
    if (*head == NULL) {
        *head = aux;
    } else {
        struct node_client *current = *head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = aux;
    }
}

void delete(struct node_client **head, char *user){
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
    struct node_client* current = *head;
    while (current->next != NULL) {
        if (strcmp(current->next->username, user) == 0) {
            if (strcmp(current->next->type, "administrador") == 0){
                send_to_client("User is an admnistrator! You can't delete it.\n");
            }
            else{
              struct node_client* temp = current->next;
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

void list(struct node_client *head){
    struct node_client *aux = head;
    while (aux != NULL) {
        char buffer[BUF_SIZE];
        strcpy(buffer, aux->username);
        send_to_client(strcat(buffer, "\n"));
        aux = aux->next;
    }
}

//------------------- Funções de Verificação para os Admins -------------------------

bool verify_admin_user(struct node_client *head, char *user){
    struct node_client *aux = (struct node_client*)malloc(sizeof(struct node_client));
    aux = head;
    while (aux != NULL){
        if(strcmp(aux->username, user) == 0 && strcmp(aux->type, "administrador") == 0)
                return true;
        aux = aux->next;
    }
    return false;
    
}

bool verify_admin_pass(struct node_client *head, char *pass, char *username){
    struct node_client *aux = (struct node_client*)malloc(sizeof(struct node_client));
    aux = head;
    while (aux != NULL){
        if(strcmp(aux->password, pass) == 0 && strcmp(aux->username, username) == 0)
                return true;
        aux = aux->next;
    }
    return false;
    
}

bool verify_user(struct node_client *head, char *user){
    struct node_client *aux = (struct node_client*)malloc(sizeof(struct node_client));
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

void write_file(struct node_client *head, char *file_name){
    FILE *file;
    if ((file = fopen(file_name, "w")) == NULL) {
        printf("Failed to open.\n");
        exit(1);
    }
    struct node_client *aux = (struct node_client*)malloc(sizeof(struct node_client));
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

void read_file_users(struct node_client **head, char *file_name) {
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

void send_to_client(char *buf) {
    if ((sendto(sock, buf, strlen(buf), 0, (struct sockaddr *) &client_sock, slen) == -1))
        erro("Failed to send message to client.");
}
