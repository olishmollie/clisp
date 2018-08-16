#include "builtins.h"
#include "parse.h"
#include "object.h"
#include "eval.h"
#include "gmp.h"
#include "global.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// #define binop(res, x, y, op)                                                   \
//     {                                                                          \
//         switch (x->num->type) {                                                \
//         case NUM_INT:                                                          \
//             mpz_##op(x->num->integ, x->num->integ, y->num->integ);             \
//             break;                                                             \
//         case NUM_RAT:                                                          \
//             mpq_##op(x->num->rat, x->num->rat, y->num->rat);                   \
//             break;                                                             \
//         case NUM_DBL:                                                          \
//             mpf_##op(x->num->dbl, x->num->dbl, y->num->dbl);                   \
//             break;                                                             \
//         case NUM_ERR:                                                          \
//             fprintf(                                                           \
//                 stderr,                                                        \
//                 "warning: trying to perform arithmetic on unknown num type");  \
//         }                                                                      \
//     }

// obj *builtin_plus(obj *args) {
//     ARG_TYPECHECK(args, "plus", OBJ_NUM);
//     if (args == the_empty_list)
//         return mk_err("plus passed no arguments");
// }

// void unary_minus(obj *x) {
//     switch (x->num->type) {
//     case NUM_INT:
//         mpz_neg(x->num->integ, x->num->integ);
//         return;
//     case NUM_RAT:
//         mpq_neg(x->num->rat, x->num->rat);
//         return;
//     case NUM_DBL:
//         mpf_neg(x->num->dbl, x->num->dbl);
//         return;
//     case NUM_ERR:
//         fprintf(stderr, "cannot apply unary minus to error number type");
//         return;
//     }
// }

// obj *builtin_minus(obj *args) {
//     ARG_TYPECHECK(args, "minus", OBJ_NUM);
//     if (args == the_empty_list)
//         return mk_err("minus passed no arguments");

//     obj *x = car(args);
//     args = cdr(args);

//     /* unary minus */
//     if (args == the_empty_list) {
//         printf("unary minus\n");
//         unary_minus(x);
//         return x;
//     }

//     while (args != the_empty_list) {
//         obj *y = car(args);
//         binop(x, y, sub);
//         args = cdr(args);
//     }

//     return x;
// }

// obj *builtin_times(obj *args) {
//     ARG_TYPECHECK(args, "times", OBJ_NUM);
//     if (args == the_empty_list)
//         return mk_err("times passed no arguments");
//     obj *x = car(args);
//     args = cdr(args);
//     while (args != the_empty_list) {
//         obj *y = car(args);
//         binop(x, y, mul);
//         args = cdr(args);
//     }
//     return x;
// }

// obj *builtin_divide(obj *args) {
//     ARG_TYPECHECK(args, "divide", OBJ_NUM);
//     if (args == the_empty_list)
//         return mk_err("divide passed no arguments");
//     obj *x = car(args);
//     args = cdr(args);
//     while (args != the_empty_list) {
//         obj *y = car(args);
//         if (mpz_sgn(y->num->integ) == 0)
//             return mk_err("division by zero");
//         binop(x, y, div);
//         args = cdr(args);
//     }
//     return x;
// }

// obj *builtin_remainder(obj *args) {
//     ARG_TYPECHECK(args, "remainder", OBJ_NUM);
//     if (args == the_empty_list)
//         return mk_err("remainder passed no arguments");

//     obj *x = car(args);
//     if (x->num->type != NUM_INT)
//         return mk_err("cannot perform mod on non-integers");

//     args = cdr(args);
//     while (args != the_empty_list) {

//         obj *y = car(args);

//         if (y->num->type != NUM_INT)
//             return mk_err("cannot perorm mod on non-integers");

//         if (mpz_sgn(y->num->integ) == 0)
//             return mk_err("division by zero");

//         mpz_mod(x->num->integ, x->num->integ, y->num->integ);
//         args = cdr(args);
//     }
//     return x;
// }

// int cmp(obj *x, obj *y) {
//     conv(&x, &y);
//     switch (x->num->type) {
//     case NUM_INT:
//         return mpz_cmp(x->num->integ, y->num->integ);
//     case NUM_RAT:
//         return mpq_cmp(x->num->rat, y->num->rat);
//     case NUM_DBL:
//         return mpf_cmp(x->num->dbl, y->num->dbl);
//     case NUM_ERR:
//         fprintf(stderr, "warning: cannot compare numbers of unknown type");
//         return 0;
//     }
// }

// obj *builtin_gt(obj *args) {
//     ARG_TYPECHECK(args, "gt", OBJ_NUM);
//     ARG_NUMCHECK(args, "gt", 2);

//     obj *x = car(args);
//     obj *y = cadr(args);

//     int res = cmp(x, y);

//     return res > 0 ? true : false;
// }

// obj *builtin_gte(obj *args) {
//     ARG_TYPECHECK(args, "gte", OBJ_NUM);
//     ARG_NUMCHECK(args, "gte", 2);

//     obj *x = car(args);
//     obj *y = cadr(args);

//     int res = cmp(x, y);

//     return res >= 0 ? true : false;
// }

