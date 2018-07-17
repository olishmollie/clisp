#ifndef _OBJECT_H
#define _OBJECT_H

#include "list.h"

typedef enum { OBJ_LONG, OBJ_SYM, OBJ_SEXP, OBJ_ERROR } object_t;

typedef struct object {
    object_t type;
    int numobj;
    union {
        long lval;
        char *ident;
        list *cell;
        char *error;
    };
} object;

object *object_long(long lval);

object *object_sym(char *ident);

object *object_sexp(void);

object *object_error(char *error);

void object_delete(object *o);

void object_println(object *o);

void object_print(object *o);

object *object_add(object *o, object *x);

#endif