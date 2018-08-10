#ifndef OBJECT_H
#define OBJECT_H

#include "lex.h"
#include "gmp.h"

typedef struct obj obj;

typedef enum { NUM_INT, NUM_RAT, NUM_DBL, NUM_ERR } num_type;

typedef struct {
    num_type type;
    union {
        mpz_t integ;
        mpq_t rat;
        mpf_t dbl;
        char *err;
    };
} num_t;

typedef struct {
    obj *car;
    obj *cdr;
} cons_t;

typedef struct env env;
typedef obj *(*builtin)(env *, obj *);

typedef struct {
    char *name;
    builtin proc;
    env *e;
    obj *params;
    obj *body;
} fun_t;

typedef enum { CONST_CHAR, CONST_BOOL, CONST_ERR } const_type;
typedef enum { BOOL_T, BOOL_F } bool_t;

typedef struct {
    const_type type;
    char *repr;
    union {
        char c;
        bool_t bool;
        char *err;
    };
} const_t;

typedef enum {
    OBJ_NUM,
    OBJ_SYM,
    OBJ_STR,
    OBJ_CONS,
    OBJ_CONST,
    OBJ_FUN,
    OBJ_KEYWORD,
    OBJ_NIL,
    OBJ_ERR
} obj_t;

struct obj {
    obj_t type;
    int nargs;
    union {
        num_t *num;
        char *sym;
        char *str;
        cons_t *cons;
        const_t *constant;
        fun_t *fun;
        char *keyword;
        char *err;
    };
};

struct env {
    struct env *parent;
    int count;
    char **syms;
    obj **vals;
};

env *env_new(void);
obj *env_lookup(env *e, obj *k);
void env_insert(env *e, obj *k, obj *v);
void env_set(env *e, obj *k, obj *v);
void env_delete(env *e);
void env_print(env *e);

obj *obj_num(char *numstr, token_t ttype);
obj *obj_int(mpz_t integ);
obj *obj_rat(mpq_t rat);
obj *obj_dbl(mpf_t dbl);

obj *obj_sym(char *name);
obj *obj_str(char *str);
obj *obj_cons(obj *car, obj *cdr);
obj *obj_builtin(char *name, builtin proc);
obj *obj_lambda(env *e, obj *params, obj *body);
obj *obj_const(char *constant);
obj *obj_char(char c);
obj *obj_bool(bool_t type);
int obj_isfalse(obj *c);
int obj_istrue(obj *c);

obj *obj_keyword(char *name);
obj *obj_nil(void);
obj *obj_err(char *fmt, ...);
int obj_isatom(obj *o);

char *obj_typename(obj_t type);
obj *obj_car(obj *o);
obj *obj_cdr(obj *o);
obj *obj_cadr(obj *o);
obj *obj_popcar(obj **o);
obj *cpy_const(obj *o);
obj *obj_cpy(obj *o);
void obj_print(obj *o);
void obj_println(obj *o);
void obj_delete(obj *o);

#endif