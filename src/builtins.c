#include "builtins.h"

#include <stdlib.h>

obj *builtin_plus(env *e, obj *args) {
    LASSERT(args, args->sexpr->count > 0, "plus passed no arguments");
    TYPEASSERT(args, OBJ_NUM);
    obj *x = obj_pop(args, 0);
    while (args->sexpr->count > 0) {
        obj *y = obj_pop(args, 0);
        x->num->val += y->num->val;
        obj_delete(y);
    }
    obj_delete(args);
    return x;
}

obj *builtin_minus(env *e, obj *args) {
    LASSERT(args, args->sexpr->count > 0, "minus passed no arguments");
    TYPEASSERT(args, OBJ_NUM);
    obj *x = obj_pop(args, 0);
    while (args->sexpr->count > 0) {
        obj *y = obj_pop(args, 0);
        x->num->val -= y->num->val;
        obj_delete(y);
    }
    obj_delete(args);
    return x;
}

obj *builtin_times(env *e, obj *args) {
    LASSERT(args, args->sexpr->count > 0, "times passed no arguments");
    TYPEASSERT(args, OBJ_NUM);
    obj *x = obj_pop(args, 0);
    while (args->sexpr->count > 0) {
        obj *y = obj_pop(args, 0);
        x->num->val *= y->num->val;
        obj_delete(y);
    }
    obj_delete(args);
    return x;
}

obj *builtin_divide(env *e, obj *args) {
    LASSERT(args, args->sexpr->count > 0, "times passed no arguments");
    TYPEASSERT(args, OBJ_NUM);
    obj *x = obj_pop(args, 0);
    while (args->sexpr->count > 0) {
        obj *y = obj_pop(args, 0);
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
    LASSERT(args, args->sexpr->count > 0, "times passed no arguments");
    obj *x = obj_pop(args, 0);
    while (args->sexpr->count > 0) {
        obj *y = obj_pop(args, 0);
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
    NUMARGSASSERT(args, "cons", 2);
    obj *car = obj_pop(args, 0);
    obj *cdr = obj_pop(args, 0);
    obj_delete(args);
    return obj_cons(car, cdr);
}

obj *builtin_car(env *e, obj *args) {
    NUMARGSASSERT(args, "car", 1);
    TYPEASSERT(args, OBJ_CONS);
    return obj_take(args, 0)->cons->car;
}

obj *builtin_cdr(env *e, obj *args) {
    NUMARGSASSERT(args, "cdr", 1);
    TYPEASSERT(args, OBJ_CONS);
    return obj_take(args, 0)->cons->cdr;
}

obj *builtin_exit(env *e, obj *args) {
    NUMARGSASSERT(args, "exit", 0);
    env_delete(e);
    obj_delete(args);
    exit(0);
    return NULL;
}