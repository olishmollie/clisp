#include "builtins.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* builtins ---------------------------------------------------------------- */
obj *builtin_plus(env *e, obj *args) {
    CASSERT(args, args->list->count > 0, "plus passed no arguments");
    TARGCHECK(args, "plus", OBJ_NUM);
    obj *x = obj_popcar(args);
    while (args->list->count > 0) {
        obj *y = obj_popcar(args);
        x->num->val += y->num->val;
        obj_delete(y);
    }
    obj_delete(args);
    return x;
}

obj *builtin_minus(env *e, obj *args) {
    CASSERT(args, args->list->count > 0, "minus passed no arguments");
    TARGCHECK(args, "minus", OBJ_NUM);
    obj *x = obj_popcar(args);
    while (args->list->count > 0) {
        obj *y = obj_popcar(args);
        x->num->val -= y->num->val;
        obj_delete(y);
    }
    obj_delete(args);
    return x;
}

obj *builtin_times(env *e, obj *args) {
    CASSERT(args, args->list->count > 0, "times passed no arguments");
    TARGCHECK(args, "times", OBJ_NUM);
    obj *x = obj_popcar(args);
    while (args->list->count > 0) {
        obj *y = obj_popcar(args);
        x->num->val *= y->num->val;
        obj_delete(y);
    }
    obj_delete(args);
    return x;
}

obj *builtin_divide(env *e, obj *args) {
    CASSERT(args, args->list->count > 0, "times passed no arguments");
    TARGCHECK(args, "divide", OBJ_NUM);
    obj *x = obj_popcar(args);
    while (args->list->count > 0) {
        obj *y = obj_popcar(args);
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
    CASSERT(args, args->list->count > 0, "times passed no arguments");
    TARGCHECK(args, "remainder", OBJ_NUM);
    obj *x = obj_popcar(args);
    while (args->list->count > 0) {
        obj *y = obj_popcar(args);
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

    obj *car = obj_popcar(args);
    obj *cdr = obj_popcar(args);

    obj *res;
    if (cdr->type == OBJ_LIST) {
        res = obj_list();
        obj_add(res, car);
        while (cdr->list->count > 0)
            obj_add(res, obj_popcar(cdr));
    } else {
        res = obj_cons(car, cdr);
    }

    obj_delete(args);
    return res;
}

obj *builtin_car(env *e, obj *args) {
    NARGCHECK(args, "car", 1);
    CASSERT(args, args->type == OBJ_LIST || args->type == OBJ_CONS,
            "argument to car must be a cons or a list, got %s",
            obj_typename(args->type));
    obj *car = obj_popcar(args);
    obj *res = obj_popcar(car);
    obj_delete(args);
    obj_delete(car);
    return res;
}

obj *builtin_cdr(env *e, obj *args) {
    NARGCHECK(args, "cdr", 1);
    CASSERT(args, args->type == OBJ_LIST || args->type == OBJ_CONS,
            "argument to cdr must be a cons or a list, got %s",
            obj_typename(args->type));
    obj *car = obj_popcar(args);
    obj *res = obj_popcdr(car);
    obj_delete(args);
    obj_delete(car);
    return res;
}

obj *builtin_list(env *e, obj *args) { return args; }

obj *builtin_eq(env *e, obj *args) {
    NARGCHECK(args, "eq", 2);
    obj *x = obj_popcar(args);
    obj *y = obj_popcar(args);
    CASSERT(args, x->type != OBJ_CONS && y->type != OBJ_LIST,
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
    case OBJ_BUILTIN:
        res = strcmp(x->bltin->name, y->bltin->name) == 0 ? obj_bool(TRUE)
                                                          : obj_bool(FALSE);
        break;
    case OBJ_NIL:
        res = obj_bool(TRUE);
        break;
    case OBJ_KEYWORD:
        res = strcmp(x->keyword, y->keyword) == 0 ? obj_bool(TRUE)
                                                  : obj_bool(FALSE);
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

obj *builtin_atom(env *e, obj *args) {
    NARGCHECK(args, "atom", 1);
    obj *x = obj_popcar(args);

    obj *res = x->type != OBJ_CONS && x->type != OBJ_LIST ? obj_bool(TRUE)
                                                          : obj_bool(FALSE);

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