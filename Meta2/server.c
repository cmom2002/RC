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
#define FILE_TOPIC "Topicos.txt"
#define FILE_USER_TOPICS "Users_Topics.txt"

struct node_client {
    char username[BUF_SIZE];
    char password[BUF_SIZE];
    char type[BUF_SIZE];
    struct node_client *next;
};

struct node_client_topics{
    char username[BUF_SIZE];
    char topics[MAX_TOPICS][BUF_SIZE];
    int num_topics;
    struct node_client_topics* next;
};

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

char data[MAX_TOPICS][BUF_SIZE];

struct sockaddr_in server_sock, client_sock;
socklen_t slen = sizeof(client_sock);

int sock;

void send_to_client(char *buf);

void erro(char *s);

//---------------------------------------- Conexão TCP ----------------------------------------
void start_connection(int client, struct node_client *head, struct node_client_topics **topics, struct node_topics *general);

bool return_type_user(char *username, struct node_client *head);

bool verify_login(char *username, char *password, struct node_client *head);

bool find_topic(struct node_topics *head, char *topic);

//---------------------- Criar Lista Ligada dos Topicos de Utilizadores -----------------------
void read_file_ct(struct node_client_topics **head, char *file_name);

void write_file_ct(struct node_client_topics *head, char *file_name);

bool find_user_ct(struct node_client_topics *head, char *username);

struct node_client_topics* create_node_ct(char *username, int pos);

void add_node_ct(struct node_client_topics **head, char *username, int pos);

char *list_topics_user(struct node_client_topics *head, char *username, char *ya);

//------------------------------- Criar Lista Ligada de Tópicos -------------------------------
void read_file_topics(struct node_topics **head, char *file_name);

void write_file_topics(struct node_topics *head, char *file_name);

struct node_topics *create_node_topics(char *topic, char *title, char *text, char *author);

void add_node_topics(struct node_topics **head, char *topic, char *title, char *text, char *author);

struct node_news* create_node_news(char *topic, char *title, char *text, char *author);

void add_node_news(struct node_news **news, char *topic, char *title, char *text, char *author);

char *list_all_topics(struct node_topics *head, char *buffer);

struct node_topics *search_topic(struct node_topics **head, char *topic);

//------------------------------- Criar Lista Ligada de Clientes ------------------------------
void read_file_users(struct node_client **head, char *file_name);

void write_file_users(struct node_client *head, char *file_name);

struct node_client* create_node_client(char *user, char *pass, char *type);

void add_node_client(struct node_client **head, char *user, char *pass, char *type);

void delete_client(struct node_client **head, char *user);

void list_clients(struct node_client *head);

//------------------------------ Funções de Verificação de Admins -----------------------------
bool verify_admin_user(struct node_client *head, char *user);

bool verify_admin_pass(struct node_client *head, char *pass, char *username);

bool verify_user(struct node_client *head, char *user);

bool verify_type(char *type);


int main(int argc, char *argv[]) {
    if (argc != 4)
        erro("new_server {PORTO_NOTICIAS} {PORTO_CONFIG} {ficheiro configuração}");

    struct node_client *head = NULL;
    read_file_users(&head, argv[3]);
    struct node_topics *news = NULL;
    read_file_topics(&news, FILE_TOPIC);
    struct node_client_topics *topics = NULL;
    read_file_ct(&topics, FILE_USER_TOPICS);

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

        while(waitpid(-1,NULL,WNOHANG)>0);
        while (1) {
            client = accept(fd,(struct sockaddr *)&client_addr,(socklen_t *)&client_addr_size);
            if (client > 0) {
                if (fork() == 0) {                    
                    start_connection(client, head, &topics, news);
                    exit(0);
                }
            close(client);
            }
        } 
    }
    else{
        char line[BUF_SIZE];
        int recv_len;

        if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
            erro("Failed to create socket.");

        server_sock.sin_family = AF_INET;
        server_sock.sin_port = htons((int) atoi(argv[2]));
        server_sock.sin_addr.s_addr = ntohl(INADDR_ANY);

        if (bind(sock, (struct sockaddr *) &server_sock, sizeof(server_sock)) == -1)
            erro("Failed to bind");

        char instructions[4][BUF_SIZE];
        char *token;
        bool login = false, start_login = true, valid_user = false;

        char username[BUF_SIZE];

        while (1) {
            if ((recv_len = (int) recvfrom(sock, line, BUF_SIZE, 0, (struct sockaddr *) &client_sock,(socklen_t *) &slen)) == -1)
                erro("Fail in recvfrom");

            line[recv_len] = '\0';

            if (start_login) {
                send_to_client("===============\n= Login admin =\n===============\nUsername: ");
                start_login = false;
            }

            if (strcmp(line, "X") != 0 && strlen(line) != 1){
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
                            delete_client(&head, instructions[1]);   
                        }
                    } else if (strcmp(instructions[0], "LIST") == 0) {
                        list_clients(head);
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
        write_file_users(head, argv[3]);
    }
    return 0;
}

