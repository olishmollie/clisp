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
typedef obj *(*builtin)(obj *);

typedef struct {
    char *name;
    builtin proc;
} builtin_t;

typedef struct {
    char *name;
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
    OBJ_BUILTIN,
    OBJ_FUN,
    OBJ_NIL,
    OBJ_ERR
} obj_t;

struct obj {
    obj_t type;
    int nargs;
    unsigned char mark;
    union {
        num_t *num;
        char *sym;
        char *str;
        cons_t *cons;
        const_t *constant;
        builtin_t *bltin;
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
obj *lookup(env *e, obj *k);
void insert(env *e, obj *k, obj *v);
obj *env_set(env *e, obj *k, obj *v);
env *env_cpy(env *e);
void env_delete(env *e);
void env_print(env *e);

obj *mk_num(char *numstr);
obj *mk_int(mpz_t integ);
obj *mk_rat(mpq_t rat);
obj *mk_dbl(mpf_t dbl);
char *num_to_string(obj *o);

obj *mk_sym(char *name);
obj *mk_string(char *str);

obj *mk_const(char *constant);
obj *mk_char(char c);
obj *mk_bool(bool_t type);

obj *mk_builtin(char *name, builtin proc);
obj *mk_lambda(obj *params, obj *body);
obj *mk_cons(obj *car, obj *cdr);

obj *mk_nil(void);
obj *mk_err(char *fmt, ...);

int is_the_empty_list(obj *o);
int is_false(obj *c);
int is_true(obj *c);

int is_pair(obj *o);

int is_num(obj *o);
int is_int(obj *o);
int is_rat(obj *o);
int is_double(obj *o);

int is_symbol(obj *o);
int is_boolean(obj *o);
int is_char(obj *o);
int is_string(obj *o);
int is_builtin(obj *o);
int is_fun(obj *o);
int is_error(obj *o);

char *type_name(obj_t type);

obj *car(obj *o);
obj *cdr(obj *o);

#define caar(obj) car(car(obj))
#define cadr(obj) car(cdr(obj))
#define cdar(obj) cdr(car(obj))
#define cddr(obj) cdr(cdr(obj))
#define caaar(obj) car(car(car(obj)))
#define caadr(obj) car(car(cdr(obj)))
#define cadar(obj) car(cdr(car(obj)))
#define caddr(obj) car(cdr(cdr(obj)))
#define cdaar(obj) cdr(car(car(obj)))
#define cdadr(obj) cdr(car(cdr(obj)))
#define cddar(obj) cdr(cdr(car(obj)))
#define cdddr(obj) cdr(cdr(cdr(obj)))
#define caaaar(obj) car(car(car(car(obj))))
#define caaadr(obj) car(car(car(cdr(obj))))
#define caadar(obj) car(car(cdr(car(obj))))
#define caaddr(obj) car(car(cdr(cdr(obj))))
#define cadaar(obj) car(cdr(car(car(obj))))
#define cadadr(obj) car(cdr(car(cdr(obj))))
#define caddar(obj) car(cdr(cdr(car(obj))))
#define cadddr(obj) car(cdr(cdr(cdr(obj))))
#define cdaaar(obj) cdr(car(car(car(obj))))
#define cdaadr(obj) cdr(car(car(cdr(obj))))
#define cdadar(obj) cdr(car(cdr(car(obj))))
#define cdaddr(obj) cdr(car(cdr(cdr(obj))))
#define cddaar(obj) cdr(cdr(car(car(obj))))
#define cddadr(obj) cdr(cdr(car(cdr(obj))))
#define cdddar(obj) cdr(cdr(cdr(car(obj))))
#define cddddr(obj) cdr(cdr(cdr(cdr(obj))))

obj *cpy_const(obj *o);
obj *copy(obj *o);
void print(obj *o);
void println(obj *o);
void obj_delete(obj *o);

#endif