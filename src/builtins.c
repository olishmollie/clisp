#include "builtins.h"
#include "parse.h"
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

obj *builtin_plus(obj *args) {
    ARG_TYPECHECK(args, "plus", OBJ_NUM);
    if (args == the_empty_list)
        return mk_err("plus passed no arguments");
    obj *x = car(args);
    args = cdr(args);
    while (args != the_empty_list) {
        obj *y = car(args);
        binop(x, y, add);
        args = cdr(args);
    }
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

obj *builtin_minus(obj *args) {
    ARG_TYPECHECK(args, "minus", OBJ_NUM);
    if (args == the_empty_list)
        return mk_err("minus passed no arguments");

    obj *x = car(args);
    args = cdr(args);

    /* unary minus */
    if (args == the_empty_list) {
        unary_minus(x);
        return x;
    }

    while (args != the_empty_list) {
        obj *y = car(args);
        binop(x, y, sub);
        args = cdr(args);
    }

    return x;
}

obj *builtin_times(obj *args) {
    ARG_TYPECHECK(args, "times", OBJ_NUM);
    if (args == the_empty_list)
        return mk_err("times passed no arguments");
    obj *x = car(args);
    args = cdr(args);
    while (args != the_empty_list) {
        obj *y = car(args);
        binop(x, y, mul);
        args = cdr(args);
    }
    return x;
}

obj *builtin_divide(obj *args) {
    ARG_TYPECHECK(args, "divide", OBJ_NUM);
    if (args == the_empty_list)
        return mk_err("divide passed no arguments");
    obj *x = car(args);
    args = cdr(args);
    while (args != the_empty_list) {
        obj *y = car(args);
        if (mpz_sgn(y->num->integ) == 0)
            return mk_err("division by zero");
        binop(x, y, div);
        args = cdr(args);
    }
    return x;
}

obj *builtin_remainder(obj *args) {
    ARG_TYPECHECK(args, "remainder", OBJ_NUM);
    if (args == the_empty_list)
        return mk_err("remainder passed no arguments");

    obj *x = car(args);
    if (x->num->type != NUM_INT)
        return mk_err("cannot perform mod on non-integers");

    args = cdr(args);
    while (args != the_empty_list) {

        obj *y = car(args);

        if (y->num->type != NUM_INT)
            return mk_err("cannot perorm mod on non-integers");

        if (mpz_sgn(y->num->integ) == 0)
            return mk_err("division by zero");

        mpz_mod(x->num->integ, x->num->integ, y->num->integ);
        args = cdr(args);
    }
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

obj *builtin_gt(obj *args) {
    ARG_TYPECHECK(args, "gt", OBJ_NUM);
    ARG_NUMCHECK(args, "gt", 2);

    obj *x = car(args);
    obj *y = cadr(args);

    int res = cmp(x, y);

    return res > 0 ? true : false;
}

obj *builtin_gte(obj *args) {
    ARG_TYPECHECK(args, "gte", OBJ_NUM);
    ARG_NUMCHECK(args, "gte", 2);

    obj *x = car(args);
    obj *y = cadr(args);

    int res = cmp(x, y);

    return res >= 0 ? true : false;
}

obj *builtin_lt(obj *args) {
    ARG_TYPECHECK(args, "lt", OBJ_NUM);
    ARG_NUMCHECK(args, "lt", 2);

    obj *x = car(args);
    obj *y = cadr(args);

    int res = cmp(x, y);

    return res < 0 ? true : false;
}

obj *builtin_lte(obj *args) {
    ARG_TYPECHECK(args, "lte", OBJ_NUM);
    ARG_NUMCHECK(args, "lte", 2);

    obj *x = car(args);
    obj *y = cadr(args);

    int res = cmp(x, y);

    return res <= 0 ? true : false;
}

obj *builtin_is_null(obj *args) {
    return car(args) == the_empty_list ? true : false;
}

obj *builtin_cons(obj *args) {
    ARG_NUMCHECK(args, "cons", 2);
    obj *car_obj = car(args);
    obj *cdr_obj = cadr(args);
    return mk_cons(car_obj, cdr_obj);
}

obj *builtin_car(obj *args) {
    ARG_NUMCHECK(args, "car", 1);
    ARG_TYPECHECK(args, "car", OBJ_CONS);
    return caar(args);
}

obj *builtin_cdr(obj *args) {
    ARG_NUMCHECK(args, "cdr", 1);
    ARG_TYPECHECK(args, "cdr", OBJ_CONS);
    return cdar(args);
}

obj *readfile(char *fname) {

    FILE *infile;
    infile = fopen(fname, "r");

    if (!infile)
        return mk_err("could not open %s", fname);

    parser *p = parser_new(infile);

    while (!feof(infile)) {
        obj *o = eval(universe, read(p));
        if (o && o->type == OBJ_ERR) {
            println(o);
            break;
        }
    }

    parser_delete(p);
    fclose(infile);

    return NULL;
}

obj *builtin_load(obj *args) {
    obj *f = car(args);
    char *filename = f->str;
    obj *res = readfile(filename);
    return res;
}

obj *builtin_exit(obj *args) {
    env_delete(universe);
    parser_delete(repl_parser);
    free(input);
    exit(0);
    return NULL;
}
