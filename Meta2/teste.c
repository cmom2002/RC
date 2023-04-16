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

