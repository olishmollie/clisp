#ifndef BUILTINS_H
#define BUILTINS_H

#include "object.h"
#include "eval.h"
#include "read.h"
#include "assert.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
obj *builtin_is_boolean(obj *args);
obj *builtin_is_symbol(obj *args);
obj *builtin_is_num(obj *args);
obj *builtin_is_char(obj *args);
obj *builtin_is_string(obj *args);
obj *builtin_is_pair(obj *args);
obj *builtin_is_proc(obj *args);
obj *builtin_is_equal(obj *args);

obj *builtin_char_to_int(obj *args);
obj *builtin_int_to_char(obj *args);
obj *builtin_number_to_string(obj *args);
obj *builtin_string_to_number(obj *args);
obj *builtin_symbol_to_string(obj *args);
obj *builtin_string_to_symbol(obj *args);

obj *builtin_cons(obj *args);
obj *builtin_car(obj *args);
obj *builtin_cdr(obj *args);
obj *builtin_list(obj *args);

obj *builtin_setcar(obj *args);
obj *builtin_setcdr(obj *args);

obj *builtin_exit(obj *args);
obj *builtin_load(obj *args);

#endif