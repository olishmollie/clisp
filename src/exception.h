#ifndef EXCEPTION_H
#define EXCEPTION_H

#include "common.h"

typedef struct VM VM;

void raise(VM *vm, char *fmt, ...);

#endif