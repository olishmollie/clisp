#ifndef _ENV_H
#define _ENV_H

#include "object.h"

struct obj;
typedef struct obj obj;

typedef struct env {
    struct env *parent;
    int count;
    char **syms;
    obj **vals;
} env;

env *env_new(void);
obj *env_lookup(env *e, obj *k);
void env_insert(env *e, obj *k, obj *v);

void env_delete(env *e);

void env_print(env *e);

#endif