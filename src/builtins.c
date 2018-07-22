#include "builtins.h"

obj *builtin_plus(env *e, obj *args) {
    LASSERT(args, args->sexpr->count > 0, "plus passed no arguments");
    obj *x = obj_pop(args, 0);
    while (args->sexpr->count > 0) {
        obj *y = obj_pop(args, 0);
        x->num->val += y->num->val;
        obj_delete(y);
    }
    return x;
}

obj *builtin_minus(env *e, obj *args) {
    LASSERT(args, args->sexpr->count > 0, "minus passed no arguments");
    obj *x = obj_pop(args, 0);
    while (args->sexpr->count > 0) {
        obj *y = obj_pop(args, 0);
        x->num->val -= y->num->val;
        obj_delete(y);
    }
    return x;
}

obj *builtin_times(env *e, obj *args) {
    LASSERT(args, args->sexpr->count > 0, "times passed no arguments");
    obj *x = obj_pop(args, 0);
    while (args->sexpr->count > 0) {
        obj *y = obj_pop(args, 0);
        x->num->val *= y->num->val;
        obj_delete(y);
    }
    return x;
}

obj *builtin_divide(env *e, obj *args) {
    LASSERT(args, args->sexpr->count > 0, "times passed no arguments");
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
    return x;
}
