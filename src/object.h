#ifndef _OBJECT_H
#define _OBJECT_H

#include "list.h"

typedef struct {
    long val;
} num_t;

typedef struct {
    char *name;
} sym_t;

typedef struct obj obj;

typedef struct {
    obj *car;
    obj *cdr;
} cons_t;

typedef struct {
    int count;
    obj **cell;
} sexpr_t;

typedef enum {
    OBJ_NUM,
    OBJ_SYM,
    OBJ_CONS,
    OBJ_NIL,
    OBJ_BOOL,
    OBJ_SEXPR,
    OBJ_ERR
} obj_t;

typedef enum { TRUE, FALSE } bool_t;

struct obj {
    obj_t type;
    union {
        num_t num;
        sym_t sym;
        cons_t cons;
        bool_t bool;
        sexpr_t sexpr;
        char *err;
    };
};

num_t mk_num(long);
sym_t mk_sym(char *);
cons_t mk_cons(obj *, obj *);
sexpr_t mk_sexpr(void);

obj *obj_num(long);
obj *obj_sym(char *);
obj *obj_nil(void);
obj *obj_bool(char *);
obj *obj_err(char *, ...);

obj *obj_cons(obj *, obj *);
obj *car(obj *);
obj *cdr(obj *);

obj *obj_sexpr(void);
void obj_add(obj *, obj *);
obj *obj_pop(obj *, int);
obj *obj_take(obj *, int);

char *obj_typename(obj_t);

void obj_println(obj *o);

void obj_delete(obj *o);

#endif