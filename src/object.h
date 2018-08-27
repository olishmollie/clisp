#ifndef OBJECT_H
#define OBJECT_H

#include "table.h"
#include "env.h"
#include "vm.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define MAXSTRLEN 512

typedef struct env_t env_t;
typedef struct table_t table_t;

env_t *universe;

table_t *symbol_table;

typedef struct obj_t obj_t;

obj_t *the_empty_list;

obj_t * true;
obj_t * false;

obj_t *quote_sym;
obj_t *define_sym;
obj_t *set_sym;
obj_t *if_sym;
obj_t *lambda_sym;
obj_t *begin_sym;

typedef enum { NUM_INT, NUM_RAT, NUM_DBL, NUM_ERR } num_type;

typedef enum {
    OBJ_NUM,
    OBJ_SYM,
    OBJ_STR,
    OBJ_PAIR,
    OBJ_BOOL,
    OBJ_CHAR,
    OBJ_BUILTIN,
    OBJ_FUN,
    OBJ_NIL,
    OBJ_ERR
} object_type;

typedef struct VM VM;

typedef obj_t *(*builtin)(VM *vm, obj_t *object);

struct obj_t {
    object_type type;
    unsigned char marked;
    obj_t *next;
    union {
        long num;
        char *sym;
        char *str;

        int boolean;
        char character;

        struct {
            obj_t *car;
            obj_t *cdr;
        };

        struct {
            char *name;
            builtin proc;
        };

        struct {
            env_t *env;
            obj_t *params;
            obj_t *body;
        };

        char *err;
    };
};

typedef struct VM VM;

obj_t *mk_cons(VM *vm, obj_t *car, obj_t *cdr);

obj_t *mk_num_from_str(VM *vm, char *numstr);
obj_t *mk_num_from_long(VM *vm, long num);
char *num_to_string(obj_t *object);

obj_t *mk_sym(VM *vm, char *name);
obj_t *mk_string(VM *vm, char *str);

obj_t *mk_char(VM *vm, char c);
obj_t *mk_bool(VM *vm, int value);

obj_t *mk_builtin(VM *vm, char *name, builtin proc);
obj_t *mk_fun(VM *vm, env_t *env, obj_t *params, obj_t *body);

obj_t *mk_nil(VM *vm);
obj_t *mk_err(char *fmt, ...);

int is_the_empty_list(obj_t *object);
int is_false(obj_t *c);
int is_true(obj_t *c);

int is_pair(obj_t *object);

int is_num(obj_t *object);
int is_rat(obj_t *object);
int is_double(obj_t *object);

int is_symbol(obj_t *object);
int is_boolean(obj_t *object);
int is_char(obj_t *object);
int is_string(obj_t *object);
int is_builtin(obj_t *object);
int is_fun(obj_t *object);
int is_error(obj_t *object);

char *type_name(object_type type);

int length(obj_t *list);

obj_t *car(obj_t *pair);
obj_t *cdr(obj_t *pair);
void set_car(obj_t *pair, obj_t *item);
void set_cdr(obj_t *pair, obj_t *item);

#define caar(obj_t) car(car(obj_t))
#define cadr(obj_t) car(cdr(obj_t))
#define cdar(obj_t) cdr(car(obj_t))
#define cddr(obj_t) cdr(cdr(obj_t))
#define caaar(obj_t) car(car(car(obj_t)))
#define caadr(obj_t) car(car(cdr(obj_t)))
#define cadar(obj_t) car(cdr(car(obj_t)))
#define caddr(obj_t) car(cdr(cdr(obj_t)))
#define cdaar(obj_t) cdr(car(car(obj_t)))
#define cdadr(obj_t) cdr(car(cdr(obj_t)))
#define cddar(obj_t) cdr(cdr(car(obj_t)))
#define cdddr(obj_t) cdr(cdr(cdr(obj_t)))
#define caaaar(obj_t) car(car(car(car(obj_t))))
#define caaadr(obj_t) car(car(car(cdr(obj_t))))
#define caadar(obj_t) car(car(cdr(car(obj_t))))
#define caaddr(obj_t) car(car(cdr(cdr(obj_t))))
#define cadaar(obj_t) car(cdr(car(car(obj_t))))
#define cadadr(obj_t) car(cdr(car(cdr(obj_t))))
#define caddar(obj_t) car(cdr(cdr(car(obj_t))))
#define cadddr(obj_t) car(cdr(cdr(cdr(obj_t))))
#define cdaaar(obj_t) cdr(car(car(car(obj_t))))
#define cdaadr(obj_t) cdr(car(car(cdr(obj_t))))
#define cdadar(obj_t) cdr(car(cdr(car(obj_t))))
#define cdaddr(obj_t) cdr(car(cdr(cdr(obj_t))))
#define cddaar(obj_t) cdr(cdr(car(car(obj_t))))
#define cddadr(obj_t) cdr(cdr(car(cdr(obj_t))))
#define cdddar(obj_t) cdr(cdr(cdr(car(obj_t))))
#define cddddr(obj_t) cdr(cdr(cdr(cdr(obj_t))))

void print(obj_t *object);
void println(obj_t *object);

#endif