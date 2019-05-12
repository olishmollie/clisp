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
        return mk_err(vm, "invalid syntax");
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
        return mk_err(vm, "invalid syntax in 'unquote'");
    }

    return eval(vm, env, cadr(expr));
}

obj_t *eval_unquote_splicing(obj_t *env, obj_t *expr) {
    if (!is_the_empty_list(cddr(expr))) {
        return mk_err(vm, "invalid syntax in 'unquote-splicint'");
    }

    obj_t *res = eval(vm, env, cadr(expr));

    if (!is_list(res)) {
        return mk_err(vm, "unquote_splicing must result in a list");
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
        return mk_err(vm, "invalid syntax");
    }

    obj_t *list = cadr(expr);

    while (!is_the_empty_list(list)) {
        obj_t *item = car(list);

        if (is_unquote(item)) {
            obj_t *res = eval_unquote(env, item);

            if (is_error(res)) {
                return res;
            }

            set_car(list, res);
        } else if (is_unquote_splicing(item)) {
            obj_t *res = eval_unquote_splicing(env, item);

            if (is_error(res)) {
                return res;
            }

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

int is_cond(obj_t *expr) { return is_tagged_list(expr, cond_sym); }

int is_else(obj_t *expr) { return is_tagged_list(expr, else_sym); }

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
        return mk_err(vm, "invalid syntax '%s'", car(arglist));
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

    FIG_ERRORCHECK(expr);

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
        return mk_err(vm, "improper setting for unquote");
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

        obj_t *condition = cadr(expr);

        if (is_top_level_only(condition)) {
            return mk_err(vm, "invalid syntax %s", car(condition));
        }

        condition = eval(vm, env, condition);
        FIG_ERRORCHECK(condition);

        if (is_true(condition)) {
            expr = caddr(expr);
        } else {
            expr = !is_the_empty_list(cdddr(expr)) ? cadddr(expr) : NULL;
        }

        goto tailcall;
    }
    else if (is_cond(expr)) {
        FIG_ASSERT(vm, length(cdr(expr)) >= 1, "invalid syntax cond");

        obj_t *clauses = cdr(expr);
        while (!is_the_empty_list(cdr(clauses))) {
            obj_t *clause = car(clauses);
            FIG_ASSERT(vm, length(clause) == 2,
                "clauses in cond must have a predicate and a consequent");

            obj_t *pred = eval(vm, env, car(clause));
            FIG_ERRORCHECK(pred);

            if (is_true(pred)) {
                expr = cadar(clauses);
                goto tailcall;
            }
            clauses = cdr(clauses);
        }

        if (is_else(car(clauses))) {
            expr = cadar(clauses);
            goto tailcall;
        }

        obj_t *clause = car(clauses);
        FIG_ASSERT(vm, length(clause) == 2,
            "clauses in cond must have a predicate and a consequent");

        obj_t *pred = eval(vm, env, caar(clauses));
        FIG_ERRORCHECK(pred);

        if (is_true(pred)) {
            expr = cadar(clauses);
            goto tailcall;
        }

        return NULL;
    }
    else if (is_and(expr)) {
        obj_t *args = cdr(expr);
        while (!is_the_empty_list(args)) {
            obj_t *pred = eval(vm, env, car(args));
            FIG_ERRORCHECK(pred);
            if (is_false(pred))
                return false;
            args = cdr(args);
        }
        return true;
    }
    else if (is_or(expr)) {
        obj_t *args = cdr(expr);
        while (!is_the_empty_list(args)) {
            obj_t *pred = eval(vm, env, car(args));
            FIG_ERRORCHECK(pred);
            if (is_true(pred))
                return true;
            args = cdr(args);
        }
        return false;
    }
    else if (is_list(expr)) {
        if (is_the_empty_list(expr)) {
            return mk_err(vm, "cannot evaluate the empty list");
        }

        obj_t *procedure = car(expr);
        char *fn_name = procedure->sym;

        procedure = eval(vm, env, procedure);

        FIG_ERRORCHECK(procedure);
        FIG_ASSERT(vm, is_callable(procedure), "invalid procedure");

        obj_t *args = eval_arglist(vm, env, cdr(expr));
        FIG_ERRORCHECK(args);

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
        return mk_err(vm, "invalid syntax");
    }
}