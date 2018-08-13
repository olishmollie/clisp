#ifndef GC_H
#define GC_H

#include "object.h"

#define STACK_MAX 256

typedef struct stack {
    obj *store[STACK_MAX];
    int size;
} stack;

stack *stack_new(void);

void stack_push(stack *s, obj *item);
obj *stack_pop(stack *s);
obj *stack_peek(stack *s);

#endif