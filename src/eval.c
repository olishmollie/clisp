#include "eval.h"

int is_tagged_list(obj_t *expr, obj_t *tag) {
    obj_t *car_obj;
    if (is_pair(expr)) {
        car_obj = car(expr);
        return is_symbol(car_obj) && car_obj == tag;
    }
    return 0;
}

int is_quotation(obj_t *expr) { return is_tagged_list(expr, quote_sym); }

obj_t *text_of_quotation(obj_t *expr) {
    if (cddr(expr) != the_empty_list)
        return mk_err(vm, "invalid syntax");
    return cadr(expr);
}

int is_assignment(obj_t *expr) { return is_tagged_list(expr, set_sym); }

obj_t *eval_assignment(VM *vm, obj_t *env, obj_t *expr) {
    ARG_NUMCHECK(vm, cdr(expr), "set!", 2);
    obj_t *var = cadr(expr);
    obj_t *val = eval(vm, env, caddr(expr));
    FIG_ERRORCHECK(var);
    return env_set(vm, env, var, val);
}

int is_definition(obj_t *expr) { return is_tagged_list(expr, define_sym); }

obj_t *eval_definition(VM *vm, obj_t *env, obj_t *expr) {
    obj_t *var, *val;
    FIG_ASSERT(vm, length(cdr(expr)) >= 2, "invalid syntax define");

    if (is_pair(cadr(expr))) {
        var = caadr(expr);
        obj_t *params = cdadr(expr);
        obj_t *body = cddr(expr);
        val = mk_fun(vm, env, params, body);
    } else {
        var = cadr(expr);
        val = eval(vm, env, caddr(expr));
        FIG_ERRORCHECK(val);
    }

    return env_define(vm, env, var, val);
}

int is_if(obj_t *expr) { return is_tagged_list(expr, if_sym); }

int is_and(obj_t *expr) { return is_tagged_list(expr, and_sym); }

int is_or(obj_t *expr) { return is_tagged_list(expr, or_sym); }

int is_lambda(obj_t *expr) { return is_tagged_list(expr, lambda_sym); }

int is_begin(obj_t *expr) { return is_tagged_list(expr, begin_sym); }

int is_top_level_only(obj_t *expr) {
    return is_definition(expr) || is_assignment(expr);
}

int is_callable(obj_t *expr) {
    return expr->type == OBJ_FUN || expr->type == OBJ_BUILTIN;
}

obj_t *eval_arglist(VM *vm, obj_t *env, obj_t *arglist) {
    if (arglist == the_empty_list)
        return arglist;
    obj_t *expr = car(arglist);
    if (is_top_level_only(expr))
        return mk_err(vm, "invalid syntax %s", car(arglist));
    expr = eval(vm, env, expr);
    mk_cons(vm, expr, eval_arglist(vm, env, cdr(arglist)));
    return pop(vm);
}

int is_self_evaluating(obj_t *expr) {
    return expr == NULL || is_char(expr) || is_boolean(expr) ||
           is_string(expr) || is_num(expr) || is_error(expr);
}

obj_t *eval(VM *vm, obj_t *env, obj_t *expr) {

tailcall:

    FIG_ERRORCHECK(expr);

    if (is_self_evaluating(expr)) {
        return expr;
    }
    else if (is_quotation(expr)) {
        return text_of_quotation(expr);
    }
    else if (expr->type == OBJ_SYM) {
        return env_lookup(vm, env, expr);
    }
    else if (is_definition(expr)) {
        return eval_definition(vm, env, expr);
    }
    else if (is_assignment(expr)) {
        return eval_assignment(vm, env, expr);
    }
    else if (is_lambda(expr)) {
        return mk_fun(vm, env, cadr(expr), cddr(expr));
    }
    else if (is_begin(expr)) {
        expr = cdr(expr);
        while (!is_the_empty_list(cdr(expr))) {
            obj_t *cur = eval(vm, env, car(expr));
            FIG_ERRORCHECK(cur);
            expr = cdr(expr);
        }

        expr = car(expr);
        goto tailcall;
    }
    else if (is_if(expr)) {
        FIG_ASSERT(vm, length(cdr(expr)) == 2 || length(cdr(expr)) == 3,
                   "invalid syntax if");
        obj_t *cond = cadr(expr);
        if (is_top_level_only(cond))
            return mk_err(vm, "invalid syntax %s", car(cond));

        cond = eval(vm, env, cond);
        FIG_ERRORCHECK(cond);

        if (is_true(cond)) {
            expr = caddr(expr);
        } else {
            expr = !is_the_empty_list(cdddr(expr)) ? cadddr(expr) : NULL;
        }

        goto tailcall;
    }
    else if (is_and(expr)) {
        obj_t *args = cdr(expr);
        while (!is_the_empty_list(args)) {
            obj_t *pred = eval(vm, env, car(args));
            if (is_false(pred)) {
                expr = false;
                goto tailcall;
            }
            args = cdr(args);
        }
        expr = true;
        goto tailcall;
    }
    else if (is_or(expr)) {
        obj_t *args = cdr(expr);
        while (!is_the_empty_list(args)) {
            obj_t *pred = eval(vm, env, car(args));
            if (is_true(pred)) {
                expr = true;
                goto tailcall;
            }
            args = cdr(args);
        }
        expr = false;
        goto tailcall;
    }
    else if (is_pair(expr)) {
        obj_t *procedure = eval(vm, env, car(expr));
        FIG_ERRORCHECK(procedure);
        FIG_ASSERT(vm, is_callable(procedure), "invalid procedure");

        obj_t *args = eval_arglist(vm, env, cdr(expr));
        FIG_ERRORCHECK(args);

        if (is_builtin(procedure)) {
            return procedure->proc(vm, args);
        } else {
            FIG_ASSERT(vm, length(procedure->params) == length(args),
                       "incorrect number of arguments to function");

            env = env_extend(vm, procedure->env, procedure->params, args);

            mk_cons(vm, begin_sym, procedure->body);
            expr = pop(vm);
            goto tailcall;
        }
    }
    else {
        return mk_err(vm, "invalid syntax");
    }
}