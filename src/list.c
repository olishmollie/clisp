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

void list_delete(list *this, void (*del_fn)(void *)) {
    node *cur = this->head;
    for (int i = 0; i < this->size; i++) {
        node *tmp = cur;
        if (del_fn)
            del_fn(cur->item);
        cur = cur->next;
        free(tmp);
    }
    free(this);
}

void *list_at(list *this, int index) {
    if (index < 0 || index >= this->size) {
        fprintf(stderr, "error list_at(%d): index out of bounds\n", index);
        return NULL;
    }
    node *cur = this->head;
    for (int i = 0; i < index; i++) {
        cur = cur->next;
    }
    return cur->item;
}

void *list_pushfront(list *this, void *item) {
    node *n = node_new(item);
    if (this->size) {
        n->next = this->head;
        this->head = n;
    } else {
        this->head = this->tail = n;
    }
    this->size++;
    return item;
}

void *list_pushback(list *this, void *item) {
    node *n = node_new(item);
    if (this->size) {
        this->tail->next = n;
        this->tail = n;
    } else {
        this->head = this->tail = n;
    }
    this->size++;
    return item;
}

void *list_insert(list *this, void *item, int index) {
    if (index == 0)
        return list_pushfront(this, item);
    if (index == this->size)
        return list_pushback(this, item);

    if (index < 0 || index >= this->size) {
        fprintf(stderr, "error list_insert(%d): index out of bounds\n", index);
        return NULL;
    }

    node *n = node_new(item);
    node *cur = this->head;
    for (int i = 0; i < index - 1; i++) {
        cur = cur->next;
    }
    n->next = cur->next;
    cur->next = n;
    this->size++;
    return item;
}

void *list_popfront(list *this) {
    if (this->size == 0) {
        fprintf(stderr,
                "error list_popfront(): cannot pop from an empty list\n");
        return NULL;
    }
    node *tmp = this->head;
    void *item = tmp->item;
    free(tmp);
    this->head = this->head->next;
    this->size--;
    return item;
}

void *list_popback(list *this) {
    if (this->size == 0) {
        fprintf(stderr,
                "error list_popback(): cannot pop from an empty list\n");
        return NULL;
    }
    node *cur = this->head;
    for (int i = 0; i < this->size - 1; i++) {
        cur = cur->next;
    }
    node *tmp = this->tail;
    void *item = tmp->item;
    free(tmp);
    this->tail = cur;
    this->size--;
    return item;
}

void *list_remove(list *this, int index) {
    if (this->size == 0) {
        fprintf(stderr,
                "error list_remove(%d): cannot remove from an empty list\n",
                index);
        return NULL;
    }
    if (index == 0)
        return list_popfront(this);
    if (index == this->size - 1)
        return list_popback(this);

    if (index < 0 || index >= this->size) {
        fprintf(stderr, "error list_remove(%d): pop() index out of bounds\n",
                index);
        return NULL;
    }

    node *cur = this->head;
    for (int i = 0; i < index - 1; i++) {
        cur = cur->next;
    }
    node *tmp = cur->next;
    void *item = tmp->item;
    cur->next = tmp->next;
    free(tmp);
    this->size--;
    return item;
}

void *list_replace(list *this, void *item, int index) {
    list_remove(this, index);
    return list_insert(this, item, index);
}

void list_print(list *this, void (*print_fn)(void *)) {
    putchar('[');
    node *cur = this->head;
    for (int i = 0; i < this->size; i++) {
        if (print_fn) {
            char *delim = (i == this->size - 1) ? "" : ", ";
            print_fn(cur->item);
            printf("%s", delim);
        }
        cur = cur->next;
    }
    printf("]\n");
}

unsigned long list_size(list *this) { return this->size; }

// int main(void) {
//     char *strs[] = {"foo", "bar", "baz", "boom"};

//     list *l = list_new();

//     for (int i = 0; i < 4; i++) {
//         list_pushback(l, strs[i]);
//     }

//     list_print(l);
//     putchar('\n');

//     list_insert(l, "foobar", 1);
//     list_insert(l, "barfoo", 3);

//     list_print(l);
//     printf("\n");

//     list_replace(l, "x", 1);
//     list_replace(l, "x", 3);

//     list_print(l);

//     list_delete(l, NULL);
//     return 0;
// }