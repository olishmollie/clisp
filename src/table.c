#include "table.h"

entry_t *entry_new(obj_t *object) {
    entry_t *entry = malloc(sizeof(entry_t));
    entry->object = object;
    entry->next = NULL;
    return entry;
}

table_t *table_new(void) {
    table_t *table = calloc(MAX_TABLE_SIZE, sizeof(table_t));
    table->size = MAX_TABLE_SIZE;
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
    entry_t **entry = &table->store[h % table->size];
    while (*entry) {
       entry = &(*entry)->next;
    }
    *entry = entry_new(value);
}

obj_t *table_get(table_t *table, char *key) {
    unsigned long h = hash(key);
    entry_t *entry = table->store[h % table->size];
    while (entry) {
        if (strcmp(entry->object->sym, key) == 0) {
            return entry->object;
        }
        entry = entry->next;
    }

    return NULL;
}

void table_print(table_t *table) {
    for (int i = 0; i < MAX_TABLE_SIZE; i++) {
        if (table->store[i]) {
            println(table->store[i]->object);
            entry_t *tmp = table->store[i]->next;
            while (tmp) {
                printf("\t%s\n", tmp->object->sym);
                tmp = tmp->next;
            }
        }
    }
}

void table_delete(table_t *table) {
    for (int i = 0; i < table->size; i++) {
        entry_t *entry = table->store[i];
        while (entry) {
            free(entry);
            entry = entry->next;
        }
    }
    free(table);
}