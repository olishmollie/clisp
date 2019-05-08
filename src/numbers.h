#ifndef NUMBERS_H
#define NUMBERS_H

#include "object.h"

obj_t *reduce(VM *vm, obj_t *num_);

obj_t *num_add(VM *vm, obj_t *a, obj_t *b);
obj_t *num_sub(VM *vm, obj_t *a, obj_t *b);
obj_t *num_mul(VM *vm, obj_t *a, obj_t *b);
obj_t *num_div(VM *vm, obj_t *a, obj_t *b);
obj_t *num_mod(VM *vm, obj_t *a, obj_t *b);

obj_t *num_gt(VM *vm, obj_t *a, obj_t *b);
obj_t *num_gte(VM *vm, obj_t *a, obj_t *b);
obj_t *num_lt(VM *vm, obj_t *a, obj_t *b);
obj_t *num_lte(VM *vm, obj_t *a, obj_t *b);

obj_t *num_eq(VM *vm, obj_t *a, obj_t *b);

#endif