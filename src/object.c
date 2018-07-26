#include "object.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int numobj = 0;

/* interior types ---------------------------------------------------------- */

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

builtin_t *mk_builtin(char *name, builtin bltin) {
    builtin_t *b = malloc(sizeof(builtin_t));
    b->proc = bltin;
    b->name = malloc(sizeof(char) * (strlen(name) + 1));
    strcpy(b->name, name);
    return b;
}

lambda_t *mk_lambda(obj *params, obj *body) {
    lambda_t *l = malloc(sizeof(lambda_t));
    l->e = env_new();
    l->params = params;
    l->body = body;
    return l;
}

/* obj types --------------------------------------------------------------- */

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

obj *obj_builtin(char *name, builtin proc) {
    obj *o = malloc(sizeof(obj));
    o->count = 0;
    o->type = OBJ_BUILTIN;
    o->bltin = mk_builtin(name, proc);
    return o;
}

obj *obj_lambda(obj *params, obj *body) {
    obj *o = malloc(sizeof(obj));
    o->count = 0;
    o->type = OBJ_LAMBDA;
    o->lambda = mk_lambda(params, body);
    return o;
}

obj *obj_bool(bool_t b) {
    obj *o = malloc(sizeof(obj));
    o->count = 0;
    o->type = OBJ_BOOL;
    o->bool = b;
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
    case OBJ_BUILTIN:
        return "function";
    case OBJ_KEYWORD:
        return "keyword";
    default:
        return "unknown";
    }
}

/* list fns ---------------------------------------------------------------- */

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

/* copying ----------------------------------------------------------------- */

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
        case OBJ_BUILTIN:
            return obj_builtin(o->bltin->name, o->bltin->proc);
        case OBJ_LAMBDA:
            return obj_lambda(o->lambda->params, o->lambda->body);
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

/* printing ---------------------------------------------------------------- */

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
    case OBJ_BOOL:
        printf("%s", o->bool == TRUE ? "true" : "false");
        break;
    case OBJ_BUILTIN:
        printf("<function %s>", o->bltin->name);
        break;
    case OBJ_LAMBDA:
        printf("<function>");
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

void obj_println(obj *o) {
    obj_print(o);
    putchar('\n');
}

/* deleting ---------------------------------------------------------------- */

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
        case OBJ_BUILTIN:
            free(o->bltin->name);
            free(o->bltin);
            break;
        case OBJ_LAMBDA:
            env_delete(o->lambda->e);
            obj_delete(o->lambda->params);
            obj_delete(o->lambda->body);
            free(o->lambda);
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