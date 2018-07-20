#include "object.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

num_t mk_num(long val) {
    num_t n;
    n.val = val;
    return n;
}

sym_t mk_sym(char *sym) {
    sym_t s;
    s.name = malloc(sizeof(char) * strlen(sym) + 1);
    strcpy(s.name, sym);
    return s;
}

cons_t mk_cons(obj *a, obj *b) {
    cons_t c;
    c.car = a;
    c.cdr = b;
    return c;
}

obj *obj_num(long val) {
    obj *o = malloc(sizeof(obj));
    o->type = OBJ_NUM;
    o->num = mk_num(val);
    return o;
}

obj *obj_sym(char *name) {
    obj *o = malloc(sizeof(obj));
    o->type = OBJ_SYM;
    o->sym = mk_sym(name);
    return o;
}

obj *obj_cons(obj *car, obj *cdr) {
    obj *o = malloc(sizeof(obj));
    o->type = OBJ_CONS;
    o->cons = mk_cons(car, cdr);
    return o;
}

obj *obj_nil(void) {
    obj *o = malloc(sizeof(obj));
    o->type = OBJ_NIL;
    return o;
}

obj *obj_bool(char *val) {
    obj *o = malloc(sizeof(obj));
    o->type = OBJ_BOOL;
    o->bool = strcmp(val, "true") == 0 ? TRUE : FALSE;
    return o;
}

obj *obj_err(char *err) {
    obj *o = malloc(sizeof(obj));
    o->type = OBJ_ERR;
    o->err = malloc(sizeof(char) * strlen(err) + 1);
    strcpy(o->err, err);
    return o;
}

obj *_car(obj *o) {
    if (o->type == OBJ_CONS)
        return o->cons.car;
    return obj_err("param passed to car not a cons");
}

obj *_cdr(obj *o) {
    if (o->type == OBJ_CONS)
        return o->cons.cdr;
    return obj_err("param passed to cdr not a cons");
}

obj *obj_pop(obj **o) {
    obj *res = _car(*o);
    *o = _cdr(*o);
    return res;
}

void obj_print(obj *o);

void print_cons(obj *o) {
    putchar('(');
    obj_print(_car(o));
    printf(" . ");
    obj_print(_cdr(o));
    putchar(')');
}

void obj_print(obj *o) {
    switch (o->type) {
    case OBJ_NUM:
        printf("%li", o->num.val);
        break;
    case OBJ_SYM:
        printf("%s", o->sym.name);
        break;
    case OBJ_CONS:
        print_cons(o);
        break;
    case OBJ_NIL:
        // TODO
        printf("nil");
        break;
    case OBJ_BOOL:
        printf("%s", o->bool == TRUE ? "true" : "false");
        break;
    case OBJ_ERR:
        printf("%s", o->err);
        break;
    default:
        printf("Cannot print unknown obj type\n");
        exit(1);
    }
}

void obj_println(obj *o) {
    obj_print(o);
    putchar('\n');
}

void obj_delete(obj *o) {
    switch (o->type) {
    case OBJ_NUM:
        break;
    case OBJ_SYM:
        free(o->sym.name);
        break;
    case OBJ_CONS:
        obj_delete(o->cons.car);
        obj_delete(o->cons.cdr);
        break;
    case OBJ_NIL:
    case OBJ_BOOL:
        break;
    case OBJ_ERR:
        free(o->err);
    }
    free(o);
}