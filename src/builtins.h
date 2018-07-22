#ifndef _BUILTIN_H
#define _BUILTIN_H

#include "env.h"
#include "errors.h"
#include "object.h"

typedef struct obj obj;
typedef struct env env;

typedef obj *(*builtin)(env *, obj *);

obj *builtin_plus(env *, obj *);
obj *builtin_minus(env *, obj *);
obj *builtin_times(env *, obj *);
obj *builtin_divide(env *, obj *);
obj *builtin_remainder(env *, obj *);

obj *builtin_cons(env *, obj *);
obj *builtin_exit(env *, obj *);

#endif