#include "builtins.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* builtins ---------------------------------------------------------------- */
obj *builtin_plus(env *e, obj *args) {
    CASSERT(args, args->count > 0, "plus passed no arguments");
    TARGCHECK(args, OBJ_NUM);
    obj *x = obj_popcar(&args);
    while (args->count > 0) {
        obj *y = obj_popcar(&args);
        x->num->val += y->num->val;
        obj_delete(y);
    }
    obj_delete(args);
    return x;
}

obj *builtin_minus(env *e, obj *args) {
    CASSERT(args, args->count > 0, "minus passed no arguments");
    TARGCHECK(args, OBJ_NUM);
    obj *x = obj_popcar(&args);
    while (args->count > 0) {
        obj *y = obj_popcar(&args);
        x->num->val -= y->num->val;
        obj_delete(y);
    }
    obj_delete(args);
    return x;
}

obj *builtin_times(env *e, obj *args) {
    CASSERT(args, args->count > 0, "times passed no arguments");
    TARGCHECK(args, OBJ_NUM);
    obj *x = obj_popcar(&args);
    while (args->count > 0) {
        obj *y = obj_popcar(&args);
        x->num->val *= y->num->val;
        obj_delete(y);
    }
    obj_delete(args);
    return x;
}

obj *builtin_divide(env *e, obj *args) {
    CASSERT(args, args->count > 0, "times passed no arguments");
    TARGCHECK(args, OBJ_NUM);
    obj *x = obj_popcar(&args);
    while (args->count > 0) {
        obj *y = obj_popcar(&args);
        if (y->num->val == 0) {
            x = obj_err("division by zero");
            obj_delete(y);
            break;
        }
        x->num->val /= y->num->val;
        obj_delete(y);
    }
    return x;
    obj_delete(args);
}

obj *builtin_remainder(env *e, obj *args) {
    CASSERT(args, args->count > 0, "times passed no arguments");
    obj *x = obj_popcar(&args);
    while (args->count > 0) {
        obj *y = obj_popcar(&args);
        if (y->num->val == 0) {
            x = obj_err("division by zero");
            obj_delete(y);
            break;
        }
        x->num->val %= y->num->val;
        obj_delete(y);
    }
    obj_delete(args);
    return x;
}

obj *builtin_cons(env *e, obj *args) {
    NARGCHECK(args, "cons", 2);
    obj *car = obj_popcar(&args);
    obj *cdr = obj_popcar(&args);
    obj_delete(args);
    return obj_cons(car, cdr);
}

obj *builtin_car(env *e, obj *args) {
    NARGCHECK(args, "car", 1);
    TARGCHECK(args, OBJ_CONS);
    obj *car = obj_popcar(&args);
    obj *res = obj_popcar(&car);
    obj_delete(args);
    obj_delete(car);
    return res;
}

obj *builtin_cdr(env *e, obj *args) {
    NARGCHECK(args, "cdr", 1);
    TARGCHECK(args, OBJ_CONS);
    obj *car = obj_popcar(&args);
    obj *res = obj_popcdr(&car);
    obj_delete(args);
    obj_delete(car);
    return res;
}

obj *builtin_list(env *e, obj *args) { return args; }

obj *builtin_eq(env *e, obj *args) {
    NARGCHECK(args, "eq", 2);
    obj *x = obj_popcar(&args);
    obj *y = obj_popcar(&args);
    CASSERT(args, x->type != OBJ_CONS && y->type != OBJ_CONS,
            "parameters passed to eq must be atomic");

    if (x->type != y->type) {
        obj_delete(x);
        obj_delete(y);
        obj_delete(args);
        return obj_bool(FALSE);
    }

    obj *res;
    switch (x->type) {
    case OBJ_NUM:
        res = x->num->val == y->num->val ? obj_bool(TRUE) : obj_bool(FALSE);
        break;
    case OBJ_SYM:
        res = strcmp(x->sym, y->sym) == 0 ? obj_bool(TRUE) : obj_bool(FALSE);
        break;
    case OBJ_BOOL:
        res = x->bool == y->bool ? obj_bool(TRUE) : obj_bool(FALSE);
        break;
    case OBJ_FUN:
        res = strcmp(x->fun->name, y->fun->name) == 0 ? obj_bool(TRUE)
                                                      : obj_bool(FALSE);
        break;
    case OBJ_NIL:
        res = obj_bool(TRUE);
        break;
    case OBJ_ERR:
        res = obj_err("fuuuuck");
        break;
    default:
        res = obj_err("parameter passed to eq must be atomic");
    }

    obj_delete(x);
    obj_delete(y);
    obj_delete(args);

    return res;
}

obj *builtin_atom(env *e, obj *args) {
    NARGCHECK(args, "atom", 1);
    obj *x = obj_popcar(&args);

    obj *res = x->type != OBJ_CONS ? obj_bool(TRUE) : obj_bool(FALSE);

    obj_delete(x);
    obj_delete(args);

    return res;
}

obj *builtin_exit(env *e, obj *args) {
    NARGCHECK(args, "exit", 0);
    env_delete(e);
    exit(0);
    return NULL;
}