//---------------------------------------- Conexão TCP ----------------------------------------
void start_connection(int client, struct node_client *head, struct node_client_topics **topics, struct node_topics *general){
    char username[BUF_SIZE], password[BUF_SIZE];
    write(client, "===============\n= Login =\n===============\nUsername: ", strlen("===============\n= Login =\n===============\nUsername: "));
    while(1){
        char username[BUF_SIZE], password[BUF_SIZE];
        read(client, username, BUF_SIZE);
        write(client, "Password: ", strlen("Password: "));
        read(client, password, BUF_SIZE);
        if (verify_login(username, password, head)){  
            
            bool is_writer = return_type_user(username, head);
            if(is_writer){
               write(client, "writer", strlen("writer")); 
            }
            else{
                write(client, "reader", strlen("reader"));
            }
            
            char buffer[BUF_SIZE];
            read(client, buffer, BUF_SIZE);
            memset(buffer, 0, BUF_SIZE);
            char ya[BUF_SIZE] = "";
            list_topics_user(*topics, username, ya);
            write(client, ya, BUF_SIZE);
            while(1){
                read(client, buffer, BUF_SIZE);
                if (strcmp(buffer, "1") == 0){
                    char ya2[BUF_SIZE] = "";
                    list_all_topics(general, ya2);
                    write(client, ya2, BUF_SIZE);
                }
                else if (strcmp(buffer, "2") == 0){
                    bool exists = true, not_in_list = true;
                    memset(buffer, 0, BUF_SIZE);
                    write(client, "What topic do you want to subscribe?\n", strlen("What topic do you want to subscribe?\n"));
                    read(client, buffer, BUF_SIZE);
                    if(find_topic(general, buffer)){
                        struct node_client_topics *aux = *topics;
                        while (aux != NULL) {
                            if(strcmp(username, aux->username) == 0){  
                                not_in_list = false;
                                for(int i = 0; i < MAX_TOPICS; i++){
                                    if(strcmp(aux->topics[i], buffer) == 0){
                                        exists = false;
                                    }
                                }
                                if(exists){
                                    strcpy(aux->topics[aux->num_topics], buffer);
                                    aux->num_topics++;
                                    write(client, "Topic Subscribed!!\n", strlen("Topic Subscribed!!\n"));
                                    write_file_ct(*topics, FILE_USER_TOPICS);
                                }
                                else{
                                    write(client, "Topic is already subscribed!\n", strlen("Topic is already subscribed!\n"));
                                }
                                break;
                            }
                            aux = aux->next; 
                        }   

                        if(not_in_list){
                            strcpy(data[0], buffer);
                            add_node_ct(topics, username, 1);
                            memset(data, 0, sizeof(data));
                            write(client, "Topic Subscribed!!\n", strlen("Topic Subscribed!!\n"));
                            write_file_ct(*topics, FILE_USER_TOPICS);

                        }
                    }      
                    else{
                        write(client, "Topic Not Found!!\n", strlen("Topic Not Found!!\n"));
                    }             
                }
                else if(is_writer){
                    if(strcmp(buffer, "3") == 0){
                        //create_topic
                    }
                    else if(strcmp(buffer, "4") == 0){
                        //send_topic
                    }
                    else if(strcmp(buffer, "5") == 0){
                        write(client, "Bai Bai\n", strlen("Bai Bai\n"));
                        exit(0);
                    }
                }
                else if (strcmp(buffer, "3") == 0){
                    write(client, "Bai Bai\n", strlen("Bai Bai\n"));
                    exit(0);
                }
                memset(buffer, 0, BUF_SIZE);
            }
        }
        else{
            write(client, "Wrong Login\nUsername: ", strlen("Wrong Login\nUsername: "));
            memset(username, 0, BUF_SIZE);
            memset(password, 0, BUF_SIZE);
        }
    }   
}