// obj *builtin_lt(obj *args) {
//     ARG_TYPECHECK(args, "lt", OBJ_NUM);
//     ARG_NUMCHECK(args, "lt", 2);

//     obj *x = car(args);
//     obj *y = cadr(args);

//     int res = cmp(x, y);

//     return res < 0 ? true : false;
// }

// obj *builtin_lte(obj *args) {
//     ARG_TYPECHECK(args, "lte", OBJ_NUM);
//     ARG_NUMCHECK(args, "lte", 2);

//     obj *x = car(args);
//     obj *y = cadr(args);

//     int res = cmp(x, y);

//     return res <= 0 ? true : false;
// }

obj *builtin_is_null(obj *args) {
    ARG_NUMCHECK(args, "null?", 1);
    return car(args) == the_empty_list ? true : false;
}

obj *builtin_is_boolean(obj *args) {
    ARG_NUMCHECK(args, "boolean?", 1);
    return is_boolean(car(args)) ? true : false;
}

obj *builtin_is_symbol(obj *args) {
    ARG_NUMCHECK(args, "symbol?", 1);
    return is_symbol(car(args)) ? true : false;
}

obj *builtin_is_num(obj *args) {
    ARG_NUMCHECK(args, "number?", 1);
    return is_num(car(args)) ? true : false;
}

obj *builtin_is_char(obj *args) {
    ARG_NUMCHECK(args, "char?", 1);
    return is_char(car(args)) ? true : false;
}

obj *builtin_is_string(obj *args) {
    ARG_NUMCHECK(args, "string?", 1);
    return is_string(car(args)) ? true : false;
}

obj *builtin_is_pair(obj *args) {
    ARG_NUMCHECK(args, "pair?", 1);
    return is_pair(car(args)) ? true : false;
}

obj *builtin_is_proc(obj *args) {
    ARG_NUMCHECK(args, "proc?", 1);
    return is_builtin(car(args)) ? true : false;
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

obj *builtin_setcar(obj *args) {
    ARG_NUMCHECK(args, "set-car!", 2);
    FIG_ASSERT(is_pair(car(args)), "invalid argument passed to set-car!");
    obj *arg = car(args);
    arg->cons->car = cadr(args);
    return NULL;
}

obj *builtin_setcdr(obj *args) {
    ARG_NUMCHECK(args, "set-cdr!", 2);
    FIG_ASSERT(is_pair(car(args)), "invalid argument passed to set-cdr!");
    obj *arg = car(args);
    arg->cons->cdr = cadr(args);
    return NULL;
}

obj *builtin_list(obj *args) { return args; }

obj *builtin_char_to_int(obj *args) {
    ARG_NUMCHECK(args, "char->int", 1);
    FIG_ASSERT(is_char(car(args)), "invalid argument passed to char->int");
    obj *arg = car(args);
    mpz_t i;
    mpz_init(i);
    mpz_set_ui(i, arg->constant->c);
    return mk_int(i);
}

obj *builtin_int_to_char(obj *args) {
    ARG_NUMCHECK(args, "int->char", 1);
    FIG_ASSERT(is_int(car(args)), "invalid argument passed to int->char");
    unsigned long num = mpz_get_ui(car(args)->num->integ);
    FIG_ASSERT(num >= 0 && num <= 9, "invalid argument passed to int->char");
    return mk_char(num);
}

obj *builtin_number_to_string(obj *args) {
    ARG_NUMCHECK(args, "number->string", 1);
    FIG_ASSERT(is_num(car(args)), "invalid argument passed to number->string");
    obj *arg = car(args);
    return mk_string(num_to_string(arg));
}

obj *builtin_string_to_number(obj *args) {
    ARG_NUMCHECK(args, "string->number", 1);
    FIG_ASSERT(is_string(car(args)),
               "invalid argument passed to string->number")
    obj *arg = car(args);
    return mk_num(arg->str);
}

obj *builtin_symbol_to_string(obj *args) {
    ARG_NUMCHECK(args, "symbol->string", 1);
    FIG_ASSERT(is_symbol(car(args)),
               "invalid argument passed to symbol->string");
    obj *arg = car(args);
    return mk_string(arg->sym);
}

obj *builtin_string_to_symbol(obj *args) {
    ARG_NUMCHECK(args, "symbol->string", 1);
    FIG_ASSERT(is_string(car(args)),
               "invalid argument passed to string->symbol");
    obj *arg = car(args);
    return mk_sym(arg->sym);
}

obj *builtin_is_equal(obj *args) {
    ARG_NUMCHECK(args, "eq?", 2);

    obj *x = car(args);
    obj *y = cadr(args);

    if (x->type != y->type)
        return false;

    switch (x->type) {
    case OBJ_CONST:
        if (x->constant->type == y->constant->type) {
            switch (x->constant->type) {
            case CONST_BOOL:
                return x->constant->bool == y->constant->bool ? true : false;
            case CONST_CHAR:
                return x->constant->c == y->constant->c ? true : false;
            default:
                fprintf(stderr,
                        "warning: cannot compare equality of unknown constant "
                        "type");
                return false;
            }
        }
        return false;
    default:
        return x == y ? true : false;
    }
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
    parser_delete(repl_parser);
    free(input);
    exit(0);
    return NULL;
}
