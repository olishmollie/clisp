#include "global.h"
#include "builtins.h"

void register_builtin(obj *env, builtin fun, char *name) {
    obj *var = mk_sym(name);
    obj *fn = mk_builtin(name, fun);
    env_insert(env, var, fn);
}

obj *global_env(void) {
    obj *env = env_new();
    register_builtin(env, builtin_plus, "+");
    register_builtin(env, builtin_minus, "-");
    register_builtin(env, builtin_times, "*");
    register_builtin(env, builtin_divide, "/");
    register_builtin(env, builtin_remainder, "mod");

    register_builtin(env, builtin_gt, ">");
    register_builtin(env, builtin_gte, ">=");
    register_builtin(env, builtin_lt, "<");
    register_builtin(env, builtin_lte, "<=");

    register_builtin(env, builtin_is_null, "null?");
    register_builtin(env, builtin_is_boolean, "boolean?");
    register_builtin(env, builtin_is_symbol, "symbol?");
    register_builtin(env, builtin_is_num, "number?");
    register_builtin(env, builtin_is_char, "char?");
    register_builtin(env, builtin_is_string, "string?");
    register_builtin(env, builtin_is_pair, "pair?");
    register_builtin(env, builtin_is_proc, "procedure?");
    register_builtin(env, builtin_is_equal, "eq?");

    register_builtin(env, builtin_char_to_int, "char->int");
    register_builtin(env, builtin_int_to_char, "int->char");
    register_builtin(env, builtin_number_to_string, "number->string");
    register_builtin(env, builtin_string_to_number, "string->number");
    register_builtin(env, builtin_symbol_to_string, "symbol->string");
    register_builtin(env, builtin_string_to_symbol, "string->symbol");

    register_builtin(env, builtin_cons, "cons");
    register_builtin(env, builtin_car, "car");
    register_builtin(env, builtin_cdr, "cdr");
    register_builtin(env, builtin_setcar, "set-car!");
    register_builtin(env, builtin_setcdr, "set-cdr!");
    register_builtin(env, builtin_list, "list");

    register_builtin(env, builtin_load, "load");
    register_builtin(env, builtin_exit, "exit");

    return env;
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
