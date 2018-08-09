#include "object.h"
#include "global.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

env *env_new(void) {
    env *e = malloc(sizeof(env));
    e->count = 0;
    e->parent = NULL;
    e->syms = NULL;
    e->vals = NULL;
    return e;
}

obj *env_lookup(env *e, obj *k) {
    /* search local env */
    for (int i = 0; i < e->count; i++) {
        if (strcmp(e->syms[i], k->sym) == 0)
            return obj_cpy(e->vals[i]);
    }

    /* search parent env */
    if (e->parent)
        return env_lookup(e->parent, k);

    return obj_err("unbound symbol '%s'", k->sym);
}

void env_insert(env *e, obj *k, obj *v) {

    /* Overwrite an exisiting symbol if exists */
    for (int i = 0; i < e->count; i++) {
        if (strcmp(e->syms[i], k->sym) == 0) {
            obj_delete(e->vals[i]);
            e->vals[i] = obj_cpy(v);
            return;
        }
    }

    /* Insert new object */
    ++e->count;
    e->vals = realloc(e->vals, sizeof(obj *) * e->count);
    e->syms = realloc(e->syms, sizeof(char *) * e->count);

    e->vals[e->count - 1] = obj_cpy(v);
    e->syms[e->count - 1] = malloc(sizeof(char) * (strlen(k->sym) + 1));
    strcpy(e->syms[e->count - 1], k->sym);
}

env *cpy_env(env *e) {
    env *res = env_new();
    for (int i = 0; i < e->count; i++) {
        obj *k = obj_sym(e->syms[i]);
        obj *v = e->vals[i];
        env_insert(res, k, v);
        obj_delete(k);
    }
    return res;
}

void env_delete(env *e) {
    if (e) {
        for (int i = 0; i < e->count; i++) {
            free(e->syms[i]);
            obj_delete(e->vals[i]);
        }
        free(e->syms);
        free(e->vals);
        free(e);
    }
}

void obj_println(obj *);

