/*
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#define BUF_SIZE 1024

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

void read_file_topics(struct node_topics **head, char *file_name);
void add_node_topics(struct node_topics **head, char *topic, char *title, char *text, char *author);
void add_node_news(struct node_news **news, char *topic, char *title, char *text, char *author);
struct node_topics* create_node_topics(char *topic, char *title, char *text, char *author);
struct node_news* create_node_news(char *topic, char *title, char *text, char *author);
void list(struct node_topics *head);
struct node_topics *search_topic(struct node_topics **head, char *topic);


void read_file_topics(struct node_topics **head, char *file_name) {
    printf("7");

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

struct node_topics *create_node_topics(char *topic, char *title, char *text, char *author) {
    struct node_topics* new_node = (struct node_topics*)malloc(sizeof(struct node_topics));
    strcpy(new_node->topic, topic);

    new_node->next = NULL;
    new_node->news = NULL;
    add_node_news(&new_node->news, topic, title, text, author);

    return new_node;
}

struct node_news* create_node_news(char *topic, char *title, char *text, char *author) {

    struct node_news* new_node = (struct node_news*)malloc(sizeof(struct node_news));
    strcpy(new_node->title, title);
    strcpy(new_node->text, text);
    strcpy(new_node->author, author);
    new_node->next = NULL;
    return new_node;
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
int main(int argc, char *argv[]){
    struct node_topics *news = NULL;
    read_file_topics(&news, "Topicos.txt");
    list(news);
    return 0;
}*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#define BUF_SIZE 1024
#define MAX_TOPICS 10

struct node_client_topics{
    char username[BUF_SIZE];
    char topics[MAX_TOPICS][BUF_SIZE];
    struct node_client_topics* next;
};

void read_file_ct(struct node_client_topics **head, char *file_name);

int pos = 0;

void add_node_ct(struct node_client_topics **head, char *username, char topics[MAX_TOPICS][BUF_SIZE]);

struct node_client_topics* create_node_ct(char *username, char topics[MAX_TOPICS][BUF_SIZE]);

void add_node_ct(struct node_client_topics **head, char *username, char topics[MAX_TOPICS][BUF_SIZE]);

void list(struct node_client_topics *head);

int main(int argc, char *argv[]){
    struct node_client_topics *head = NULL;
    read_file_ct(&head, "Topicos.txt");
    list(head);
    return 0;

}

void read_file_ct(struct node_client_topics **head, char *file_name){
    FILE *file;
    char line[BUF_SIZE], data[MAX_TOPICS][BUF_SIZE];
    if ((file = fopen(file_name, "r")) == NULL) {
        printf("Failed to open.\n");
        exit(1);
    }
    while (fgets(line, sizeof(line), file) != NULL) {
        char username[BUF_SIZE];
        char *token = strtok(line, "|\n\r");
        strcpy(username, token);

        while(token != NULL){
            token = strtok(NULL, ";\n\r");
            strcpy(data[pos], token);
            pos++;
        }
        printf("ASDBVGFDS\n");
        //add_node_ct(head, username, data);
    }

}

struct node_client_topics* create_node_ct(char *username, char topics[MAX_TOPICS][BUF_SIZE]) {
    struct node_client_topics* new_node = (struct node_client_topics*)malloc(sizeof(struct node_client_topics));
    printf("ASDBVGFDS\n");
    strcpy(new_node->username, username);
    printf("ASDBVGFDS\n");
    for(int i = 0; i < pos; i++){
        strcpy(new_node->topics[i], topics[i]);
    }

    new_node->next = NULL;
    return new_node;
}

void add_node_ct(struct node_client_topics **head, char *username, char topics[MAX_TOPICS][BUF_SIZE]){
    struct node_client_topics *aux = create_node_ct(username, topics);
    printf("ASDBVGFDS\n");
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

void list(struct node_client_topics *head){
    struct node_client_topics *aux = head;
    while (aux != NULL) {
        printf("%s - ", aux->username);
        printf("[");
        for(int i = 0; i < MAX_TOPICS; i++){
            printf("%s; ", aux->topics[i]);
        }
        printf("]\n");
        aux = aux->next;
    }
}



