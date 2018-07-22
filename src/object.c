#include "object.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int numobj = 0;

num_t *mk_num(long val) {
    num_t *n = malloc(sizeof(num_t));
    n->val = val;
    return n;
}

cons_t *mk_cons(obj *a, obj *b) {
    cons_t *c = malloc(sizeof(cons_t));
    c->car = a;
    c->cdr = b;
    return c;
}

fun_t *mk_fun(char *name, builtin proc) {
    fun_t *f = malloc(sizeof(fun_t));
    f->proc = proc;
    f->name = malloc(sizeof(char) * (strlen(name) + 1));
    strcpy(f->name, name);
    return f;
}

sexpr_t *mk_sexpr(void) {
    sexpr_t *e = malloc(sizeof(sexpr_t));
    e->count = 0;
    e->cell = NULL;
    return e;
}

qexpr_t *mk_qexpr(obj *o) {
    qexpr_t *q = malloc(sizeof(qexpr_t));
    q->child = o;
    return q;
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
    o->sym = malloc(sizeof(char) * (strlen(name) + 1));
    strcpy(o->sym, name);
    return o;
}

obj *obj_cons(obj *car, obj *cdr) {
    obj *o = malloc(sizeof(obj));
    o->type = OBJ_CONS;
    o->cons = mk_cons(car, cdr);
    return o;
}

obj *obj_fun(char *name, builtin proc) {
    obj *o = malloc(sizeof(obj));
    o->type = OBJ_FUN;
    o->fun = mk_fun(name, proc);
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

obj *obj_qexpr(obj *child) {
    obj *o = malloc(sizeof(obj));
    o->type = OBJ_QEXPR;
    o->qexpr = mk_qexpr(child);
    return o;
}

obj *obj_keyword(char *name) {
    obj *o = malloc(sizeof(obj));
    o->type = OBJ_KEYWORD;
    o->keyword = malloc(sizeof(char) * (strlen(name) + 1));
    strcpy(o->keyword, name);
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

obj *obj_car(obj *o) { return o->cons->car; }
obj *obj_cdr(obj *o) { return o->cons->cdr; }

void obj_add(obj *o, obj *p) {
    o->sexpr->cell = realloc(o->sexpr->cell, sizeof(obj *) * ++o->sexpr->count);
    o->sexpr->cell[o->sexpr->count - 1] = p;
}

obj *obj_pop(obj *o, int i) {
    obj *res = o->sexpr->cell[i];
    memmove(&o->sexpr->cell[i], &o->sexpr->cell[i + 1],
            sizeof(obj *) * (o->sexpr->count - i - 1));
    o->sexpr->cell = realloc(o->sexpr->cell, sizeof(obj *) * --o->sexpr->count);
    return res;
}

obj *obj_take(obj *o, int i) {
    obj *res = obj_pop(o, i);
    obj_delete(o);
    return res;
}

obj *cpy_sexpr(obj *o) {
    obj *cpy = obj_sexpr();
    for (int i = 0; i < o->sexpr->count; i++) {
        obj_add(cpy, obj_cpy(o->sexpr->cell[i]));
    }
    return cpy;
}

obj *obj_cpy(obj *o) {
    switch (o->type) {
    case OBJ_NUM:
        return obj_num(o->num->val);
    case OBJ_SYM:
        return obj_sym(o->sym);
    case OBJ_CONS:
        return obj_cons(obj_cpy(obj_car(o)), obj_cpy(obj_cdr(o)));
    case OBJ_SEXPR:
        return cpy_sexpr(o);
    case OBJ_QEXPR:
        return obj_qexpr(obj_cpy(o->qexpr->child));
    case OBJ_FUN:
        return obj_fun(o->fun->name, o->fun->proc);
    case OBJ_ERR:
        return obj_err(o->err);
    case OBJ_KEYWORD:
    case OBJ_BOOL:
    case OBJ_NIL:
        return o;
    }
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
    case OBJ_FUN:
        return "function";
    case OBJ_SEXPR:
        return "s-expression";
    case OBJ_QEXPR:
        return "q-expression";
    case OBJ_KEYWORD:
        return "keyword";
    default:
        return "unknown";
    }
}

void obj_print(obj *o);

void print_cons(obj *o) {
    putchar('(');
    obj_print(obj_car(o));
    printf(" . ");
    obj_print(obj_cdr(o));
    putchar(')');
}

void print_sexpr(obj *o) {
    putchar('(');
    for (int i = 0; i < o->sexpr->count; i++) {
        obj_print(o->sexpr->cell[i]);
        printf("%s", i != o->sexpr->count - 1 ? " " : "");
    }
    putchar(')');
}

void obj_print(obj *o) {
    if (o) {
        switch (o->type) {
        case OBJ_NUM:
            printf("%li", o->num->val);
            break;
        case OBJ_SYM:
            printf("%s", o->sym);
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
        case OBJ_QEXPR:
            obj_print(o->qexpr->child);
            break;
        case OBJ_BOOL:
            printf("%s", o->bool == TRUE ? "true" : "false");
            break;
        case OBJ_FUN:
            printf("<function '%s'>", o->fun->name);
            break;
        case OBJ_ERR:
            printf("Error: %s", o->err);
            break;
        case OBJ_KEYWORD:
            printf("%s", o->keyword);
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
    for (int i = 0; i < o->sexpr->count; i++) {
        obj_delete(o->sexpr->cell[i]);
    }
    free(o->sexpr->cell);
}

void obj_delete(obj *o) {
    if (o) {
        switch (o->type) {
        case OBJ_NUM:
            free(o->num);
            break;
        case OBJ_SYM:
            free(o->sym);
            break;
        case OBJ_CONS:
            obj_delete(o->cons->car);
            obj_delete(o->cons->cdr);
            free(o->cons);
            break;
        case OBJ_NIL:
        case OBJ_BOOL:
            break;
        case OBJ_ERR:
            free(o->err);
            break;
        case OBJ_SEXPR:
            delete_sexpr(o);
            break;
        case OBJ_QEXPR:
            obj_delete(o->qexpr->child);
            free(o->qexpr);
            break;
        case OBJ_FUN:
            free(o->fun->name);
            free(o->fun);
            break;
        case OBJ_KEYWORD:
            free(o->keyword);
        }
    }
    free(o);
}