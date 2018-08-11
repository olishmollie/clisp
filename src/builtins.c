#include "builtins.h"
#include "object.h"
#include "eval.h"
#include "conversions.h"
#include "gmp.h"
#include "global.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define binop(x, y, op)                                                        \
    {                                                                          \
        conv(&x, &y);                                                          \
        switch (x->num->type) {                                                \
        case NUM_INT:                                                          \
            mpz_##op(x->num->integ, x->num->integ, y->num->integ);             \
            break;                                                             \
        case NUM_RAT:                                                          \
            mpq_##op(x->num->rat, x->num->rat, y->num->rat);                   \
            break;                                                             \
        case NUM_DBL:                                                          \
            mpf_##op(x->num->dbl, x->num->dbl, y->num->dbl);                   \
            break;                                                             \
        case NUM_ERR:                                                          \
            fprintf(                                                           \
                stderr,                                                        \
                "warning: trying to perform arithmetic on unknown num type");  \
        }                                                                      \
    }

obj *builtin_plus(env *e, obj *args) {
    CASSERT(args, args->nargs > 0, "plus passed no arguments");
    TARGCHECK(args, "plus", OBJ_NUM);
    obj *x = obj_popcar(&args);
    while (args->nargs > 0) {
        obj *y = obj_popcar(&args);
        binop(x, y, add);
        obj_delete(y);
    }
    obj_delete(args);
    return x;
}

void unary_minus(obj *x) {
    switch (x->num->type) {
    case NUM_INT:
        mpz_neg(x->num->integ, x->num->integ);
        return;
    case NUM_RAT:
        mpq_neg(x->num->rat, x->num->rat);
        return;
    case NUM_DBL:
        mpf_neg(x->num->dbl, x->num->dbl);
        return;
    case NUM_ERR:
        fprintf(stderr, "cannot apply unary minus to error number type");
        return;
    }
}

obj *builtin_minus(env *e, obj *args) {
    CASSERT(args, args->nargs > 0, "minus passed no arguments");
    TARGCHECK(args, "minus", OBJ_NUM);

    /* unary minus */
    if (args->nargs == 1) {
        obj *x = obj_popcar(&args);
        unary_minus(x);
        obj_delete(args);
        return x;
    }

    obj *x = obj_popcar(&args);
    while (args->nargs > 0) {
        obj *y = obj_popcar(&args);
        binop(x, y, sub);
        obj_delete(y);
    }
    obj_delete(args);
    return x;
}

obj *builtin_times(env *e, obj *args) {
    CASSERT(args, args->nargs > 0, "times passed no arguments");
    TARGCHECK(args, "times", OBJ_NUM);
    obj *x = obj_popcar(&args);
    while (args->nargs > 0) {
        obj *y = obj_popcar(&args);
        binop(x, y, mul);
        obj_delete(y);
    }
    obj_delete(args);
    return x;
}

obj *builtin_divide(env *e, obj *args) {
    CASSERT(args, args->nargs > 0, "times passed no arguments");
    TARGCHECK(args, "divide", OBJ_NUM);
    obj *x = obj_popcar(&args);
    while (args->nargs > 0) {
        obj *y = obj_popcar(&args);
        if (mpz_sgn(y->num->integ) == 0) {
            obj_delete(x);
            x = obj_err("division by zero");
            obj_delete(y);
            break;
        }
        binop(x, y, div);
        obj_delete(y);
    }
    obj_delete(args);
    return x;
}

obj *builtin_remainder(env *e, obj *args) {
    CASSERT(args, args->nargs > 0, "times passed no arguments");
    TARGCHECK(args, "remainder", OBJ_NUM);
    obj *x = obj_popcar(&args);

    if (x->num->type != NUM_INT) {
        obj_delete(x);
        obj_delete(args);
        return obj_err("cannot perform mod on non-integers");
    }

    while (args->nargs > 0) {

        obj *y = obj_popcar(&args);

        if (y->num->type != NUM_INT) {
            obj_delete(x);
            obj_delete(y);
            obj_delete(args);
            return obj_err("cannot perorm mod on non-integers");
        }

        if (mpz_sgn(y->num->integ) == 0) {
            obj_delete(x);
            x = obj_err("division by zero");
            obj_delete(y);
            break;
        }
        mpz_mod(x->num->integ, x->num->integ, y->num->integ);
        obj_delete(y);
    }
    obj_delete(args);
    return x;
}

