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
    obj *head;
    int count;
} list_t;

typedef struct env env;
typedef obj *(*builtin)(env *, obj *);
typedef struct {
    char *name;
    builtin proc;
} builtin_t;

typedef struct {
    env *e;
    obj *params;
    obj *body;
} lambda_t;

typedef enum {
    OBJ_NUM,
    OBJ_SYM,
    OBJ_CONS,
    OBJ_LIST,
    OBJ_BOOL,
    OBJ_BUILTIN,
    OBJ_LAMBDA,
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
        list_t *list;
        bool_t bool;
        builtin_t *bltin;
        lambda_t *lambda;
        char *keyword;
        char *err;
    };
};

/* interior types ---------------------------------------------------------- */
num_t *mk_num(long);
cons_t *mk_cons(obj *, obj *);
list_t *mk_list(void);
builtin_t *mk_builtin(char *, builtin);
lambda_t *mk_lambda(obj *, obj *);

/* object types ------------------------------------------------------------ */
obj *obj_num(long);
obj *obj_sym(char *);
obj *obj_bool(bool_t);
obj *obj_cons(obj *, obj *);
obj *obj_list(void);
obj *obj_keyword(char *);
obj *obj_nil(void);
obj *obj_err(char *, ...);
obj *obj_builtin(char *, builtin);
obj *obj_lambda(obj *, obj *);
char *obj_typename(obj_t);

/* list fns ---------------------------------------------------------------- */
obj *obj_add(obj *, obj *);
obj *obj_popcar(obj *);
obj *obj_car(obj *);
obj *obj_cdr(obj *);

/* copying ----------------------------------------------------------------- */
obj *obj_cpy(obj *o);

/* printing ---------------------------------------------------------------- */
void obj_println(obj *o);

/* deleting ---------------------------------------------------------------- */
void obj_delete(obj *o);

#endif