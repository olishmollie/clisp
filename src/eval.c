#include "eval.h"
#include "object.h"
#include "global.h"

#include <stdlib.h>
#include <string.h>

int is_tagged_list(obj *expr, obj *tag) {
    obj *car_obj;
    if (is_pair(expr)) {
        car_obj = car(expr);
        return is_symbol(car_obj) && car_obj == tag;
    }
    return 0;
}

int is_quotation(obj *expr) { return is_tagged_list(expr, quote_sym); }

obj *text_of_quotation(obj *expr) {
    if (cddr(expr) != the_empty_list)
        return mk_err("invalid syntax");
    return cadr(expr);
}

int is_definition(obj *expr) { return is_tagged_list(expr, define_sym); }

obj *definition_sym(obj *expr) { return cadr(expr); }

obj *definition_val(obj *expr) { return caddr(expr); }

int is_assignment(obj *expr) { return is_tagged_list(expr, set_sym); }

obj *assignment_sym(obj *expr) { return cadr(expr); }

obj *assignment_val(obj *expr) { return caddr(expr); }

obj *eval_list(obj *e, obj *expr) { return the_empty_list; }

int is_self_evaluating(obj *expr) {
    return is_char(expr) || is_boolean(expr) || is_string(expr) ||
           is_num(expr) || is_error(expr);
}

obj *eval_assignment(obj *e, obj *expr) {
    ARG_NUMCHECK(cdr(expr), "set!", 2);
    obj *k = assignment_sym(expr);
    obj *v = eval(e, assignment_val(expr));
    FIG_ERRORCHECK(v);
    obj *err = env_set(e, k, v);
    return !is_error(err) ? NULL : err;
}

obj *eval_definition(obj *env, obj *expr) {
    ARG_NUMCHECK(cdr(expr), "define", 2);
    obj *key = definition_sym(expr);
    obj *value = eval(env, definition_val(expr));
    FIG_ERRORCHECK(value);
    env_insert(env, key, value);
    return NULL;
}

int is_if(obj *expr) { return is_tagged_list(expr, if_sym); }

obj *if_condition(obj *expr) { return cadr(expr); }

obj *if_consequent(obj *expr) { return caddr(expr); }

obj *if_alternative(obj *expr) { return cadddr(expr); }

int is_lambda(obj *expr) { return is_tagged_list(expr, lambda_sym); }

obj *builtin_proc(obj *expr) { return car(expr); }

obj *builtin_args(obj *expr) { return cdr(expr); }

obj *eval_arglist(obj *e, obj *arglist) {
    if (arglist == the_empty_list)
        return arglist;
    obj *arg = eval(e, car(arglist));
    return !is_error(arg) ? mk_cons(arg, eval_arglist(e, cdr(arglist))) : arg;
}

void bind_arguments(obj *env, obj *params, obj *args) {
    while (params != the_empty_list && args != the_empty_list) {
        env_insert(env, car(params), car(args));
        params = cdr(params);
        args = cdr(args);
    }
}

obj *eval(obj *env, obj *expr) {

tailcall:

    FIG_ERRORCHECK(expr);

    if (is_self_evaluating(expr)) {
        return expr;
    } else if (is_quotation(expr)) {
        return text_of_quotation(expr);
    } else if (is_definition(expr)) {
        return eval_definition(env, expr);
    } else if (is_assignment(expr)) {
        return eval_assignment(env, expr);
    } else if (is_if(expr)) {
        obj *cond = eval(env, if_condition(expr));
        FIG_ERRORCHECK(cond);
        expr = is_true(cond) ? if_consequent(expr) : if_alternative(expr);
        goto tailcall;
    } else if (is_lambda(expr)) {
        return mk_lambda(env, cadr(expr), caddr(expr));
    } else if (is_pair(expr)) {
        obj *procedure = eval(env, car(expr));
        FIG_ERRORCHECK(procedure);
        obj *args = eval_arglist(env, cdr(expr));
        FIG_ERRORCHECK(args);

        if (is_builtin(procedure)) {
            return procedure->bltin->proc(args);
        } else {
            FIG_ASSERT(length(procedure->fun->params) == length(args),
                       "incorrect number of arguments to %s",
                       procedure->fun->name);
            obj *local_env = env_new();
            bind_arguments(local_env, procedure->fun->params, args);
            env_extend(local_env, procedure->fun->env);
            expr = eval(local_env, procedure->fun->body);
            goto tailcall;
        }
    } else if (expr->type == OBJ_SYM) {
        return env_lookup(env, expr);
    } else {
        return mk_err("invalid syntax");
    }
}