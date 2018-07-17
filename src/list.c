#include "list.h"

#include <stdio.h>
#include <stdlib.h>

typedef struct node {
    void *item;
    struct node *next;
} node;

node *node_new(void *item) {
    node *n = malloc(sizeof(node));
    n->item = item;
    n->next = NULL;
    return n;
}

struct list {
    node *head;
    node *tail;
    unsigned long size;
};

list *list_new() {
    list *l = malloc(sizeof(list));
    l->head = NULL;
    l->tail = NULL;
    l->size = 0;
    return l;
}

void list_delete(list *self, void (*del_fn)(void *)) {
    node *cur = self->head;
    for (int i = 0; i < self->size; i++) {
        node *tmp = cur;
        if (del_fn)
            del_fn(cur->item);
        cur = cur->next;
        free(tmp);
    }
    free(self);
}

void *list_at(list *self, int index) {
    if (index < 0 || index >= self->size) {
        fprintf(stderr, "index out of bounds\n");
        return NULL;
    }
    node *cur = self->head;
    for (int i = 0; i < index; i++) {
        cur = cur->next;
    }
    return cur->item;
}

void list_push(list *self, void *item) {
    node *n = node_new(item);
    if (self->size) {
        self->tail->next = n;
        self->tail = n;
    } else {
        self->head = self->tail = n;
    }
    self->size++;
}

void *list_pop(list *self) {
    if (self->size == 0) {
        fprintf(stderr, "error: cannot pop from an empty list\n");
        return NULL;
    }
    node *tmp = self->head;
    void *item = tmp->item;
    free(tmp);
    self->head = self->head->next;
    self->size--;
    return item;
}

void list_print(list *self) {
    putchar('[');
    node *cur = self->head;
    for (int i = 0; i < self->size; i++) {
        char *delim = (i == self->size - 1) ? "" : ", ";
        printf("%s%s", (char *)cur->item, delim);
        cur = cur->next;
    }
    printf("]\n");
}

unsigned long list_size(list *self) { return self->size; }

int list_test(void) {
    char *strs[] = {"foo", "bar", "baz", "boom"};

    list *l = list_new();

    for (int i = 0; i < 4; i++) {
        list_push(l, strs[i]);
    }

    list_print(l);

    printf("%s\n", list_at(l, 0));

    for (int i = 0; i < 4; i++) {
        printf("popping '%s' from list...\n", (char *)list_pop(l));
        list_print(l);
    }

    list_delete(l, NULL);
    return 0;
}