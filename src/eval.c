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

    obj *res = obj_lambda(params, args);

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

obj *eval_cond(env *e, obj *args) {
    CASSERT(args, args->nargs > 0, "invalid syntax cond");
    TARGCHECK(args, "cond", OBJ_CONS);

    while (args->nargs > 1) {

        CASSERT(args, obj_car(args)->nargs == 2,
                "arguments to cond must themselves have two arguments");

        obj *arg = obj_popcar(&args);

        obj *pred = eval(e, obj_popcar(&arg));

        if (pred->type == OBJ_ERR) {
            obj_delete(arg);
            obj_delete(args);
            return pred;
        }

        if (obj_istrue(pred)) {
            obj *res = obj_popcar(&arg);
            obj_delete(pred);
            obj_delete(arg);
            obj_delete(args);
            return eval(e, res);
        }

        obj_delete(pred);
        obj_delete(arg);
    }

    CASSERT(args, obj_car(args)->nargs == 2,
            "arguments to cond must themselves have two arguments");

    obj *arg = obj_popcar(&args);
    obj *maybe_else = obj_car(arg);

    obj *res = NULL;
    if (maybe_else->type == OBJ_KEYWORD &&
        strcmp(maybe_else->keyword, "else") == 0) {
        maybe_else = obj_popcar(&arg);
        obj_delete(maybe_else);
        res = eval(e, obj_popcar(&arg));
    } else {
        obj *pred = eval(e, obj_popcar(&arg));
        if (obj_istrue(pred)) {
            res = eval(e, obj_popcar(&arg));
        }
        obj_delete(pred);
    }

    obj_delete(arg);
    obj_delete(args);

    return res;
}

obj *eval_let(env *e, obj *args) {
    NARGCHECK(args, "let", 2);

    obj *params = obj_popcar(&args);
    env *local = env_new();
    local->parent = e;

    while (params->nargs > 0) {
        obj *pair = obj_popcar(&params);

        obj *k = obj_popcar(&pair);
        if (k->type != OBJ_SYM) {
            obj *err = obj_err("let binding must be symbol, got %s",
                               obj_typename(k->type));
            obj_delete(k);
            obj_delete(pair);
            obj_delete(params);
            obj_delete(args);
            return err;
        }

        obj *v = eval(e, obj_popcar(&pair));
        env_insert(local, k, v);

        obj_delete(k);
        obj_delete(v);
        obj_delete(pair);
    }

    obj *body = obj_popcar(&args);
    obj *res = eval(local, body);

    obj_delete(params);
    obj_delete(args);
    env_delete(local);

    return res;
}

obj *eval_quote(env *e, obj *args) {
    NARGCHECK(args, "quote", 1);
    obj *quote = obj_popcar(&args);
    obj_delete(args);
    return quote;
}

obj *eval_and(env *e, obj *args) {

    while (args->nargs > 0) {
        obj *pred = eval(e, obj_popcar(&args));
        if (obj_isfalse(pred)) {
            obj_delete(args);
            return pred;
        }
        obj_delete(pred);
    }

    obj_delete(args);

    return obj_bool(BOOL_T);
}

obj *eval_or(env *e, obj *args) {

    while (args->nargs > 0) {
        obj *pred = eval(e, obj_popcar(&args));
        if (obj_istrue(pred)) {
            obj_delete(args);
            return pred;
        }
        obj_delete(pred);
    }

    obj_delete(args);

    return obj_bool(BOOL_F);
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

obj *eval_begin(env *e, obj *args) {

    while (args->nargs > 1) {
        obj *arg = eval(e, obj_popcar(&args));
        obj_delete(arg);
    }

    obj *res = eval(e, obj_popcar(&args));
    obj_delete(args);

    return res;
}

obj *eval_keyword(env *e, obj *o) {
    obj *res;
    obj *k = obj_popcar(&o);
    ERRCHECK(o);
    if (strcmp(k->keyword, "quote") == 0)
        res = eval_quote(e, o);
    else if (strcmp(k->keyword, "lambda") == 0)
        res = eval_lambda(e, o);
    else if (strcmp(k->keyword, "let") == 0)
        res = eval_let(e, o);
    else if (strcmp(k->keyword, "cond") == 0)
        res = eval_cond(e, o);
    else if (strcmp(k->keyword, "if") == 0)
        res = eval_if(e, o);
    else if (strcmp(k->keyword, "define") == 0)
        res = eval_def(e, o);
    else if (strcmp(k->keyword, "and") == 0)
        res = eval_and(e, o);
    else if (strcmp(k->keyword, "or") == 0)
        res = eval_or(e, o);
    else if (strcmp(k->keyword, "set!") == 0)
        res = eval_set(e, o);
    else if (strcmp(k->keyword, "begin") == 0)
        res = eval_begin(e, o);
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