int cmp(obj *x, obj *y) {
    conv(&x, &y);
    switch (x->num->type) {
    case NUM_INT:
        return mpz_cmp(x->num->integ, y->num->integ);
    case NUM_RAT:
        return mpq_cmp(x->num->rat, y->num->rat);
    case NUM_DBL:
        return mpf_cmp(x->num->dbl, y->num->dbl);
    case NUM_ERR:
        fprintf(stderr, "warning: cannot compare numbers of unknown type");
        return 0;
    }
}

obj *builtin_gt(env *e, obj *args) {
    CASSERT(args, args->nargs > 0, "gt passed no arguments");
    NARGCHECK(args, "gt", 2);
    TARGCHECK(args, "gt", OBJ_NUM);

    obj *x = obj_popcar(&args);
    obj *y = obj_popcar(&args);

    int res = cmp(x, y);

    obj_delete(x);
    obj_delete(y);
    obj_delete(args);

    return res > 0 ? obj_bool(BOOL_T) : obj_bool(BOOL_F);
}

obj *builtin_gte(env *e, obj *args) {
    CASSERT(args, args->nargs > 0, "gte passed no arguments");
    NARGCHECK(args, "gte", 2);
    TARGCHECK(args, "gte", OBJ_NUM);

    obj *x = obj_popcar(&args);
    obj *y = obj_popcar(&args);

    int res = cmp(x, y);

    obj_delete(x);
    obj_delete(y);
    obj_delete(args);

    return res >= 0 ? obj_bool(BOOL_T) : obj_bool(BOOL_F);
}

obj *builtin_lt(env *e, obj *args) {
    CASSERT(args, args->nargs > 0, "lt passed no arguments");
    NARGCHECK(args, "lt", 2);
    TARGCHECK(args, "lt", OBJ_NUM);

    obj *x = obj_popcar(&args);
    obj *y = obj_popcar(&args);

    int res = cmp(x, y);

    obj_delete(x);
    obj_delete(y);
    obj_delete(args);

    return res < 0 ? obj_bool(BOOL_T) : obj_bool(BOOL_F);
}

obj *builtin_lte(env *e, obj *args) {
    CASSERT(args, args->nargs > 0, "lte passed no arguments");
    NARGCHECK(args, "lte", 2);
    TARGCHECK(args, "lte", OBJ_NUM);

    obj *x = obj_popcar(&args);
    obj *y = obj_popcar(&args);

    int res = cmp(x, y);

    obj_delete(x);
    obj_delete(y);
    obj_delete(args);

    return res <= 0 ? obj_bool(BOOL_T) : obj_bool(BOOL_F);
}

obj *builtin_eq(env *e, obj *args) {
    NARGCHECK(args, "eq", 2);

    obj *x = obj_popcar(&args);
    obj *y = obj_popcar(&args);

    if (!obj_isatom(x) || !obj_isatom(y)) {
        obj *err = obj_err("arguments passed to eq must be atomic");
        obj_delete(x);
        obj_delete(y);
        obj_delete(args);
        return err;
    }

    if (x->type != y->type) {
        obj_delete(x);
        obj_delete(y);
        obj_delete(args);
        return obj_bool(BOOL_F);
    }

    obj *res;
    switch (x->type) {
    case OBJ_NUM:
        res = cmp(x, y) == 0 ? obj_bool(BOOL_T) : obj_bool(BOOL_F);
        break;
    case OBJ_SYM:
        res = strcmp(x->sym, y->sym) == 0 ? obj_bool(BOOL_T) : obj_bool(BOOL_F);
        break;
    case OBJ_STR:
        res = strcmp(x->str, y->str) == 0 ? obj_bool(BOOL_T) : obj_bool(BOOL_F);
        break;
    case OBJ_CONST:
        res = strcmp(x->constant->repr, y->constant->repr) == 0
                  ? obj_bool(BOOL_T)
                  : obj_bool(BOOL_F);
        break;
    case OBJ_NIL:
        res = obj_bool(BOOL_T);
        break;
    case OBJ_KEYWORD:
        res = strcmp(x->keyword, y->keyword) == 0 ? obj_bool(BOOL_T)
                                                  : obj_bool(BOOL_F);
        break;
    default:
        res = obj_err("parameter passed to eq must be atomic, got %s",
                      obj_typename(x->type));
    }

    obj_delete(x);
    obj_delete(y);
    obj_delete(args);

    return res;
}