void env_print(env *e) {
    printf("{\n");
    for (int i = 0; i < e->count; i++) {
        printf("\t%s: ", e->syms[i]);
        obj_println(e->vals[i]);
    }
    printf("}\n");
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

num_t *mk_num(char *numstr, token_t ttype) {
    num_t *n = malloc(sizeof(num_t));
    switch (ttype) {
    case TOK_INT:
        n->type = NUM_INT;
        mpz_init_set_str(n->integ, numstr, 10);
        break;
    case TOK_RAT:
        n->type = NUM_RAT;
        init_rat(n, numstr);
        break;
    case TOK_FLOAT:
        n->type = NUM_DBL;
        mpf_init_set_str(n->dbl, numstr, 10);
        break;
    default:
        n->type = NUM_ERR;
        n->err = "cannot make num_t with unknown token type";
        break;
    }
    return n;
}

num_t *mk_int(mpz_t integ) {
    num_t *n = malloc(sizeof(num_t));
    n->type = NUM_INT;
    mpz_init_set(n->integ, integ);
    return n;
}

num_t *mk_rat(mpq_t rat) {
    num_t *n = malloc(sizeof(num_t));
    n->type = NUM_RAT;
    mpq_init(n->rat);
    mpq_set(n->rat, rat);
    return n;
}

num_t *mk_dbl(mpf_t dbl) {
    num_t *n = malloc(sizeof(num_t));
    n->type = NUM_DBL;
    mpf_init_set(n->dbl, dbl);
    return n;
}

cons_t *mk_cons(obj *car, obj *cdr) {
    cons_t *c = malloc(sizeof(cons_t));
    c->car = car;
    c->cdr = cdr;
    return c;
}

obj *obj_nil(void);

fun_t *mk_fun(char *name, builtin bltin, obj *params, obj *body) {
    fun_t *fun = malloc(sizeof(fun_t));
    fun->proc = bltin;
    fun->params = params;
    fun->body = body;

    fun->e = !fun->proc ? env_new() : NULL;

    if (name) {
        fun->name = malloc(sizeof(char) * (strlen(name) + 1));
        strcpy(fun->name, name);
    } else {
        fun->name = NULL;
    }
    return fun;
}

const_t *mk_const(char *c) {
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

obj *obj_num(char *numstr, token_t ttype) {
    obj *o = malloc(sizeof(obj));
    o->type = OBJ_NUM;
    o->num = mk_num(numstr, ttype);
    if (o->type == NUM_ERR) {
        obj *err = obj_err(o->num->err);
        obj_delete(o);
        return err;
    }
    INCR_OBJ(o);
    o->nargs = 0;
    return o;
}

obj *obj_int(mpz_t integ) {
    obj *o = malloc(sizeof(obj));
    o->type = OBJ_NUM;
    o->num = mk_int(integ);
    o->nargs = 0;
    INCR_OBJ(o);
    return o;
}

obj *obj_rat(mpq_t rat) {
    obj *o = malloc(sizeof(obj));
    o->type = OBJ_NUM;
    o->num = mk_rat(rat);
    o->nargs = 0;
    INCR_OBJ(o);
    return o;
}

obj *obj_dbl(mpf_t dbl) {
    obj *o = malloc(sizeof(obj));
    o->type = OBJ_NUM;
    o->num = mk_dbl(dbl);
    o->nargs = 0;
    INCR_OBJ(o);
    return o;
}

obj *obj_sym(char *name) {
    obj *o = malloc(sizeof(obj));
    o->type = OBJ_SYM;
    o->sym = malloc(sizeof(char) * (strlen(name) + 1));
    strcpy(o->sym, name);
    o->nargs = 0;
    INCR_OBJ(o);
    return o;
}

obj *obj_str(char *str) {
    obj *o = malloc(sizeof(obj));
    o->type = OBJ_STR;
    o->str = malloc(sizeof(char) * (strlen(str) + 1));
    strcpy(o->str, str);
    o->nargs = 0;
    INCR_OBJ(o);
    return o;
}

obj *obj_cons(obj *car, obj *cdr) {
    obj *o = malloc(sizeof(obj));
    o->type = OBJ_CONS;
    o->cons = mk_cons(car, cdr);
    o->nargs = 0;
    INCR_OBJ(o);
    return o;
}

obj *obj_builtin(char *name, builtin proc) {
    obj *o = malloc(sizeof(obj));
    o->type = OBJ_FUN;
    o->fun = mk_fun(name, proc, NULL, NULL);
    o->nargs = 0;
    INCR_OBJ(o);
    return o;
}

obj *obj_lambda(obj *params, obj *body) {
    obj *o = malloc(sizeof(obj));
    o->type = OBJ_FUN;
    o->fun = mk_fun(NULL, NULL, params, body);
    o->nargs = 0;
    INCR_OBJ(o);
    return o;
}

obj *obj_const(char *constant) {
    obj *o = malloc(sizeof(obj));
    o->type = OBJ_CONST;

    o->constant = mk_const(constant);
    if (o->constant->type == CONST_ERR) {
        obj *err = obj_err(o->constant->err);
        free(o->constant->err);
        obj_delete(o);
        return err;
    }

    o->nargs = 0;
    INCR_OBJ(o);
    return o;
}

obj *obj_char(char c) {
    char *repr = 0;
    obj *res;
    switch (c) {
    case '\n':
    case '\f':
        res = obj_const("#\\newline");
        break;
    case ' ':
        res = obj_const("#\\space");
        break;
    case '\t':
        res = obj_const("#\\tab");
        break;
    default:
        repr = malloc(sizeof(char) * 4);
        sprintf(repr, "#\\%c", c);
        res = obj_const(repr);
        free(repr);
    }

    return res;
}

obj *obj_bool(bool_t type) {
    obj *res = type == BOOL_T ? obj_const("#t") : obj_const("#f");
    return res;
}

int obj_isfalse(obj *c) {
    return c->type == OBJ_CONST && c->constant->type == CONST_BOOL &&
           c->constant->bool == BOOL_F;
}

int obj_istrue(obj *c) { return !obj_isfalse(c); }

obj *obj_keyword(char *name) {
    obj *o = malloc(sizeof(obj));
    o->type = OBJ_KEYWORD;
    o->keyword = malloc(sizeof(char) * (strlen(name) + 1));
    strcpy(o->keyword, name);
    o->nargs = 0;
    INCR_OBJ(o);
    return o;
}

obj *obj_nil(void) {
    obj *o = malloc(sizeof(obj));
    o->type = OBJ_NIL;
    o->nargs = 0;
    INCR_OBJ(o);
    return o;
}

obj *obj_err(char *fmt, ...) {
    obj *o = malloc(sizeof(obj));
    o->type = OBJ_ERR;

    va_list args;
    va_start(args, fmt);
    o->err = malloc(sizeof(char) * 512);
    vsnprintf(o->err, 511, fmt, args);
    va_end(args);
    o->nargs = 0;
    INCR_OBJ(o);

    return o;
}

int obj_isatom(obj *o) { return o->type != OBJ_CONS && o->type != OBJ_FUN; }

int obj_ispair(obj *o) {
    return o->type == OBJ_CONS && obj_isatom(obj_car(o)) &&
           obj_isatom(obj_cdr(o));
}

char *obj_typename(obj_t type) {
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
    case OBJ_KEYWORD:
        return "keyword";
    case OBJ_NIL:
        return "nil";
    case OBJ_ERR:
        return "error";
    default:
        return "unknown";
    }
}

/* list fns ---------------------------------------------------------------- */

obj *obj_car(obj *o) { return o->cons->car; }

obj *obj_cdr(obj *o) { return o->cons->cdr; }

obj *obj_cadr(obj *o) { return obj_car(obj_cdr(o)); }

obj *obj_popcar(obj **o) {
    obj *car = obj_cpy(obj_car(*o));
    obj *cdr = obj_cpy(obj_cdr(*o));
    int count = (*o)->nargs;
    obj_delete(*o);
    *o = cdr;
    (*o)->nargs = obj_isatom(*o) ? 0 : count - 1;
    return car;
}

/* copying ----------------------------------------------------------------- */

obj *cpy_const(obj *o) {
    char *repr = malloc(sizeof(char) * (strlen(o->constant->repr) + 1));
    strcpy(repr, o->constant->repr);
    obj *c = obj_const(repr);
    free(repr);
    return c;
}

obj *cpy_num(obj *o) {
    switch (o->num->type) {
    case NUM_INT:
        return obj_int(o->num->integ);
    case NUM_RAT:
        return obj_rat(o->num->rat);
    case NUM_DBL:
        return obj_dbl(o->num->dbl);
    default:
        obj_delete(o);
        return obj_err("cannot copy number of unknown type");
    }
}

obj *obj_cpy(obj *o) {
    obj *res;
    switch (o->type) {
    case OBJ_NUM:
        return cpy_num(o);
    case OBJ_SYM:
        res = obj_sym(o->sym);
        break;
    case OBJ_STR:
        res = obj_str(o->str);
        break;
    case OBJ_CONS:
        res = obj_cons(obj_cpy(o->cons->car), obj_cpy(o->cons->cdr));
        break;
    case OBJ_FUN:
        if (o->fun->proc)
            res = obj_builtin(o->fun->name, o->fun->proc);
        else {
            res = obj_lambda(obj_cpy(o->fun->params), obj_cpy(o->fun->body));
            if (o->fun->name) {
                res->fun->name =
                    malloc(sizeof(char) * (strlen(o->fun->name) + 1));
                strcpy(res->fun->name, o->fun->name);
            }
        }
        break;
    case OBJ_ERR:
        res = obj_err(o->err);
        break;
    case OBJ_NIL:
        res = obj_nil();
        break;
    case OBJ_KEYWORD:
        res = obj_keyword(o->keyword);
        break;
    case OBJ_CONST:
        res = cpy_const(o);
    }
    res->nargs = o->nargs;
    return res;
}

/* printing ---------------------------------------------------------------- */

void obj_print(obj *o);

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

// void print_cons(obj *o) {
//     printf("(");
//     obj_print(o->cons->car);
//     printf(" . ");
//     obj_print(o->cons->cdr);
//     printf(")");
// }

void print_cons(obj *o) {
    putchar('(');
    obj *p = o;
    while (1) {
        obj_print(obj_car(p));
        obj *cdr = obj_cdr(p);
        if (cdr->type != OBJ_CONS) {
            if (cdr->type != OBJ_NIL) {
                printf(" . ");
                obj_print(cdr);
            }
            putchar(')');
            break;
        }
        putchar(' ');
        p = obj_cdr(p);
    }
}

void obj_print(obj *o) {
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
        case OBJ_FUN:
            if (o->fun->name)
                printf("<function '%s'>", o->fun->name);
            else {
                printf("<function>");
            }
            break;
        case OBJ_ERR:
            printf("error: %s", o->err);
            break;
        case OBJ_KEYWORD:
            printf("%s", o->keyword);
            break;
        case OBJ_NIL:
            printf("()");
            break;
        default:
            printf("Cannot print unknown obj type\n");
        }
    }
}

void obj_println(obj *o) {
    obj_print(o);
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
        DECR_OBJ(o);
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
            obj_delete(obj_car(o));
            obj_delete(obj_cdr(o));
            free(o->cons);
            break;
        case OBJ_ERR:
            free(o->err);
            break;
        case OBJ_FUN:
            free(o->fun->name);
            env_delete(o->fun->e);
            if (o->fun->params)
                obj_delete(o->fun->params);
            if (o->fun->body)
                obj_delete(o->fun->body);
            free(o->fun);
            break;
        case OBJ_KEYWORD:
            free(o->keyword);
            break;
        case OBJ_CONST:
            free(o->constant->repr);
            free(o->constant);
            break;
        case OBJ_NIL:
            break;
        }
        free(o);
    }
}
