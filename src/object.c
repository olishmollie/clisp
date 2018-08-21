#include "object.h"
#include "global.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

obj *obj_new(obj_t type) {
    obj *o = malloc(sizeof(obj));
    o->type = type;
    o->mark = 0;
    return o;
}

obj *mk_num_from_str(char *numstr) {
    obj *o = obj_new(OBJ_NUM);
    o->num = strtol(numstr, NULL, 10);
    return o;
}

obj *mk_num_from_long(long num) {
    obj *o = obj_new(OBJ_NUM);
    o->num = num;
    return o;
}

char *num_to_string(obj *o) {
    char *buf = malloc(sizeof(char) * MAXSTRLEN);
    snprintf(buf, MAXSTRLEN - 1, "%li", o->num);
    return buf;
}

obj *mk_sym(char *name) {
    obj *o;
    obj *el;

    el = symbol_table;
    while (!is_the_empty_list(el)) {
        if (strcmp(car(el)->sym, name) == 0) {
            return car(el);
        }
        el = cdr(el);
    }

    o = obj_new(OBJ_SYM);
    o->sym = malloc(sizeof(char) * (strlen(name) + 1));
    strcpy(o->sym, name);
    symbol_table = cons(o, symbol_table);
    return o;
}

obj *mk_string(char *str) {
    obj *o = obj_new(OBJ_STR);
    o->str = malloc(sizeof(char) * (strlen(str) + 1));
    strcpy(o->str, str);
    return o;
}

cons_t *mk_cons_t(obj *car, obj *cdr) {
    cons_t *c = malloc(sizeof(cons_t));
    c->car = car;
    c->cdr = cdr;
    return c;
}

obj *cons(obj *car, obj *cdr) {
    obj *o = obj_new(OBJ_CONS);
    o->cons = mk_cons_t(car, cdr);
    return o;
}

builtin_t *mk_builtin_t(char *name, builtin proc) {
    builtin_t *bltin = malloc(sizeof(builtin_t));
    bltin->name = name;
    bltin->proc = proc;
    return bltin;
}

obj *mk_builtin(char *name, builtin proc) {
    obj *o = obj_new(OBJ_BUILTIN);
    o->bltin = mk_builtin_t(name, proc);
    return o;
}

fun_t *mk_fun_t(char *name, obj *params, obj *body) {
    fun_t *fun = malloc(sizeof(fun_t));
    fun->params = params;
    fun->body = body;
    fun->env = NULL;

    if (name) {
        fun->name = malloc(sizeof(char) * (strlen(name) + 1));
        strcpy(fun->name, name);
    } else {
        fun->name = NULL;
    }

    return fun;
}

obj *mk_fun(obj *env, obj *params, obj *body) {
    obj *o = obj_new(OBJ_FUN);
    o->fun = mk_fun_t(NULL, params, body);
    o->fun->env = env;
    return o;
}

obj *mk_char(char c) {
    obj *o = obj_new(OBJ_CHAR);
    o->character = c;
    return o;
}

obj *mk_bool(int value) {
    obj *o = obj_new(OBJ_BOOL);
    o->boolean = value;
    return o;
}

obj *mk_nil(void) {
    obj *o = obj_new(OBJ_NIL);
    return o;
}

obj *mk_err(char *fmt, ...) {
    obj *o = obj_new(OBJ_ERR);

    va_list args;
    va_start(args, fmt);
    o->err = malloc(sizeof(char) * 512);
    vsnprintf(o->err, 511, fmt, args);
    va_end(args);

    return o;
}

int is_the_empty_list(obj *o) { return o == the_empty_list; }
int is_false(obj *o) { return o == false; }
int is_true(obj *o) { return !is_false(o); }

int is_pair(obj *o) { return o->type == OBJ_CONS; }

int is_num(obj *o) { return o->type == OBJ_NUM; }

int is_symbol(obj *o) { return o->type == OBJ_SYM; }
int is_boolean(obj *o) { return o == true || o == false; }
int is_char(obj *o) { return o->type == OBJ_CHAR; }
int is_string(obj *o) { return o->type == OBJ_STR; }
int is_builtin(obj *o) { return o->type == OBJ_BUILTIN; }
int is_fun(obj *o) { return o->type == OBJ_FUN; }
int is_error(obj *o) { return o->type == OBJ_ERR; }

char *type_name(obj_t type) {
    switch (type) {
    case OBJ_NUM:
        return "number";
    case OBJ_SYM:
        return "symbol";
    case OBJ_STR:
        return "string";
    case OBJ_CONS:
        return "cons";
    case OBJ_BOOL:
        return "boolean";
    case OBJ_CHAR:
        return "character";
    case OBJ_FUN:
        return "function";
    case OBJ_NIL:
        return "nil";
    case OBJ_ERR:
        return "error";
    default:
        return "unknown";
    }
}

int length(obj *o) {
    int l = 0;
    while (!is_the_empty_list(o)) {
        l++;
        o = cdr(o);
    }
    return l;
}

/* list fns ---------------------------------------------------------------- */

obj *car(obj *pair) { return pair->cons->car; }
obj *cdr(obj *pair) { return pair->cons->cdr; }
void set_car(obj *pair, obj *value) { pair->cons->car = value; }
void set_cdr(obj *pair, obj *value) { pair->cons->cdr = value; }

/* environments ------------------------------------------------------------ */

// initial env is ( '()  )
obj *mk_frame(obj *vars, obj *vals) { return cons(vars, vals); }

