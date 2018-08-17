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
    return expr == NULL || is_char(expr) || is_boolean(expr) ||
           is_string(expr) || is_num(expr) || is_error(expr);
}

obj *eval_assignment(obj *env, obj *expr) {
    ARG_NUMCHECK(cdr(expr), "set!", 2);
    obj *var = assignment_sym(expr);
    obj *val = eval(env, assignment_val(expr));
    FIG_ERRORCHECK(var);
    return env_set(env, var, val);
}

obj *eval_definition(obj *env, obj *expr) {
    obj *var, *val;
    FIG_ASSERT(length(cdr(expr)) >= 2, "invalid syntax define");

    if (is_pair(cadr(expr))) {
        var = caadr(expr);
        obj *params = cdadr(expr);
        obj *body = cddr(expr);
        val = mk_fun(env, params, body);
    } else {
        var = definition_sym(expr);
        val = eval(env, definition_val(expr));
        FIG_ERRORCHECK(val);
    }

    return env_define(env, var, val);
}

int is_if(obj *expr) { return is_tagged_list(expr, if_sym); }

int is_lambda(obj *expr) { return is_tagged_list(expr, lambda_sym); }

int is_begin(obj *expr) { return is_tagged_list(expr, begin_sym); }

obj *eval_arglist(obj *env, obj *arglist) {
    if (arglist == the_empty_list)
        return arglist;
    obj *arg = eval(env, car(arglist));
    return cons(arg, eval_arglist(env, cdr(arglist)));
}

obj *eval(obj *env, obj *expr) {

tailcall:

    FIG_ERRORCHECK(expr);

    if (is_self_evaluating(expr)) {
        return expr;
    } else if (is_quotation(expr)) {
        return text_of_quotation(expr);
    } else if (expr->type == OBJ_SYM) {
        return env_lookup(env, expr);
    } else if (is_definition(expr)) {
        return eval_definition(env, expr);
    } else if (is_assignment(expr)) {
        return eval_assignment(env, expr);
    } else if (is_lambda(expr)) {
        return mk_fun(env, cadr(expr), cddr(expr));
    } else if (is_begin(expr)) {

        expr = cdr(expr);
        while (!is_the_empty_list(cdr(expr))) {
            obj *cur = eval(env, car(expr));
            FIG_ERRORCHECK(cur);
            expr = cdr(expr);
        }

        expr = car(expr);
        goto tailcall;

    } else if (is_if(expr)) {

        FIG_ASSERT(length(cdr(expr)) == 2 || length(cdr(expr)) == 3,
                   "invalid syntax if");
        obj *cond = eval(env, cadr(expr));
        FIG_ERRORCHECK(cond);

        if (is_true(cond)) {
            expr = caddr(expr);
        } else {
            expr = !is_the_empty_list(cdddr(expr)) ? cadddr(expr) : NULL;
        }

        goto tailcall;

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

            env = env_extend(procedure->fun->env, procedure->fun->params, args);

            expr = cons(begin_sym, procedure->fun->body);
            goto tailcall;
        }
    } else {
        return mk_err("invalid syntax");
    }
}