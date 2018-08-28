#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include "object.h"

#define MAX_ENV_SIZE 2048

typedef struct obj_t obj_t;

typedef struct env_t {
    struct env_t *enclosing;
    int obj_count;
    obj_t *symbols[MAX_ENV_SIZE];
    obj_t *objects[MAX_ENV_SIZE];
} env_t;

env_t *env_new(void);

obj_t *env_define(env_t *env, obj_t *symbol, obj_t *object);
obj_t *env_set(env_t *env, obj_t *sybol, obj_t *object);

obj_t *env_lookup(env_t *env, obj_t *symbol);

void env_delete(env_t *env);

#endif