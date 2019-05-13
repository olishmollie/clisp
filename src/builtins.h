#ifndef BUILTINS_H
#define BUILTINS_H

#include "object.h"
#include "eval.h"
#include "read.h"
#include "assert.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct obj_t obj_t;
typedef struct VM VM;

obj_t *builtin_plus(VM *vm, obj_t *args);
obj_t *builtin_minus(VM *vm, obj_t *args);
obj_t *builtin_times(VM *vm, obj_t *args);
obj_t *builtin_divide(VM *vm, obj_t *args);
obj_t *builtin_remainder(VM *vm, obj_t *args);

obj_t *builtin_gt(VM *vm, obj_t *args);
obj_t *builtin_gte(VM *vm, obj_t *args);
obj_t *builtin_lt(VM *vm, obj_t *args);
obj_t *builtin_lte(VM *vm, obj_t *args);
obj_t *builtin_numeq(VM *vm, obj_t *args);

obj_t *builtin_is_null(VM *vm, obj_t *args);
obj_t *builtin_is_boolean(VM *vm, obj_t *args);
obj_t *builtin_is_symbol(VM *vm, obj_t *args);
obj_t *builtin_is_num(VM *vm, obj_t *args);
obj_t *builtin_is_integer(VM *vm, obj_t *args);
obj_t *builtin_is_char(VM *vm, obj_t *args);
obj_t *builtin_is_string(VM *vm, obj_t *args);
obj_t *builtin_is_pair(VM *vm, obj_t *args);
obj_t *builtin_is_list(VM *vm, obj_t *args);
obj_t *builtin_is_vector(VM *vm, obj_t *args);
obj_t *builtin_is_proc(VM *vm, obj_t *args);
obj_t *builtin_is_equal(VM *vm, obj_t *args);

obj_t *builtin_char_to_int(VM *vm, obj_t *args);
obj_t *builtin_int_to_char(VM *vm, obj_t *args);
obj_t *builtin_number_to_string(VM *vm, obj_t *args);
obj_t *builtin_string_to_number(VM *vm, obj_t *args);
obj_t *builtin_symbol_to_string(VM *vm, obj_t *args);
obj_t *builtin_string_to_symbol(VM *vm, obj_t *args);

obj_t *builtin_cons(VM *vm, obj_t *args);
obj_t *builtin_car(VM *vm, obj_t *args);
obj_t *builtin_cdr(VM *vm, obj_t *args);
obj_t *builtin_list(VM *vm, obj_t *args);
obj_t *builtin_setcar(VM *vm, obj_t *args);
obj_t *builtin_setcdr(VM *vm, obj_t *args);

obj_t *builtin_make_vector(VM *vm, obj_t *args);
obj_t *builtin_vector_length(VM *vm, obj_t *args);
obj_t *builtin_vector_set(VM *vm, obj_t *args);
obj_t *builtin_vector_ref(VM *vm, obj_t *args);

obj_t *builtin_string_append(VM *vm, obj_t *args);

obj_t *builtin_display(VM *vm, obj_t *args);
obj_t *builtin_env(VM *vm, obj_t *args);
obj_t *read_file(VM *vm, char *fname);
obj_t *builtin_load(VM *vm, obj_t *args);

obj_t *builtin_exit(VM *vm, obj_t *args);

#endif