#ifndef EVAL_H
#define EVAL_H

#include "object.h"
#include "assert.h"

#include <stdlib.h>
#include <string.h>

obj_t *eval(VM *vm, obj_t *env, obj_t *expr);

#endif