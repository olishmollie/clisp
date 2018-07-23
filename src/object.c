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

cons_t *mk_cons(obj *car, obj *cdr) {
    cons_t *c = malloc(sizeof(cons_t));
    c->car = car;
    c->cdr = cdr;
    return c;
}

fun_t *mk_fun(char *name, builtin proc) {
    fun_t *f = malloc(sizeof(fun_t));
    f->proc = proc;
    f->name = malloc(sizeof(char) * (strlen(name) + 1));
    strcpy(f->name, name);
    return f;
}

qexpr_t *mk_qexpr(obj *o) {
    qexpr_t *q = malloc(sizeof(qexpr_t));
    q->child = o;
    return q;
}

obj *obj_num(long val) {
    obj *o = malloc(sizeof(obj));
    o->count = 0;
    o->type = OBJ_NUM;
    o->num = mk_num(val);
    return o;
}

obj *obj_sym(char *name) {
    obj *o = malloc(sizeof(obj));
    o->count = 0;
    o->type = OBJ_SYM;
    o->sym = malloc(sizeof(char) * (strlen(name) + 1));
    strcpy(o->sym, name);
    return o;
}

obj *obj_cons(obj *car, obj *cdr) {
    obj *o = malloc(sizeof(obj));
    o->count = 0;
    o->type = OBJ_CONS;
    o->cons = mk_cons(car, cdr);
    return o;
}

obj *obj_fun(char *name, builtin proc) {
    obj *o = malloc(sizeof(obj));
    o->count = 0;
    o->type = OBJ_FUN;
    o->fun = mk_fun(name, proc);
    return o;
}

obj *obj_bool(bool_t b) {
    obj *o = malloc(sizeof(obj));
    o->count = 0;
    o->type = OBJ_BOOL;
    o->bool = b;
    return o;
}

obj *obj_qexpr(obj *child) {
    obj *o = malloc(sizeof(obj));
    o->count = 0;
    o->type = OBJ_QEXPR;
    o->qexpr = mk_qexpr(child);
    return o;
}

obj *obj_keyword(char *name) {
    obj *o = malloc(sizeof(obj));
    o->count = 0;
    o->type = OBJ_KEYWORD;
    o->keyword = malloc(sizeof(char) * (strlen(name) + 1));
    strcpy(o->keyword, name);
    return o;
}

obj *obj_nil(void) {
    obj *o = malloc(sizeof(obj));
    o->type = OBJ_NIL;
    o->count = 0;
    return o;
}

obj *obj_err(char *fmt, ...) {
    obj *o = malloc(sizeof(obj));
    o->count = 0;
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

obj *obj_popcar(obj **o) {
    obj *car = obj_car(*o);
    *o = obj_cdr(*o);
    return car;
}

obj *obj_popcdr(obj **o) {
    obj *cdr = obj_cdr(*o);
    *o = obj_car(*o);
    return cdr;
}

obj *cpy_cons(obj *o) {
    obj *car = obj_cpy(obj_car(o));
    obj *cdr = obj_cpy(obj_cdr(o));
    obj *cpy = obj_cons(car, cdr);
    cpy->cons->car = car;
    cpy->cons->cdr = cdr;
    return cpy;
}

obj *obj_cpy(obj *o) {
    if (o) {
        switch (o->type) {
        case OBJ_NUM:
            return obj_num(o->num->val);
        case OBJ_SYM:
            return obj_sym(o->sym);
        case OBJ_CONS:
            return cpy_cons(o);
        case OBJ_QEXPR:
            return obj_qexpr(obj_cpy(o->qexpr->child));
        case OBJ_FUN:
            return obj_fun(o->fun->name, o->fun->proc);
        case OBJ_ERR:
            return obj_err(o->err);
        case OBJ_NIL:
            return obj_nil();
        case OBJ_KEYWORD:
            return obj_keyword(o->keyword);
        case OBJ_BOOL:
            return obj_bool(o->bool);
        }
    }
    return NULL;
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
    case OBJ_FUN:
        return "function";
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
    obj *p = o;
    while (1) {
        obj_print(obj_car(p));
        obj *cdr = obj_cdr(p);
        if (cdr->type != OBJ_CONS) {
            if (cdr->type != OBJ_NIL) {
                printf(" . ");
                obj_print(cdr);
            }
            putchar(')');
            break;
        }
        putchar(' ');
        p = obj_cdr(p);
    }
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
        case OBJ_NIL:
            printf("()");
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
            obj_delete(obj_car(o));
            obj_delete(obj_cdr(o));
            free(o->cons);
            break;
        case OBJ_BOOL:
            break;
        case OBJ_ERR:
            free(o->err);
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
            break;
        case OBJ_NIL:
            break;
        }
    }
    free(o);
}