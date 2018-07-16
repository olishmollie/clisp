#include "object.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

object *object_long(long lval) {
    object *o = malloc(sizeof(object));
    o->numobj = 0;
    o->type = OBJ_LONG;
    o->lval = lval;
    return o;
}

object *object_sym(char *ident) {
    object *o = malloc(sizeof(object));
    o->numobj = 0;
    o->type = OBJ_SYM;
    o->ident = malloc(sizeof(char) * strlen(ident));
    strcpy(o->ident, ident);
    return o;
}

object *object_sexp(void) {
    object *o = malloc(sizeof(object));
    o->numobj = 0;
    o->type = OBJ_SEXP;
    o->cell = NULL;
    return o;
}

object *object_error(char *error) {
    object *o = malloc(sizeof(object));
    o->numobj = 0;
    o->type = OBJ_ERROR;
    o->error = malloc(sizeof(char) * strlen(error));
    strcpy(o->error, error);
    return o;
}

void object_delete(object *o) {
    switch (o->type) {
    case OBJ_LONG:
        break;
    case OBJ_SYM:
        free(o->ident);
        break;
    case OBJ_ERROR:
        free(o->error);
        break;
    case OBJ_SEXP:
        for (int i = 0; i < o->numobj; i++) {
            object_delete(o->cell[i]);
        }
        free(o->cell);
        break;
    }
    free(o);
}

void print_sexp(object *o) {
    putchar('(');
    for (int i = 0; i < o->numobj; i++) {
        object_print(o->cell[i]);
        if (i != o->numobj - 1) {
            putchar(' ');
        }
    }
    putchar(')');
}

void object_println(object *o) {
    object_print(o);
    putchar('\n');
}

void object_print(object *o) {
    switch (o->type) {
    case OBJ_LONG:
        printf("%li", o->lval);
        break;
    case OBJ_SYM:
        printf("%s", o->ident);
        break;
    case OBJ_ERROR:
        printf("error: %s", o->error);
        break;
    case OBJ_SEXP:
        print_sexp(o);
    }
}

object *object_add(object *o, object *x) {
    o->numobj++;
    o->cell = o->cell ? realloc(o->cell, sizeof(object *) * o->numobj)
                      : malloc(sizeof(object *) * o->numobj);
    o->cell[o->numobj - 1] = x;
    return o;
}
