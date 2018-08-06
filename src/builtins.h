#ifndef BUILTINS_H
#define BUILTINS_H

#include "object.h"

obj *builtin_plus(env *e, obj *args);
obj *builtin_minus(env *e, obj *args);
obj *builtin_times(env *e, obj *args);
obj *builtin_divide(env *e, obj *args);
obj *builtin_remainder(env *e, obj *args);
obj *builtin_gt(env *e, obj *args);
obj *builtin_gte(env *e, obj *args);
obj *builtin_lt(env *e, obj *args);
obj *builtin_lte(env *e, obj *args);
obj *builtin_eq(env *e, obj *args);
obj *builtin_cons(env *e, obj *args);
obj *builtin_car(env *e, obj *args);
obj *builtin_cdr(env *e, obj *args);
obj *builtin_list(env *e, obj *args);
obj *builtin_atom(env *e, obj *args);
obj *builtin_strtolist(env *e, obj *args);
obj *builtin_listtostr(env *e, obj *args);
obj *builtin_type(env *e, obj *args);
obj *builtin_print(env *e, obj *args);
obj *builtin_println(env *e, obj *args);
obj *builtin_error(env *e, obj *args);
obj *builtin_eval(env *e, obj *args);

#endif