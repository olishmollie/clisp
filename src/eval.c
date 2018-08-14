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

obj *eval_list(env *e, obj *expr) { return the_empty_list; }

int is_self_evaluating(obj *expr) {
    return is_char(expr) || is_boolean(expr) || is_string(expr) || is_num(expr);
}

obj *eval_assignment(env *e, obj *expr) {
    obj *k = assignment_sym(expr);
    obj *v = eval(e, assignment_val(expr));
    obj *err = env_set(e, k, v);
    return !is_error(err) ? NULL : err;
}

obj *eval_definition(env *e, obj *expr) {
    obj *k = definition_sym(expr);
    obj *v = eval(e, definition_val(expr));
    insert(e, k, v);
    return NULL;
}

int is_if(obj *expr) { return is_tagged_list(expr, if_sym); }

obj *if_condition(obj *expr) { return cadr(expr); }

obj *if_consequent(obj *expr) { return caddr(expr); }

obj *if_alternative(obj *expr) { return cadddr(expr); }

obj *builtin_proc(obj *expr) { return car(expr); }

obj *builtin_args(obj *expr) { return cdr(expr); }

obj *eval_arglist(env *e, obj *arglist) {
    if (arglist == the_empty_list)
        return arglist;
    obj *arg = eval(e, car(arglist));
    return !is_error(arg) ? mk_cons(arg, eval_arglist(e, cdr(arglist))) : arg;
}

obj *eval_procedure(env *e, obj *expr) {
    obj *procedure = eval(e, car(expr));
    if (is_error(procedure))
        return procedure;
    if (is_builtin(procedure)) {
        obj *arguments = eval_arglist(e, cdr(expr));
        if (is_error(arguments))
            return arguments;
        return procedure->bltin->proc(arguments);
    } else {
        return the_empty_list;
    }
}

obj *eval(env *e, obj *expr) {

tailcall:

    FIG_ERRORCHECK(expr);

    if (is_self_evaluating(expr)) {
        return expr;
    } else if (is_quotation(expr)) {
        return text_of_quotation(expr);
    } else if (is_definition(expr)) {
        return eval_definition(e, expr);
    } else if (is_assignment(expr)) {
        return eval_assignment(e, expr);
    } else if (is_if(expr)) {
        expr = is_true(eval(e, if_condition(expr))) ? if_consequent(expr)
                                                    : if_alternative(expr);
        goto tailcall;
    } else if (is_pair(expr)) {
        return eval_procedure(e, expr);
    } else if (expr->type == OBJ_SYM) {
        return lookup(e, expr);
    } else {
        return mk_err("invalid syntax");
    }
}