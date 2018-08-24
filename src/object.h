#ifndef OBJECT_H
#define OBJECT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define MAXSTRLEN 512

typedef struct obj obj;

obj *the_empty_list;
obj *symbol_table;

obj *universe;

obj * true;
obj * false;

obj *quote_sym;
obj *define_sym;
obj *set_sym;
obj *if_sym;
obj *lambda_sym;
obj *begin_sym;

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
} obj_t;

typedef obj *(*builtin)(obj *);

struct obj {
    obj_t type;
    unsigned char mark;
    union {
        long num;
        char *sym;
        char *str;

        int boolean;
        char character;

        struct {
            obj *car;
            obj *cdr;
        };

        struct {
            char *name;
            builtin proc;
        };

        struct {
            obj *env;
            obj *params;
            obj *body;
        };

        char *err;
    };
};

obj *env_new(void);
obj *env_lookup(obj *env, obj *var);
obj *env_insert(obj *env, obj *var, obj *vals);
obj *env_set(obj *env, obj *var, obj *vals);
obj *env_define(obj *env, obj *var, obj *vals);
obj *env_extend(obj *env, obj *vars, obj *vals);

obj *cons(obj *car, obj *cdr);

obj *mk_num_from_str(char *numstr);
obj *mk_num_from_long(long num);
char *num_to_string(obj *object);

obj *mk_sym(char *name);
obj *mk_string(char *str);

obj *mk_char(char c);
obj *mk_bool(int value);

obj *mk_builtin(char *name, builtin proc);
obj *mk_fun(obj *env, obj *params, obj *body);

obj *mk_nil(void);
obj *mk_err(char *fmt, ...);

int is_the_empty_list(obj *object);
int is_false(obj *c);
int is_true(obj *c);

int is_pair(obj *object);

int is_num(obj *object);
int is_rat(obj *object);
int is_double(obj *object);

int is_symbol(obj *object);
int is_boolean(obj *object);
int is_char(obj *object);
int is_string(obj *object);
int is_builtin(obj *object);
int is_fun(obj *object);
int is_error(obj *object);

char *type_name(obj_t type);

int length(obj *list);

obj *car(obj *pair);
obj *cdr(obj *pair);
void set_car(obj *pair, obj *item);
void set_cdr(obj *pair, obj *item);

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

void print(obj *object);
void println(obj *object);

#endif