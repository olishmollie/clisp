#include "object.h"

#include <stdarg.h>
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

sexpr_t mk_sexpr(void) {
    sexpr_t s;
    s.count = 0;
    s.cell = NULL;
    return s;
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

obj *obj_sexpr(void) {
    obj *o = malloc(sizeof(obj));
    o->type = OBJ_SEXPR;
    o->sexpr = mk_sexpr();
    return o;
}

obj *obj_err(char *fmt, ...) {
    obj *o = malloc(sizeof(obj));
    o->type = OBJ_ERR;

    va_list args;
    va_start(args, fmt);
    o->err = malloc(sizeof(char) * 512);
    vsnprintf(o->err, 511, fmt, args);
    va_end(args);

    return o;
}

obj *car(obj *o) {
    if (o && o->type == OBJ_CONS)
        return o->cons.car;
    return obj_err("param passed to car not a cons");
}

obj *cdr(obj *o) {
    if (o && o->type == OBJ_CONS)
        return o->cons.cdr;
    return obj_err("param passed to cdr not a cons");
}

void obj_add(obj *o, obj *p) {
    o->sexpr.cell = realloc(o->sexpr.cell, sizeof(obj *) * ++o->sexpr.count);
    o->sexpr.cell[o->sexpr.count - 1] = p;
}

obj *obj_pop(obj *o, int i) {
    obj *res = o->sexpr.cell[i];
    memmove(o->sexpr.cell[i], o->sexpr.cell[i + 1],
            sizeof(obj *) * (o->sexpr.count - i - 1));
    o->sexpr.cell = realloc(o->sexpr.cell, sizeof(obj *) * --o->sexpr.count);
    return res;
}

obj *obj_take(obj *o, int i) {
    obj *res = obj_pop(o, i);
    obj_delete(o);
    return res;
}

char *obj_typename(obj_t type) {
    switch (type) {
    case OBJ_NUM:
        return "number";
    case OBJ_SYM:
        return "symbol";
    case OBJ_BOOL:
        return "bool";
    case OBJ_CONS:
        return "cons";
    case OBJ_ERR:
        return "error";
    case OBJ_NIL:
        return "nil";
    case OBJ_SEXPR:
        return "s-expression";
    default:
        return "unknown";
    }
}

void obj_print(obj *o);

void print_cons(obj *o) {
    putchar('(');
    obj_print(car(o));
    printf(" . ");
    obj_print(cdr(o));
    putchar(')');
}

void print_sexpr(obj *o) {
    putchar('(');
    for (int i = 0; i < o->sexpr.count; i++) {
        char *delim = i == o->sexpr.count - 1 ? "" : " ";
        obj_print(o->sexpr.cell[i]);
        printf("%s", i != o->sexpr.count - 1 ? " " : "");
    }
    putchar(')');
}

void obj_print(obj *o) {
    if (o) {
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
            printf("nil");
            break;
        case OBJ_SEXPR:
            print_sexpr(o);
            break;
        case OBJ_BOOL:
            printf("%s", o->bool == TRUE ? "true" : "false");
            break;
        case OBJ_ERR:
            printf("Error: %s", o->err);
            break;
        default:
            printf("Cannot print unknown obj type\n");
        }
    }
}

void obj_println(obj *o) {
    obj_print(o);
    putchar('\n');
}

void obj_delete(obj *o);

void delete_sexpr(obj *o) {
    for (int i = 0; i < o->sexpr.count; i++) {
        obj_delete(o->sexpr.cell[i]);
    }
    free(o->sexpr.cell);
}

void obj_delete(obj *o) {
    if (o) {
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
            break;
        case OBJ_SEXPR:
            delete_sexpr(o);
        }
    }
    free(o);
}