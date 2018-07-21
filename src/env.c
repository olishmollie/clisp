#include "env.h"
#include "object.h"

#include <stdlib.h>
#include <string.h>

env *env_new(void) {
    env *e = malloc(sizeof(env));
    e->count = 0;
    e->syms = NULL;
    e->vals = NULL;
    return e;
}

obj *env_lookup(env *e, obj *k) {
    for (int i = 0; i < e->count; i++) {
        if (strcmp(e->syms[i], k->sym.name) == 0)
            return obj_cpy(e->vals[i]);
    }
    return obj_err("unbound symbol '%s'", k->sym.name);
}

void env_insert(env *e, obj *k, obj *v) {

    /* Overwrite an exisiting symbol if exists */
    for (int i = 0; i < e->count; i++) {
        if (strcmp(e->syms[i], k->sym.name) == 0) {
            obj_delete(e->vals[i]);
            e->vals[i] = obj_cpy(v);
        }
    }

    /* Insert new object */
    ++e->count;
    e->vals = realloc(e->vals, sizeof(obj *) * e->count);
    e->syms = realloc(e->syms, sizeof(char *) * e->count);

    e->vals[e->count - 1] = obj_cpy(v);
    e->syms[e->count - 1] = malloc(sizeof(char) * (strlen(k->sym.name) + 1));
    strcpy(e->syms[e->count - 1], k->sym.name);
}

void env_delete(env *e) {
    for (int i = 0; i < e->count; i++) {
        free(e->syms[i]);
        obj_delete(e->vals[i]);
    }
    free(e->syms);
    free(e->vals);
    free(e);
}
