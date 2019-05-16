#ifndef OBJECT_H
#define OBJECT_H

#include "table.h"
#include "vm.h"

#define MAX_STRING_LENGTH 512

typedef enum {
    OBJ_NUM,
    OBJ_SYM,
    OBJ_STR,
    OBJ_PAIR,
    OBJ_VEC,
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
        struct {
            long numer;
            long denom;
        };

        char *sym;
        char *str;

        int boolean;
        char character;

        struct {
            obj_t *car;
            obj_t *cdr;
        };

        struct {
            obj_t **objects;
            int size;
        };

        struct {
            char *bname;
            builtin proc;
        };

        struct {
            int variadic;
            obj_t *fname;
            obj_t *env;
            obj_t *params;
            obj_t *body;
        };

        char *err;
    };
};

typedef struct VM VM;

obj_t *mk_cons(VM *vm, obj_t *car, obj_t *cdr);
obj_t *mk_vec(VM *vm, obj_t **objects, int size);

obj_t *mk_num_from_str(VM *vm, char *str, int is_decimal, int is_fractional);
obj_t *mk_num_from_long(VM *vm, long numer, long denom);
char *num_to_string(obj_t *object);

obj_t *mk_sym(VM *vm, char *bname);
obj_t *mk_string(VM *vm, char *str);

obj_t *mk_char(VM *vm, char c);
obj_t *mk_bool(VM *vm, int value);

obj_t *mk_builtin(VM *vm, char *name, builtin proc);
obj_t *mk_fun(VM *vm, obj_t *env, obj_t *params, obj_t *body);

obj_t *mk_nil(VM *vm);
obj_t *mk_err(VM *vm, char *msg);

obj_t *mk_env(VM *vm);
obj_t *env_lookup(VM *vm, obj_t *env, obj_t *symbol);
obj_t *env_define(VM *vm, obj_t *env, obj_t *symbol, obj_t *value);
obj_t *env_set(VM *vm, obj_t *env, obj_t *symbol, obj_t *value);
obj_t *env_extend(VM *vm, obj_t *env, obj_t *symbols, obj_t *values);

int is_the_empty_list(obj_t *object);
int is_false(obj_t *c);
int is_true(obj_t *c);

int is_pair(obj_t *object);
int is_list(obj_t *object);
int is_vector(obj_t *object);

int is_num(obj_t *object);
int is_integer(obj_t *object);
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

void print(obj_t *object);
void println(obj_t *object);

void obj_delete(obj_t *object);

#endif