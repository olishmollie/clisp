#ifndef _LIST_H
#define _LIST_H

typedef struct list list;

list *list_new();

unsigned long list_size(list *this);

void *list_at(list *this, int index);

void *list_remove(list *this, int index);

void *list_insert(list *this, void *item, int index);

void *list_replace(list *this, void *item, int index);

void list_delete(list *this, void (*del_fn)(void *));

void list_print(list *this, void (*print_fn)(void *));

#endif