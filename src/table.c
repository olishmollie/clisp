#include "table.h"

table_t *table_new(void) {
    table_t *table = malloc(sizeof(table_t));
    return table;
}

unsigned long hash(char *str) {
    unsigned long hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

void table_put(table_t *table, char *key, obj_t *value) {
    unsigned long h = hash(key);
    table->store[h % MAX_TABLE_SIZE] = value;
}

obj_t *table_get(table_t *table, char *key) {
    unsigned long h = hash(key);
    obj_t *result = NULL;

    return (result = table->store[h % MAX_TABLE_SIZE]) ? result : NULL;
}
