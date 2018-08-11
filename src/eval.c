#include "eval.h"
#include "object.h"
#include "builtins.h"
#include "global.h"

#include <stdlib.h>
#include <string.h>

obj *eval_lambda(env *e, obj *args) {

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

    obj *res = obj_lambda(NULL, params, args);

    return res;
}

obj *eval_def(env *e, obj *args) {
    obj *params = obj_car(args);
    CASSERT(args, params->type == OBJ_SYM || params->type == OBJ_CONS,
            "first arg to define must be symbol or cons, got %s",
            obj_typename(params->type));

    if (params->type == OBJ_SYM) {
        /* typical define, e.g. (define x 100) */
        NARGCHECK(args, "define", 2);

        obj *k = obj_popcar(&args);
        obj *v = eval(e, obj_popcar(&args));

        if (v->type == OBJ_FUN) {
            v->fun->name = malloc(sizeof(char) * (strlen(k->sym) + 1));
            strcpy(v->fun->name, k->sym);
        }

        env_insert(e, k, v);

        obj_delete(k);
        obj_delete(v);
        obj_delete(args);

    } else {
        /* syntactic sugar, e.g. (define (square x) (* x x)) */

        /* first arg to params should be symbol */
        obj *name = obj_car(params);
        CASSERT(args, name->type == OBJ_SYM, "invalid syntax define");

        /* build arguments to lambda */
        params = obj_popcar(&args);
        name = obj_popcar(&params);

        /* join list and set nargs + 1 for param list */
        obj *list = obj_cons(params, args);
        list->nargs = args->nargs + 1;

        /* create and save lambda */
        obj *lambda = eval_lambda(e, list);

        lambda->fun->name = malloc(sizeof(char) * (strlen(name->sym) + 1));
        strcpy(lambda->fun->name, name->sym);

        env_insert(e, name, lambda);

        /* delete unused objects */
        obj_delete(name);
        obj_delete(lambda);
    }

    return NULL;
}

obj *eval_if(env *e, obj *args) {
    NARGCHECK(args, "if", 3);

    obj *arg = obj_popcar(&args);
    obj *pred = eval(e, arg);

    obj *conseq = obj_popcar(&args);
    obj *alt = obj_popcar(&args);

    if (pred->type == OBJ_ERR) {
        obj_delete(conseq);
        obj_delete(alt);
        obj_delete(args);
        return pred;
    }

    obj *res;
    if (obj_istrue(pred)) {
        obj_delete(alt);
        res = eval(e, conseq);
    } else {
        obj_delete(conseq);
        res = eval(e, alt);
    }

    obj_delete(pred);
    obj_delete(args);

    return res;
}

obj *eval_quote(env *e, obj *args) {
    NARGCHECK(args, "quote", 1);
    obj *quote = obj_popcar(&args);
    obj_delete(args);
    return quote;
}

obj *eval_set(env *e, obj *args) {
    NARGCHECK(args, "set!", 2);

    obj *name = obj_popcar(&args);
    obj *res = eval(e, obj_popcar(&args));
    env_set(e, name, res);

    obj_delete(name);
    obj_delete(res);
    obj_delete(args);

    return NULL;
}

obj *eval_keyword(env *e, obj *o) {
    obj *res;
    obj *k = obj_popcar(&o);
    ERRCHECK(o);
    if (strcmp(k->keyword, "quote") == 0)
        res = eval_quote(e, o);
    else if (strcmp(k->keyword, "lambda") == 0)
        res = eval_lambda(e, o);
    else if (strcmp(k->keyword, "if") == 0)
        res = eval_if(e, o);
    else if (strcmp(k->keyword, "define") == 0)
        res = eval_def(e, o);
    else if (strcmp(k->keyword, "set!") == 0)
        res = eval_set(e, o);
    else {
        res = obj_err("invalid syntax %s", k->keyword);
        obj_delete(o);
    }

    obj_delete(k);
    return res;
}

obj *eval_call(env *e, obj *f, obj *args) {
    /* check builtin */
    if (f->fun->proc)
        return f->fun->proc(e, args);

    if (f->fun->params->nargs == 0) {

        /* variadic args */
        f->fun->e->parent = e;
        env_insert(f->fun->e, f->fun->params, args);

    } else {

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
        obj *child = eval(e, cur->cons->car);
        cur->cons->car = child;
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
    if (o->type == OBJ_SYM) {
        obj *res = env_lookup(e, o);
        obj_delete(o);
        return res;
    } else if (o->type == OBJ_CONS) {
        obj *res = eval_list(e, o);
        return res;
    } else if (o->type == OBJ_KEYWORD) {
        obj *res = obj_err("invalid syntax %s", o->keyword);
        obj_delete(o);
        return res;
    }

    return o;
}