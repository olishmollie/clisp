#include "object.h"
#include "global.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

cons_t *mk_cons_t(obj *car, obj *cdr) {
    cons_t *c = malloc(sizeof(cons_t));
    c->car = car;
    c->cdr = cdr;
    return c;
}

builtin_t *mk_builtin_t(char *name, builtin proc) {
    builtin_t *bltin = malloc(sizeof(builtin_t));
    bltin->name = name;
    bltin->proc = proc;
    return bltin;
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

const_t *mk_const_t(char *c) {
    const_t *constant = malloc(sizeof(const_t));

    constant->repr = malloc(sizeof(char) * (strlen(c) + 1));
    strcpy(constant->repr, c);

    if (strcmp("#t", constant->repr) == 0) {
        constant->type = CONST_BOOL;
        constant->bool = BOOL_T;
    } else if (strcmp("#f", constant->repr) == 0) {
        constant->type = CONST_BOOL;
        constant->bool = BOOL_F;
    } else if (strcmp(constant->repr, "#\\space") == 0) {
        constant->type = CONST_CHAR;
        constant->c = ' ';
    } else if (strcmp(constant->repr, "#\\newline") == 0 ||
               strcmp(constant->repr, "#\\linefeed") == 0) {
        constant->type = CONST_CHAR;
        constant->c = '\n';
    } else if (strcmp(constant->repr, "#\\tab") == 0) {
        constant->type = CONST_CHAR;
        constant->c = '\t';
    } else if (constant->repr[1] == '\\' && strlen(constant->repr) == 3) {
        constant->type = CONST_CHAR;
        constant->c = constant->repr[2];
    } else {
        constant->type = CONST_ERR;
        /* add length of "invalid constant", 16 */
        constant->err = malloc(sizeof(char) * (strlen(c) + 1 + 17));
        snprintf(constant->err, 17, "invalid constant %s", c);
    }

    return constant;
}

/* obj types --------------------------------------------------------------- */

obj *obj_new(obj_t type) {
    obj *o = malloc(sizeof(obj));
    o->type = type;
    o->mark = 0;
    // INCR_OBJ(o);
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
    // TODO: assign max string len somewhere
    char *buf = malloc(sizeof(char) * 512);
    snprintf(buf, 511, "%li", o->num);
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
    symbol_table = mk_cons(o, symbol_table);
    return o;
}

obj *mk_string(char *str) {
    obj *o = obj_new(OBJ_STR);
    o->str = malloc(sizeof(char) * (strlen(str) + 1));
    strcpy(o->str, str);
    return o;
}

obj *mk_cons(obj *car, obj *cdr) {
    obj *o = obj_new(OBJ_CONS);
    o->cons = mk_cons_t(car, cdr);
    return o;
}

obj *mk_builtin(char *name, builtin proc) {
    obj *o = obj_new(OBJ_BUILTIN);
    o->bltin = mk_builtin_t(name, proc);
    return o;
}

obj *mk_fun(obj *env, obj *params, obj *body) {
    obj *o = obj_new(OBJ_FUN);
    o->fun = mk_fun_t(NULL, params, body);
    o->fun->env = env;
    return o;
}

obj *mk_const(char *constant) {
    obj *o = obj_new(OBJ_CONST);

    o->constant = mk_const_t(constant);
    if (o->constant->type == CONST_ERR) {
        obj *err = mk_err(o->constant->err);
        free(o->constant->err);
        obj_delete(o);
        return err;
    }

    return o;
}

obj *mk_char(char c) {
    char *repr = 0;
    obj *res;
    switch (c) {
    case '\n':
    case '\f':
        res = mk_const("#\\newline");
        break;
    case ' ':
        res = mk_const("#\\space");
        break;
    case '\t':
        res = mk_const("#\\tab");
        break;
    default:
        repr = malloc(sizeof(char) * 4);
        sprintf(repr, "#\\%c", c);
        res = mk_const(repr);
        free(repr);
    }

    return res;
}

obj *mk_bool(bool_t type) {
    obj *res = type == BOOL_T ? mk_const("#t") : mk_const("#f");
    return res;
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
int is_char(obj *o) { return o->type == OBJ_CONST && !is_boolean(o); }
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
    case OBJ_CONST:
        return "constant";
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
obj *mk_frame(obj *vars, obj *vals) { return mk_cons(vars, vals); }

obj *frame_vars(obj *frame) { return car(frame); }

obj *frame_vals(obj *frame) { return cdr(frame); }

void add_binding_to_frame(obj *var, obj *val, obj *frame) {
    set_car(frame, mk_cons(var, car(frame)));
    set_cdr(frame, mk_cons(val, cdr(frame)));
    if (is_fun(val)) {
        val->fun->name = malloc(sizeof(char) * (strlen(var->sym) + 1));
        strcpy(val->fun->name, var->sym);
    }
}

obj *env_extend(obj *env, obj *vars, obj *vals) {
    return mk_cons(mk_frame(vars, vals), env);
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
        case OBJ_CONST:
            printf("%s", o->constant->repr);
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
        // DECR_OBJ(o);
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
        case OBJ_CONST:
            free(o->constant->repr);
            free(o->constant);
            break;
        case OBJ_NUM:
        case OBJ_NIL:
        case OBJ_BUILTIN:
            break;
        }
        free(o);
    }
}
