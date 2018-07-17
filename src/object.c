#include "object.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

object *object_long(long lval) {
    object *o = malloc(sizeof(object));
    o->type = OBJ_LONG;
    o->lval = lval;
    return o;
}

object *object_sym(char *sym) {
    object *o = malloc(sizeof(object));
    o->type = OBJ_SYM;
    o->sym = malloc(sizeof(char) * strlen(sym));
    strcpy(o->sym, sym);
    return o;
}

object *object_sexp(void) {
    object *o = malloc(sizeof(object));
    o->type = OBJ_SEXP;
    o->cell = list_new();
    return o;
}

object *object_error(char *error) {
    object *o = malloc(sizeof(object));
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
        free(o->sym);
        break;
    case OBJ_ERROR:
        free(o->error);
        break;
    case OBJ_SEXP:
        list_delete(o->cell, (void *)object_delete);
        break;
    }
    free(o);
}

void print_sexp(object *o) {
    putchar('(');
    for (int i = 0; i < list_size(o->cell); i++) {
        object_print(list_at(o->cell, i));
        if (i != list_size(o->cell) - 1) {
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
        printf("%s", o->sym);
        break;
    case OBJ_ERROR:
        printf("error: %s", o->error);
        break;
    case OBJ_SEXP:
        print_sexp(o);
    }
}
