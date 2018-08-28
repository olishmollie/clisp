#include "env.h"

env_t *env_new(void) {
    env_t *env = malloc(sizeof(env_t));
    env->enclosing = NULL;
    env->obj_count = 0;
    return env;
}

obj_t *env_define(env_t *env, obj_t *symbol, obj_t *object) {
    for (int i = 0; i < env->obj_count; i++) {
        if (env->symbols[i] == symbol) {
            env->objects[i] = object;
            return NULL;
        }
    }

    // TODO: make env variable sized array.
    if (env->obj_count >= MAX_ENV_SIZE) {
        fprintf(stderr, "environment full\n");
        exit(1);
    }

    env->symbols[env->obj_count] = symbol;
    env->objects[env->obj_count] = object;
    env->obj_count++;

    return NULL;
}

obj_t *env_set(env_t *env, obj_t *symbol, obj_t *object) {
    for (int i = 0; i < env->obj_count; i++) {
        if (env->symbols[i] == symbol) {
            env->symbols[i] = object;
            return NULL;
        }
    }

    if (env->enclosing)
        return env_set(env->enclosing, symbol, object);

    return mk_err("unbound symbol '%s'", symbol->sym);
}

obj_t *env_lookup(env_t *env, obj_t *symbol) {
    for (int i = 0; i < env->obj_count; i++) {
        if (env->symbols[i] == symbol) {
            return env->objects[i];
        }
    }

    if (env->enclosing)
        return env_lookup(env->enclosing, symbol);

    return mk_err("unbound symbol '%s'", symbol->sym);
}

void env_print(env_t *env) {
    for (int i = 0; i < env->obj_count; i++) {
        print(env->symbols[i]);
        printf(" : ");
        println(env->objects[i]);
    }
}