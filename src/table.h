#ifndef TABLE_H
#define TABLE_H

#include "object.h"

#include <stdlib.h>

#define MAX_TABLE_SIZE 2048

typedef struct obj_t obj_t;

typedef struct entry_t {
    obj_t *object;
    struct entry_t *next;
} entry_t;

entry_t *entry_new(obj_t *object);

typedef struct table_t {
    size_t size;
    entry_t *store[MAX_TABLE_SIZE];
} table_t;

table_t *table_new(void);

void table_put(table_t *table, char *key, obj_t *value);
obj_t *table_get(table_t *table, char *key);

void table_print(table_t *table);

#endif