#ifndef _OBJECT_H
#define _OBJECT_H

#include "builtins.h"

typedef struct {
    long val;
} num_t;

typedef struct obj obj;

typedef struct {
    obj *car;
    obj *cdr;
} cons_t;

typedef struct {
    int count;
    obj **cell;
} sexpr_t;

typedef struct {
    obj *child;
} qexpr_t;

typedef struct env env;
typedef obj *(*builtin)(env *, obj *);
typedef struct {
    char *name;
    builtin proc;
} fun_t;

typedef enum {
    OBJ_NUM,
    OBJ_SYM,
    OBJ_CONS,
    OBJ_NIL,
    OBJ_BOOL,
    OBJ_FUN,
    OBJ_SEXPR,
    OBJ_QEXPR,
    OBJ_KEYWORD,
    OBJ_ERR
} obj_t;

typedef enum { TRUE, FALSE } bool_t;

struct obj {
    obj_t type;
    union {
        num_t *num;
        char *sym;
        cons_t *cons;
        bool_t bool;
        fun_t *fun;
        sexpr_t *sexpr;
        qexpr_t *qexpr;
        char *keyword;
        char *err;
    };
};

num_t *mk_num(long);
cons_t *mk_cons(obj *, obj *);
fun_t *mk_fun(char *, builtin);
sexpr_t *mk_sexpr(void);
qexpr_t *mk_qexpr(obj *);

obj *obj_num(long);
obj *obj_sym(char *);
obj *obj_nil(void);
obj *obj_bool(char *);
obj *obj_cons(obj *, obj *);
obj *obj_keyword(char *);
obj *obj_err(char *, ...);

obj *obj_fun(char *, builtin);

obj *obj_sexpr(void);
void obj_add(obj *, obj *);
obj *obj_pop(obj *, int);
obj *obj_take(obj *, int);

obj *obj_qexpr(obj *);

obj *obj_cpy(obj *o);

char *obj_typename(obj_t);

void obj_println(obj *o);

void obj_delete(obj *o);

#endif