bool return_type_user(char *username, struct node_client *head){
    struct node_client *aux = head;
    while (aux != NULL) {
        if(strcmp(aux->username, username) == 0 && strcmp(aux->type, "jornalista") == 0){
            return true;
        }
        aux = aux->next;
    }
    return false;
}

bool verify_login(char *username, char *password, struct node_client *head){
    struct node_client *aux = head;
    while (aux != NULL) {
        if(strcmp(aux->username, username) == 0 && strcmp(aux->password, password) == 0){
            return true;
        }
        aux = aux->next;
    }
    return false;
}

bool find_topic(struct node_topics *head, char *topic){
    struct node_topics *aux = head;

    while (aux != NULL) {
        if(strcasecmp(topic, aux->topic) == 0){            
            return true;
            //memset(buffer, 0, BUF_SIZE);           
        } 
        aux = aux->next; 
    }
    return false;
}

//---------------------- Criar Lista Ligada dos Topicos de Utilizadores -----------------------
void read_file_ct(struct node_client_topics **head, char *file_name){
    FILE *file;
    char line[BUF_SIZE];
    if ((file = fopen(file_name, "r")) == NULL) {
        printf("Failed to open.\n");
        exit(1);
    }
    while (fgets(line, sizeof(line), file) != NULL) {
        char username[BUF_SIZE];
        char *token = strtok(line, "|\n\r");
        bool user = true;
        int pos = 0;
        while(token != NULL){
            if(user){
                strcpy(username, token);
                user = false;
            }
            else {
                strcpy(data[pos], token);
                pos++;
            }
            token = strtok(NULL, ";\n\r");

        }
        add_node_ct(head, username, pos);

        memset(username, 0, BUF_SIZE);
        memset(data, 0, sizeof(data));
        pos = 0;
    }
}

void write_file_ct(struct node_client_topics *head, char *file_name){
    FILE *file;
    if ((file = fopen(file_name, "w")) == NULL) {
        printf("Failed to open.\n");
        exit(1);
    }
    struct node_client_topics *aux = head;
    while (aux != NULL){
        char buffer[BUF_SIZE] = "";
        strcat(buffer, aux->username);
        strcat(buffer, "|");
        int i;
        for(i = 0; i < aux->num_topics; i++){
            strcat(buffer, aux->topics[i]);
            strcat(buffer, ";");
        }
        strcat(buffer, aux->topics[i + 1]);
        strcat(buffer, "\n");
        fprintf(file, "%s", buffer);
        aux = aux->next;
    }

    if (fclose(file) != 0) {
        printf("Failed to close file.\n");
        exit(1);
    }
}

bool find_user_ct(struct node_client_topics *head, char *username){
    struct node_client_topics *aux = head;
    while (aux != NULL) {
        if(strcmp(username, aux->username) == 0){            
            return true;
        } 
        aux = aux->next; 
    }
    return false;
}

struct node_client_topics* create_node_ct(char *username, int pos) {
    struct node_client_topics* new_node = (struct node_client_topics*)malloc(sizeof(struct node_client_topics));
    strcpy(new_node->username, username);
    for(int i = 0; i < pos; i++){
        strcpy(new_node->topics[i], data[i]);
    }
    new_node->num_topics = pos;
    new_node->next = NULL;
    return new_node;
}

void add_node_ct(struct node_client_topics **head, char *username, int pos){
    struct node_client_topics *aux = create_node_ct(username, pos);
    if (*head == NULL) {
        *head = aux;
    } else {
        struct node_client_topics *current = *head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = aux;
    }
}

char *list_topics_user(struct node_client_topics *head, char *username, char *ya){
    struct node_client_topics *aux = head;
    while (aux != NULL) {
        if(strcmp(username, aux->username) == 0){            
            for(int i = 0; i < aux->num_topics; i++){
                strcat(ya, aux->topics[i]);
                strcat(ya, "|");
            } 
            return ya;          
        } 
        aux = aux->next; 
    }
    strcat(ya, "User doesn't have topics subscribed\n");
    return ya;
}