obj *builtin_cons(env *e, obj *args) {
    NARGCHECK(args, "cons", 2);

    obj *car = obj_popcar(&args);
    obj *cdr = obj_popcar(&args);

    obj *res = obj_cons(car, cdr);
    obj_delete(args);
    return res;
}

obj *builtin_car(env *e, obj *args) {
    NARGCHECK(args, "car", 1);
    if (obj_car(args)->type != OBJ_CONS) {
        obj *err = obj_err("argument to car must be cons, got %s",
                           obj_typename(obj_car(args)->type));
        obj_delete(args);
        return err;
    }

    obj *car = obj_popcar(&args);
    obj *res = obj_popcar(&car);
    obj_delete(args);
    obj_delete(car);
    return res;
}

obj *builtin_cdr(env *e, obj *args) {
    NARGCHECK(args, "cdr", 1);
    if (obj_car(args)->type != OBJ_CONS) {
        obj *err = obj_err("argument to cdr must be a cons, got %s",
                           obj_typename(obj_car(args)->type));
        obj_delete(args);
        return err;
    }

    obj *list = obj_popcar(&args);
    obj *car = obj_popcar(&list);

    obj_delete(args);
    obj_delete(car);
    return list;
}

obj *builtin_atom(env *e, obj *args) {
    NARGCHECK(args, "atom", 1);
    obj *x = obj_popcar(&args);

    obj *res = obj_isatom(x) ? obj_bool(BOOL_T) : obj_bool(BOOL_F);

    obj_delete(x);
    obj_delete(args);

    return res;
}

obj *builtin_strtolist(env *e, obj *args) {
    NARGCHECK(args, "string->list", 1);
    TARGCHECK(args, "string->list", OBJ_STR);

    obj *arg = obj_popcar(&args);

    int len = strlen(arg->str);
    obj *list = obj_nil();
    for (int i = 0; i < len; i++) {
        if (list->nargs) {
            obj *cur = list;
            for (int i = 0; i < list->nargs - 1; i++) {
                cur = obj_cdr(cur);
            }
            cur->cons->cdr = obj_cons(obj_char(arg->str[i]), cur->cons->cdr);
        } else {
            list = obj_cons(obj_char(arg->str[i]), list);
        }
        list->nargs++;
    }

    obj_delete(arg);
    obj_delete(args);

    return list;
}

obj *builtin_listtostr(env *e, obj *args) {
    NARGCHECK(args, "list->string", 1);
    TARGCHECK(args, "list->string", OBJ_CONS);

    obj *arg = obj_popcar(&args);

    char *str = malloc(sizeof(char) * (arg->nargs + 1));

    /* every obj in list must be char type */
    obj *cur = arg;
    for (int i = 0; i < arg->nargs; i++) {
        obj *c = obj_car(cur);
        if (c->type != OBJ_CONST || c->constant->type != CONST_CHAR) {
            obj *err = obj_err("list->string cannot take non character type %s",
                               obj_typename(c->type));
            obj_delete(arg);
            obj_delete(args);
            return err;
        }
        str[i] = c->constant->c;
        cur = obj_cdr(cur);
    }

    obj_delete(arg);
    obj_delete(args);

    return obj_str(str);
}

obj *builtin_type(env *e, obj *args) {
    NARGCHECK(args, "type", 1);
    obj *item = obj_popcar(&args);
    obj *res = obj_str(obj_typename(item->type));
    obj_delete(item);
    obj_delete(args);
    return res;
}

obj *builtin_print(env *e, obj *args) {
    while (args->nargs > 0) {
        obj *item = obj_popcar(&args);
        switch (item->type) {
        case OBJ_STR:
            printf("%s", item->str);
            break;
        default:
            obj_print(item);
        }
        obj_delete(item);
    }

    obj_delete(args);

    return NULL;
}

obj *builtin_println(env *e, obj *args) {
    obj *res = builtin_print(e, args);
    printf("\n");
    return res;
}

obj *builtin_error(env *e, obj *args) {
    CASSERT(args, args->nargs > 0, "error expects at least one argument");
    CASSERT(args, obj_car(args)->type == OBJ_STR,
            "first argument to error must be type string, got %s",
            obj_typename(obj_car(args)->type));

    // TODO: support variadic args
    obj *msg = obj_popcar(&args);
    obj *err = obj_err(msg->str);

    obj_delete(args);
    obj_delete(msg);

    return err;
}

obj *builtin_eval(env *e, obj *args) {
    NARGCHECK(args, "eval", 1);
    obj *expr = eval(e, obj_popcar(&args));
    obj_delete(args);
    return expr;
}
