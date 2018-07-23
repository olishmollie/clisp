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
    OBJ_BOOL,
    OBJ_FUN,
    OBJ_QEXPR,
    OBJ_KEYWORD,
    OBJ_NIL,
    OBJ_ERR
} obj_t;

typedef enum { TRUE, FALSE } bool_t;

struct obj {
    obj_t type;
    int count;
    union {
        num_t *num;
        char *sym;
        cons_t *cons;
        bool_t bool;
        fun_t *fun;
        qexpr_t *qexpr;
        char *keyword;
        char *err;
    };
};

/* interior types ---------------------------------------------------------- */
num_t *mk_num(long);
cons_t *mk_cons(obj *, obj *);
fun_t *mk_fun(char *, builtin);
qexpr_t *mk_qexpr(obj *);

/* object types ------------------------------------------------------------ */
obj *obj_num(long);
obj *obj_sym(char *);
obj *obj_bool(bool_t);
obj *obj_cons(obj *, obj *);
obj *obj_keyword(char *);
obj *obj_nil(void);
obj *obj_err(char *, ...);
obj *obj_qexpr(obj *);
obj *obj_fun(char *, builtin);
char *obj_typename(obj_t);

/* list fns ---------------------------------------------------------------- */
obj *obj_car(obj *);
obj *obj_cdr(obj *);
obj *obj_popcar(obj **);
obj *obj_popcdr(obj **);

/* copying ----------------------------------------------------------------- */
obj *obj_cpy(obj *o);

/* printing ---------------------------------------------------------------- */
void obj_println(obj *o);

/* deleting ---------------------------------------------------------------- */
void obj_delete(obj *o);

#endif