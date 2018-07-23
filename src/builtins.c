#include "builtins.h"

#include <stdio.h>
#include <stdlib.h>

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

obj *builtin_exit(env *e, obj *args) {
    NARGCHECK(args, "exit", 0);
    env_delete(e);
    exit(0);
    return NULL;
}