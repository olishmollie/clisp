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

obj *lookup(env *e, obj *k) {
    /* search local env */
    for (int i = 0; i < e->count; i++) {
        if (strcmp(e->syms[i], k->sym) == 0)
            return e->vals[i];
    }

    /* search parent env */
    if (e->parent)
        return lookup(e->parent, k);

    return mk_err("unbound symbol '%s'", k->sym);
}

obj *env_set(env *e, obj *k, obj *v) {
    /* if in current env, overwrite */
    for (int i = 0; i < e->count; i++) {
        if (strcmp(k->sym, e->syms[i]) == 0) {
            e->vals[i] = v;
            return e->vals[i];
        }
    }

    /* return error if not found */
    return mk_err("unbound symbol '%s'", k->sym);
}

void insert(env *e, obj *k, obj *v) {

    /* Overwrite an exisiting symbol if exists */
    for (int i = 0; i < e->count; i++) {
        if (strcmp(e->syms[i], k->sym) == 0) {
            e->vals[i] = v;
            return;
        }
    }

    /* Insert new object */
    ++e->count;
    e->vals = realloc(e->vals, sizeof(obj *) * e->count);
    e->syms = realloc(e->syms, sizeof(char *) * e->count);

    e->vals[e->count - 1] = v;
    e->syms[e->count - 1] = k->sym;
    // e->syms[e->count - 1] = malloc(sizeof(char) * (strlen(k->sym) + 1));
    // strcpy(e->syms[e->count - 1], k->sym);
}

env *env_cpy(env *e) {
    env *res = env_new();
    res->parent = e->parent;
    for (int i = 0; i < e->count; i++) {
        obj *k = mk_sym(e->syms[i]);
        obj *v = e->vals[i];
        insert(res, k, v);
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

void println(obj *);

void env_print(env *e) {
    printf("{\n");
    for (int i = 0; i < e->count; i++) {
        printf("\t%s: ", e->syms[i]);
        println(e->vals[i]);
    }
    printf("}\n");
}

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
    fun->e = NULL;

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
    o->nargs = 0;
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

obj *mk_lambda(obj *params, obj *body) {
    obj *o = obj_new(OBJ_FUN);
    o->fun = mk_fun_t(NULL, params, body);
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

obj *car(obj *o) { return o->cons->car; }
obj *cdr(obj *o) { return o->cons->cdr; }
void set_car(obj *o, obj *v) { o->cons->car = v; }
void set_cdr(obj *o, obj *v) { o->cons->cdr = v; }

/* copying ----------------------------------------------------------------- */

obj *cpy_const(obj *o) {
    char *repr = malloc(sizeof(char) * (strlen(o->constant->repr) + 1));
    strcpy(repr, o->constant->repr);
    obj *c = mk_const(repr);
    free(repr);
    return c;
}

obj *cpy_num(obj *o) {
    switch (o->num->type) {
    case NUM_INT:
        return mk_int(o->num->integ);
    case NUM_RAT:
        return mk_rat(o->num->rat);
    case NUM_DBL:
        return mk_dbl(o->num->dbl);
    default:
        obj_delete(o);
        return mk_err("cannot copy number of unknown type");
    }
}

// obj *copy(obj *o) {
//     obj *res;
//     switch (o->type) {
//     case OBJ_NUM:
//         return cpy_num(o);
//     case OBJ_SYM:
//         res = mk_sym(o->sym);
//         break;
//     case OBJ_STR:
//         res = mk_string(o->str);
//         break;
//     case OBJ_CONS:
//         res = mk_cons(copy(o->cons->car), copy(o->cons->cdr));
//         break;
//     case OBJ_BUILTIN:
//         break;
//     case OBJ_FUN:
//         res = mk_lambda(copy(o->fun->params), copy(o->fun->body));
//         res->fun->e = o->fun->e;
//         res->fun->e->parent = o->fun->e->parent;
//         if (o->fun->name) {
//             res->fun->name = malloc(sizeof(char) * (strlen(o->fun->name) +
//             1)); strcpy(res->fun->name, o->fun->name);
//         }
//         break;
//     case OBJ_ERR:
//         res = mk_err(o->err);
//         break;
//     case OBJ_NIL:
//         res = mk_nil();
//         break;
//     case OBJ_CONST:
//         res = cpy_const(o);
//     }
//     res->nargs = o->nargs;
//     return res;
// }

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

// void print_cons(obj *o) {
//     printf("(");
//     print(o->cons->car);
//     printf(" . ");
//     print(o->cons->cdr);
//     printf(")");
// }

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
            // GC will make this work?
            // env_delete(o->fun->e);
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
