#ifndef EVAL_H
#define EVAL_H

#include "object.h"

obj *eval_list(env *, obj *);
obj *eval(env *, obj *);

#endif