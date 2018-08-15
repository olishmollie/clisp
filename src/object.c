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

void init_rat(num_t *n, char *ratstr) {
    char *nstr, *dstr;

    nstr = strtok(ratstr, "/");
    dstr = strtok(NULL, "/");

    mpq_init(n->rat);
    mpz_init_set_str(mpq_numref(n->rat), nstr, 10);
    mpz_init_set_str(mpq_denref(n->rat), dstr, 10);

    mpq_canonicalize(n->rat);
}

num_t *mk_num_t(char *numstr) {
    num_t *n = malloc(sizeof(num_t));
    if (strchr(numstr, '.')) {
        n->type = NUM_DBL;
        mpf_init_set_str(n->dbl, numstr, 10);
    } else if (strchr(numstr, '/')) {
        n->type = NUM_RAT;
        init_rat(n, numstr);
    } else {
        n->type = NUM_INT;
        mpz_init_set_str(n->integ, numstr, 10);
    }

    return n;
}

num_t *mk_int_t(mpz_t integ) {
    num_t *n = malloc(sizeof(num_t));
    n->type = NUM_INT;
    mpz_init_set(n->integ, integ);
    return n;
}

num_t *mk_rat_t(mpq_t rat) {
    num_t *n = malloc(sizeof(num_t));
    n->type = NUM_RAT;
    mpq_init(n->rat);
    mpq_set(n->rat, rat);
    return n;
}

num_t *mk_dbl_t(mpf_t dbl) {
    num_t *n = malloc(sizeof(num_t));
    n->type = NUM_DBL;
    mpf_init_set(n->dbl, dbl);
    return n;
}

obj *mk_num(char *numstr) {
    obj *o = obj_new(OBJ_NUM);
    o->num = mk_num_t(numstr);
    return o;
}

obj *mk_int(mpz_t integ) {
    obj *o = obj_new(OBJ_NUM);
    o->num = mk_int_t(integ);
    return o;
}

obj *mk_rat(mpq_t rat) {
    obj *o = obj_new(OBJ_NUM);
    o->num = mk_rat_t(rat);
    return o;
}

obj *mk_dbl(mpf_t dbl) {
    obj *o = obj_new(OBJ_NUM);
    o->num = mk_dbl_t(dbl);
    return o;
}

char *num_to_string(obj *o) {
    // TODO: assign max string len somewhere
    char *buf = malloc(sizeof(char) * 512);
    switch (o->num->type) {
    case NUM_INT:
        return mpz_get_str(buf, 10, o->num->integ);
    case NUM_RAT:
        return mpq_get_str(buf, 10, o->num->rat);
    case NUM_DBL:
        gmp_snprintf(buf, 511, "%Fg", o->num->dbl);
        return buf;
    default:
        fprintf(stderr, "warning: cannot convert unknown num type to string\n");
        return NULL;
    }
}

obj *mk_sym(char *name) {
    obj *o = obj_new(OBJ_SYM);
    o->sym = malloc(sizeof(char) * (strlen(name) + 1));
    strcpy(o->sym, name);
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

obj *mk_lambda(obj *env, obj *params, obj *body) {
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
int is_int(obj *o) { return is_num(o) && o->num->type == NUM_INT; }
int is_rat(obj *o) { return is_num(o) && o->num->type == NUM_RAT; }
int is_double(obj *o) { return is_num(o) && o->num->type == NUM_DBL; }

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

/* list fns ---------------------------------------------------------------- */

obj *car(obj *pair) { return pair->cons->car; }
obj *cdr(obj *pair) { return pair->cons->cdr; }
void set_car(obj *pair, obj *value) { pair->cons->car = value; }
void set_cdr(obj *pair, obj *value) { pair->cons->cdr = value; }

/* environments ------------------------------------------------------------ */

// initial env is ( '()  )
obj *env_new() {
    obj *empty_frame = mk_cons(the_empty_list, the_empty_list);
    return mk_cons(empty_frame, the_empty_list);
}

obj *env_insert(obj *env, obj *key, obj *value) {
    obj *frame = car(env);
    obj *varlist = mk_cons(key, mk_cons(value, the_empty_list));

    if (is_the_empty_list(car(frame))) {
        set_car(frame, varlist);
    } else {
        while (!is_the_empty_list(cdr(frame))) {
            frame = cdr(frame);
        }
        set_cdr(frame, mk_cons(varlist, the_empty_list));
    }

    /* save fn name */
    if (is_fun(value)) {
        value->fun->name = malloc(sizeof(char) * (strlen(key->sym) + 1));
        strcpy(value->fun->name, key->sym);
    }

    return NULL;
}

obj *env_lookup(obj *env, obj *key) {
    if (is_the_empty_list(env))
        return mk_err("unbound symbol '%s'", key->sym);
    obj *frame = car(env);
    while (frame != the_empty_list) {
        if (strcmp(caar(frame)->sym, key->sym) == 0)
            return cadar(frame);
        frame = cdr(frame);
    }
    return env_lookup(cdr(env), key);
}

obj *env_set(obj *env, obj *key, obj *value) {
    if (is_the_empty_list(env))
        return mk_err("unbound symbol '%s'", key->sym);
    obj *frame = car(env);
    while (frame != the_empty_list) {
        if (caar(frame) == key) {
            set_car(cdr(frame), value);
            return NULL;
        }
        frame = cdr(frame);
    }
    return env_set(cdr(env), key, value);
}

void env_extend(obj *env, obj *parent) {
    while (cdr(env) != the_empty_list) {
        env = cdr(env);
    }
    set_cdr(env, parent);
}

/* printing ---------------------------------------------------------------- */

void print(obj *o);

void print_num(obj *o) {
    switch (o->num->type) {
    case NUM_INT:
        mpz_out_str(stdout, 10, o->num->integ);
        break;
    case NUM_RAT:
        mpq_out_str(stdout, 10, o->num->rat);
        break;
    case NUM_DBL:
        gmp_printf("%Fg", o->num->dbl);
        break;
    default:
        break;
    }
}

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
            print_num(o);
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

void clear_num(obj *o) {
    switch (o->num->type) {
    case NUM_INT:
        mpz_clear(o->num->integ);
        break;
    case NUM_RAT:
        mpq_clear(o->num->rat);
        break;
    case NUM_DBL:
        mpf_clear(o->num->dbl);
        break;
    default:
        fprintf(stderr, "warning: could not clear num of unknown type\n");
        break;
    }
    free(o->num);
}

void obj_delete(obj *o) {
    if (o) {
        // DECR_OBJ(o);
        switch (o->type) {
        case OBJ_NUM:
            clear_num(o);
            break;
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
        case OBJ_NIL:
        case OBJ_BUILTIN:
            break;
        }
        free(o);
    }
}