obj *frame_vars(obj *frame) { return car(frame); }

obj *frame_vals(obj *frame) { return cdr(frame); }

void add_binding_to_frame(obj *var, obj *val, obj *frame) {
    set_car(frame, cons(var, car(frame)));
    set_cdr(frame, cons(val, cdr(frame)));
    if (is_fun(val)) {
        val->fun->name = malloc(sizeof(char) * (strlen(var->sym) + 1));
        strcpy(val->fun->name, var->sym);
    }
}

obj *env_extend(obj *env, obj *vars, obj *vals) {
    return cons(mk_frame(vars, vals), env);
}

obj *env_lookup(obj *env, obj *var) {
    obj *frame, *vars, *vals;
    while (!is_the_empty_list(env)) {
        frame = car(env);
        vars = frame_vars(frame);
        vals = frame_vals(frame);
        while (!is_the_empty_list(vars)) {
            if (car(vars) == var)
                return car(vals);
            vars = cdr(vars);
            vals = cdr(vals);
        }
        env = cdr(env);
    }
    return mk_err("unbound variable '%s'", var->sym);
}

obj *env_set(obj *env, obj *var, obj *val) {
    obj *frame, *vars, *vals;
    while (!is_the_empty_list(env)) {
        frame = car(env);
        vars = frame_vars(frame);
        vals = frame_vals(frame);
        while (!is_the_empty_list(vars)) {
            if (car(vars) == var) {
                set_car(vals, val);
                return NULL;
            }
            vars = cdr(vars);
            vals = cdr(vals);
        }
        env = cdr(env);
    }
    return mk_err("unbound variable '%s'", var->sym);
}

obj *env_define(obj *env, obj *var, obj *val) {
    obj *frame, *vars, *vals;

    frame = car(env);
    vars = frame_vars(frame);
    vals = frame_vals(frame);

    while (!is_the_empty_list(vars)) {
        if (car(vars) == var) {
            set_car(vals, val);
            return NULL;
        }
        vars = cdr(vars);
        vals = cdr(vals);
    }
    add_binding_to_frame(var, val, frame);
    return NULL;
}

obj *env_new(void) {
    obj *env;
    env = env_extend(the_empty_list, the_empty_list, the_empty_list);
    return env;
}

/* printing ---------------------------------------------------------------- */

void print(obj *o);

void print_rawstr(char *str) {
    printf("\"");
    int len = strlen(str);
    for (int i = 0; i < len; i++) {
        switch (str[i]) {
        case '\n':
            printf("\\n");
            break;
        case '\t':
            printf("\\t");
            break;
        case '\f':
            printf("\\f");
            break;
        case '\"':
            printf("\\\"");
            break;
        default:
            printf("%c", str[i]);
        }
    }
    printf("\"");
}

void print_cons(obj *o) {
    putchar('(');
    obj *p = o;
    while (1) {
        print(car(p));
        obj *cdr_obj = cdr(p);
        if (cdr_obj->type != OBJ_CONS) {
            if (cdr_obj->type != OBJ_NIL) {
                printf(" . ");
                print(cdr_obj);
            }
            putchar(')');
            break;
        }
        putchar(' ');
        p = cdr(p);
    }
}

void print(obj *o) {
    if (o) {
        switch (o->type) {
        case OBJ_NUM:
            printf("%li", o->num);
            break;
        case OBJ_SYM:
            printf("%s", o->sym);
            break;
        case OBJ_STR:
            print_rawstr(o->str);
            break;
        case OBJ_CONS:
            print_cons(o);
            break;
        case OBJ_BOOL:
            printf("%s", o->boolean ? "#t" : "#f");
            break;
        case OBJ_CHAR:
            if (o->character == '\n')
                printf("#\\newline");
            else if (o->character == '\t')
                printf("#\\tab");
            else if (o->character == ' ')
                printf("#\\space");
            else {
                printf("#\\%c", o->character);
            }
            break;
        case OBJ_BUILTIN:
            printf("#<procedure '%s'>", o->bltin->name);
            break;
        case OBJ_FUN:
            if (o->fun->name)
                printf("#<procedure '%s'>", o->fun->name);
            else {
                printf("#<procedure>");
            }
            break;
        case OBJ_ERR:
            printf("error: %s", o->err);
            break;
        case OBJ_NIL:
            printf("()");
            break;
        default:
            printf("Cannot print unknown obj type\n");
        }
    }
}

void println(obj *o) {
    print(o);
    putchar('\n');
}

/* deleting ---------------------------------------------------------------- */

void obj_delete(obj *o);

void obj_delete(obj *o) {
    if (o) {
        switch (o->type) {
        case OBJ_SYM:
            free(o->sym);
            break;
        case OBJ_STR:
            free(o->str);
            break;
        case OBJ_CONS:
            obj_delete(car(o));
            obj_delete(cdr(o));
            free(o->cons);
            break;
        case OBJ_ERR:
            free(o->err);
            break;
        case OBJ_FUN:
            free(o->fun->name);
            if (o->fun->params)
                obj_delete(o->fun->params);
            if (o->fun->body)
                obj_delete(o->fun->body);
            free(o->fun);
            break;
        case OBJ_BOOL:
        case OBJ_CHAR:
        case OBJ_NUM:
        case OBJ_NIL:
        case OBJ_BUILTIN:
            break;
        }
        free(o);
    }
}
