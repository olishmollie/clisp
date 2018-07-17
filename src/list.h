#ifndef _LIST_H
#define _LIST_H

typedef struct list list;

list *list_new();

unsigned long list_size(list *self);

void *list_at(list *self, int index);

void *list_pop(list *self);

void list_push(list *self, void *item);

void list_delete(list *self, void (*del_fn)(void *));

#endif