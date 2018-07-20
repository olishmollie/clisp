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

typedef enum { OBJ_NUM, OBJ_SYM, OBJ_CONS, OBJ_NIL, OBJ_BOOL, OBJ_ERR } obj_t;

typedef enum { TRUE, FALSE } bool_t;

struct obj {
    obj_t type;
    union {
        num_t num;
        sym_t sym;
        cons_t cons;
        bool_t bool;
        char *err;
    };
};

num_t mk_num(long num);
sym_t mk_sym(char *sym);
cons_t mk_cons(obj *a, obj *b);

obj *obj_num(long val);
obj *obj_sym(char *name);
obj *obj_cons(obj *car, obj *cdr);
obj *obj_nil(void);
obj *obj_bool(char *val);
obj *obj_err(char *err);

obj *_car(obj *o);
obj *_cdr(obj *o);

obj *obj_pop(obj **o);

void obj_println(obj *o);

void obj_delete(obj *o);

#endif