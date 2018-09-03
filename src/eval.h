#ifndef EVAL_H
#define EVAL_H

#include "object.h"
#include "assert.h"

#include <stdlib.h>
#include <string.h>

typedef struct obj_t obj_t;
typedef struct VM VM;

obj_t *eval(VM *vm, obj_t *env, obj_t *expr);

#endif