//------------------------------- Criar Lista Ligada de Tópicos -------------------------------
void read_file_topics(struct node_topics **head, char *file_name){
    FILE *file;
    char line[BUF_SIZE], noti[4][BUF_SIZE];

    if ((file = fopen(file_name, "r")) == NULL) {
        printf("Failed to open.\n");
        exit(1);
    }

    while (fgets(line, sizeof(line), file) != NULL) {
        int pos = 0;
        char *token = strtok(line, "|\r\n");
        while (token != NULL) {
            strcpy(noti[pos], token);
            token = strtok(NULL, "|\r\n");
            pos++;
        }
        add_node_topics(head, noti[0], noti[1], noti[2], noti[3]);
    }

    if (fclose(file) != 0) {
        printf("Failed to close file.\n");
        exit(1);
    }
}

void write_file_topics(struct node_topics *head, char *file_name){
    FILE *file;
    char line[BUF_SIZE], noti[4][BUF_SIZE];

    if ((file = fopen(file_name, "r")) == NULL) {
        printf("Failed to open.\n");
        exit(1);
    }

    struct node_topics *aux = head;
    while (aux != NULL){
        struct node_news *new = aux->news; 
        while(new != NULL){
            char buffer[BUF_SIZE] = "";
            strcat(buffer, aux->topic);
            strcat(buffer, "|");
            strcat(buffer, new->title);
            strcat(buffer, "|");
            strcat(buffer, new->text);
            strcat(buffer, "|");
            strcat(buffer, new->author);
            strcat(buffer, "\n");
            fprintf(file, "%s", buffer);
            new = new->next;
        }
        
        aux = aux->next;
    }

    if (fclose(file) != 0) {
        printf("Failed to close file.\n");
        exit(1);
    }
}

struct node_topics *create_node_topics(char *topic, char *title, char *text, char *author){
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

struct node_news* create_node_news(char *topic, char *title, char *text, char *author){
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

char *list_all_topics(struct node_topics *head, char *buffer){
    struct node_topics *aux_topic = head;
    while (aux_topic != NULL) {
        strcat(buffer, aux_topic->topic);
        strcat(buffer, "|");
        aux_topic = aux_topic->next;
    }
    return buffer;
}

struct node_topics *search_topic(struct node_topics **head, char *topic){
    struct node_topics *aux = *head;
    while (aux != NULL) {
        if (strcmp(aux->topic, topic) == 0) {
            return aux;
        }
        aux = aux->next;
    }
    return NULL;
}

//------------------------------- Criar Lista Ligada de Clientes ------------------------------
void read_file_users(struct node_client **head, char *file_name){
    FILE *file;
    char line[BUF_SIZE], info[3][BUF_SIZE];
    int n_users, m_count = 0, s_count = 0;

    if ((file = fopen(file_name, "r")) == NULL) {
        printf("Failed to open.\n");
        exit(1);
    }
    
    for (int i = 0; fgets(line, sizeof(line), file) != NULL; i++) {
        int pos = 0;
        char *token = strtok(line, ";\n\r");
        while (token != NULL) {
            strcpy(info[pos], token);
            token = strtok(NULL, ";\n\r");
            pos++;
        }
        add_node_client(head, info[0], info[1], info[2]);
    }

    if (fclose(file) != 0) {
        printf("Failed to close file.\n");
        exit(1);
    }
}

void write_file_users(struct node_client *head, char *file_name){
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

struct node_client* create_node_client(char *user, char *pass, char *type){
    struct node_client* new_node = (struct node_client*)malloc(sizeof(struct node_client));
    strcpy(new_node->username, user);
    strcpy(new_node->password, pass);
    strcpy(new_node->type, type);
    new_node->next = NULL;
    return new_node;
}

void add_node_client(struct node_client **head, char *user, char *pass, char *type){
    struct node_client *aux = create_node_client(user, pass, type);
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

void delete_client(struct node_client **head, char *user){
    if (*head == NULL) {
        return;
    }
    
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

void list_clients(struct node_client *head){
    struct node_client *aux = head;
    while (aux != NULL) {
        char buffer[BUF_SIZE];
        strcpy(buffer, aux->username);
        send_to_client(strcat(buffer, "\n"));
        aux = aux->next;
    }
}

//------------------------------ Funções de Verificação de Admins -----------------------------
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


void erro(char *s){
    perror(s);
    exit(1);
}

void send_to_client(char *buf){
    if ((sendto(sock, buf, strlen(buf), 0, (struct sockaddr *) &client_sock, slen) == -1))
        erro("Failed to send message to client.");
}