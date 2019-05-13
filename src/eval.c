#include "common.h"
#include "eval.h"

int is_tagged_list(obj_t *expr, obj_t *tag) {
    obj_t *car_obj;
    if (is_pair(expr)) {
        car_obj = car(expr);
        return is_symbol(car_obj) && car_obj == tag;
    }
    return 0;
}

int is_quote(obj_t *expr) {
    return is_tagged_list(expr, quote_sym);
}

obj_t *text_of_quotation(obj_t *expr) {
    if (cddr(expr) != the_empty_list) {
        raise(vm, "invalid syntax");
    }
    return cadr(expr);
}

int is_quasiquote(obj_t *expr) {
    return is_tagged_list(expr, quasiquote_sym);
}

int is_unquote(obj_t *expr) {
    return is_tagged_list(expr, unquote_sym);
}

int is_unquote_splicing(obj_t *expr) {
    return is_tagged_list(expr, unquote_splicing_sym);
}

obj_t *eval_unquote(obj_t *env, obj_t *expr) {
    if (!is_the_empty_list(cddr(expr))) {
        raise(vm, "invalid syntax in 'unquote'");
    }

    return eval(vm, env, cadr(expr));
}

obj_t *eval_unquote_splicing(obj_t *env, obj_t *expr) {
    if (!is_the_empty_list(cddr(expr))) {
        raise(vm, "invalid syntax in 'unquote-splice'");
    }

    obj_t *res = eval(vm, env, cadr(expr));

    if (!is_list(res)) {
        raise(vm, "unquote_splicing must result in a list");
    }

    return res;
}

void append(obj_t *dest, obj_t *items) {
    obj_t *tmp = items;
    while (!is_the_empty_list(cdr(tmp))) {
        tmp = cdr(tmp);
    }
    set_cdr(tmp, cdr(dest));
    *dest = *items;
}

obj_t *eval_quasiquote(obj_t *env, obj_t *expr) {
    if (cddr(expr) != the_empty_list) {
        raise(vm, "invalid syntax");
    }

    obj_t *list = cadr(expr);

    while (!is_the_empty_list(list)) {
        obj_t *item = car(list);

        if (is_unquote(item)) {
            obj_t *res = eval_unquote(env, item);
            set_car(list, res);
        } else if (is_unquote_splicing(item)) {
            obj_t *res = eval_unquote_splicing(env, item);
            append(list, res);
        }


        list = cdr(list);
    }

    return cadr(expr);
}

int is_assignment(obj_t *expr) { return is_tagged_list(expr, set_sym); }

obj_t *eval_assignment(VM *vm, obj_t *env, obj_t *expr) {
    ARG_NUMCHECK(vm, cdr(expr), "set!", 2);
    obj_t *var = cadr(expr);
    obj_t *val = eval(vm, env, caddr(expr));
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
    }

    return env_define(vm, env, var, val);
}

int is_if(obj_t *expr) { return is_tagged_list(expr, if_sym); }

int is_lambda(obj_t *expr) { return is_tagged_list(expr, lambda_sym); }

int is_begin(obj_t *expr) { return is_tagged_list(expr, begin_sym); }

int is_top_level_only(obj_t *expr) {
    return is_definition(expr) || is_assignment(expr);
}

int is_callable(obj_t *expr) {
    return expr->type == OBJ_FUN || expr->type == OBJ_BUILTIN;
}

int is_variadic(obj_t *fun) { return fun->variadic; }

obj_t *eval_arglist(VM *vm, obj_t *env, obj_t *arglist) {
    if (arglist == the_empty_list) {
        return arglist;
    }

    obj_t *expr = car(arglist);

    if (is_top_level_only(expr)) {
        raise(vm, "invalid syntax '%s'", car(arglist));
    }

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

    if (is_self_evaluating(expr)) {
        return expr;
    }
    else if (is_quote(expr)) {
        return text_of_quotation(expr);
    }
    else if (expr->type == OBJ_SYM) {
        return env_lookup(vm, env, expr);
    }
    else if (is_quasiquote(expr)) {
        return eval_quasiquote(env, expr);
    }
    else if (is_unquote(expr)) {
        raise(vm, "improper setting for 'unquote'");
    }
    else if (is_unquote_splicing(expr)) {
        raise(vm, "improper context for 'unquote-splicing'");
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
            eval(vm, env, car(expr));
            expr = cdr(expr);
        }
        expr = car(expr);

        goto tailcall;
    }
    else if (is_if(expr)) {
        FIG_ASSERT(vm, length(cdr(expr)) == 2 || length(cdr(expr)) == 3,
                   "invalid syntax if");

        obj_t *condition = cadr(expr);

        if (is_top_level_only(condition)) {
            raise(vm, "invalid syntax %s", car(condition));
        }

        condition = eval(vm, env, condition);

        if (is_true(condition)) {
            expr = caddr(expr);
        } else {
            expr = !is_the_empty_list(cdddr(expr)) ? cadddr(expr) : NULL;
        }

        goto tailcall;
    }
    else if (is_list(expr)) {
        if (is_the_empty_list(expr)) {
            raise(vm, "cannot evaluate the empty list");
        }

        obj_t *procedure = car(expr);
        char *fn_name = procedure->sym;

        procedure = eval(vm, env, procedure);

        FIG_ASSERT(vm, is_callable(procedure), "invalid procedure %s", fn_name);

        obj_t *args = eval_arglist(vm, env, cdr(expr));

        if (is_builtin(procedure)) {
            return procedure->proc(vm, args);
        } else {
            if (!is_variadic(procedure)) {
                FIG_ASSERT(vm, length(procedure->params) == length(args),
                        "incorrect number of arguments to %s", fn_name);
            }

            env = env_extend(vm, procedure->env, procedure->params, args);
            expr = mk_cons(vm, begin_sym, procedure->body);

            goto tailcall;
        }
    }
    else {
        raise(vm, "invalid syntax");
    }

    return NULL; /* unreachable */
}