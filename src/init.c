#include "global.h"
#include "builtins.h"

void register_builtin(env *e, builtin fun, char *name) {
    obj *k = mk_sym(name);
    obj *v = mk_builtin(name, fun);
    insert(e, k, v);
}

env *global_env(void) {
    env *e = env_new();
    register_builtin(e, builtin_plus, "+");
    register_builtin(e, builtin_minus, "-");
    register_builtin(e, builtin_times, "*");
    register_builtin(e, builtin_divide, "/");
    register_builtin(e, builtin_remainder, "mod");

    register_builtin(e, builtin_gt, ">");
    register_builtin(e, builtin_gte, ">=");
    register_builtin(e, builtin_lt, "<");
    register_builtin(e, builtin_lte, "<=");

    register_builtin(e, builtin_is_null, "null?");
    register_builtin(e, builtin_is_boolean, "boolean?");
    register_builtin(e, builtin_is_symbol, "symbol?");
    register_builtin(e, builtin_is_num, "number?");
    register_builtin(e, builtin_is_char, "char?");
    register_builtin(e, builtin_is_string, "string?");
    register_builtin(e, builtin_is_pair, "pair?");
    register_builtin(e, builtin_is_proc, "procedure?");
    register_builtin(e, builtin_is_equal, "eq?");

    register_builtin(e, builtin_char_to_int, "char->int");
    register_builtin(e, builtin_int_to_char, "int->char");
    register_builtin(e, builtin_number_to_string, "number->string");
    register_builtin(e, builtin_string_to_number, "string->number");
    register_builtin(e, builtin_symbol_to_string, "symbol->string");
    register_builtin(e, builtin_string_to_symbol, "string->symbol");

    register_builtin(e, builtin_cons, "cons");
    register_builtin(e, builtin_car, "car");
    register_builtin(e, builtin_cdr, "cdr");
    register_builtin(e, builtin_setcar, "set-car!");
    register_builtin(e, builtin_setcdr, "set-cdr!");
    register_builtin(e, builtin_list, "list");

    register_builtin(e, builtin_load, "load");
    register_builtin(e, builtin_exit, "exit");
    return e;
}

void init() {
    true = mk_bool(BOOL_T);
    false = mk_bool(BOOL_F);
    the_empty_list = mk_nil();
    quote_sym = mk_sym("quote");
    define_sym = mk_sym("define");
    set_sym = mk_sym("set!");
    if_sym = mk_sym("if");
    lambda_sym = mk_sym("lambda");
    universe = global_env();
    builtin_load(STDLIB);
}
