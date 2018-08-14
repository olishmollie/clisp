#ifndef BUILTINS_H
#define BUILTINS_H

#include "object.h"

obj *builtin_plus(obj *args);
obj *builtin_minus(obj *args);
obj *builtin_times(obj *args);
obj *builtin_divide(obj *args);
obj *builtin_remainder(obj *args);

obj *builtin_gt(obj *args);
obj *builtin_gte(obj *args);
obj *builtin_lt(obj *args);
obj *builtin_lte(obj *args);

obj *builtin_is_null(obj *args);

obj *builtin_cons(obj *args);
obj *builtin_car(obj *args);
obj *builtin_cdr(obj *args);
obj *builtin_list(obj *args);
obj *builtin_atom(obj *args);
obj *builtin_strtolist(obj *args);
obj *builtin_listtostr(obj *args);
obj *builtin_type(obj *args);
obj *builtin_print(obj *args);
obj *builtin_println(obj *args);
obj *builtin_error(obj *args);
obj *builtin_eval(obj *args);

obj *builtin_exit(obj *args);
obj *builtin_load(obj *args);

#endif