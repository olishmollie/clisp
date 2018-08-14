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

    register_builtin(e, builtin_cons, "cons");
    register_builtin(e, builtin_car, "car");
    register_builtin(e, builtin_cdr, "cdr");

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
    universe = global_env();
    builtin_load(STDLIB);
}
