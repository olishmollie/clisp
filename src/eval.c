#include "eval.h"
#include "object.h"
#include "global.h"

#include <string.h>

obj *eval_def(env *e, obj *args) {
    NARGCHECK(args, "define", 2);

    obj *car = obj_car(args);
    CASSERT(args, car->type == OBJ_SYM,
            "first arg to def must be symbol, got %s", obj_typename(car->type));

    obj *k = obj_popcar(&args);
    obj *v = eval(e, obj_popcar(&args));
    env_insert(e, k, v);

    obj_delete(k);
    obj_delete(v);
    obj_delete(args);
    return obj_nil();
}

obj *eval_cond(env *e, obj *args) {

    TARGCHECK(args, "cond", OBJ_CONS);

    while (args->nargs > 0) {
        obj *arg = obj_popcar(&args);

        CASSERT(args, arg->nargs == 2,
                "arguments to cond must themselves have two arguments");

        obj *pred = eval(e, obj_popcar(&arg));

        if (pred->type == OBJ_ERR) {
            obj_delete(arg);
            obj_delete(args);
            return pred;
        }

        if (!obj_isfalse(pred)) {
            obj *res = obj_popcar(&arg);
            obj_delete(pred);
            obj_delete(args);
            return eval(e, res);
        }

        obj_delete(pred);
    }

    obj_delete(args);

    return obj_nil();
}

obj *eval_quote(env *e, obj *args) {
    NARGCHECK(args, "quote", 1);
    obj *quote = obj_popcar(&args);
    obj_delete(args);
    return quote;
}

obj *eval_lambda(env *e, obj *args) {
    // NARGCHECK(args, "lambda", 2);
    CASSERT(args,
            obj_car(args)->type == OBJ_CONS || obj_car(args)->type == OBJ_NIL,
            "first argument should be a list, got %s",
            obj_typename(obj_car(args)->type));

    /* check param list for non-symbols */
    obj *params = obj_popcar(&args);
    obj *cur = params;
    for (int i = 0; i < params->nargs; i++) {
        obj *sym = obj_car(cur);
        CASSERT(args, sym->type == OBJ_SYM,
                "lambda param list must contain only symbols, got %s",
                obj_typename(sym->type));
        cur = obj_cdr(cur);
    }

    obj *res = obj_lambda(params, args);

    return res;
}

obj *eval_keyword(env *e, obj *o) {
    obj *res;
    obj *k = obj_popcar(&o);
    ERRCHECK(o);
    if (strcmp(k->keyword, "quote") == 0)
        res = eval_quote(e, o);
    if (strcmp(k->keyword, "lambda") == 0)
        res = eval_lambda(e, o);
    else if (strcmp(k->keyword, "cond") == 0)
        res = eval_cond(e, o);
    else if (strcmp(k->keyword, "def") == 0) {
        res = eval_def(e, o);
    }
    obj_delete(k);
    return res;
}

obj *eval_call(env *e, obj *f, obj *args) {

    /* check builtin */
    if (f->fun->proc)
        return f->fun->proc(e, args);

    NARGCHECK(args, f->fun->name ? f->fun->name : "lambda",
              f->fun->params->nargs);

    /* bind args to params */
    f->fun->e->parent = e;
    while (f->fun->params->nargs > 0) {
        obj *param = obj_popcar(&f->fun->params);
        obj *arg = obj_popcar(&args);
        env_insert(f->fun->e, param, arg);
        obj_delete(param);
        obj_delete(arg);
    }
    obj_delete(args);

    /* evaluate each expression in lambda body but one */
    while (f->fun->body->nargs > 1) {
        obj *expr = obj_popcar(&f->fun->body);
        obj *res = eval(f->fun->e, expr);
        obj_delete(res);
    }

    /* last expression evaluated is return value */
    obj *expr = obj_popcar(&f->fun->body);
    obj *res = eval(f->fun->e, expr);

    return res;
}

obj *eval_list(env *e, obj *o) {
    /* check for keyword */
    if (obj_car(o)->type == OBJ_KEYWORD) {
        return eval_keyword(e, o);
    }

    /* evaluate all children and check for errors*/
    obj *cur = o;
    for (int i = 0; i < o->nargs; i++) {
        cur->cons->car = eval(e, cur->cons->car);
        cur = obj_cdr(cur);
    }
    ERRCHECK(o);

    /* make sure first object is callable */
    obj *f = obj_car(o);
    CASSERT(o, f->type == OBJ_FUN, "object of type %s is not callable",
            obj_typename(f->type));

    /* pop first object and evaluate */
    f = obj_popcar(&o);
    obj *res = eval_call(e, f, o);

    obj_delete(f);

    return res;
}

obj *eval(env *e, obj *o) {
    if (o->type == OBJ_SYM)
        return env_lookup(e, o);
    if (o->type == OBJ_CONS)
        return eval_list(e, o);
    if (o->type == OBJ_KEYWORD) {
        obj *err = obj_err("invalid syntax %s", o->keyword);
        obj_delete(o);
        return err;
    }
    return o;
}