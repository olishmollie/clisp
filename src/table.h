#ifndef TABLE_H
#define TABLE_H

#include "object.h"

#define MAX_TABLE_SIZE 2048

typedef struct obj_t obj_t;

typedef struct table_t {
    obj_t *store[MAX_TABLE_SIZE];
} table_t;

table_t *table_new(void);

void table_put(table_t *table, char *key, obj_t *value);
obj_t *table_get(table_t *table, char *key);

#endif