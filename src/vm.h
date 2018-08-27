#ifndef VM_H
#define VM_H

#include "object.h"

#define MAX_STACK_LENGTH 256

typedef struct obj_t obj_t;
typedef struct VM {
    int obj_count;
    int gc_threshold;
    int sp;
    obj_t *alloc_list;
    obj_t *stack[MAX_STACK_LENGTH];
} VM;

VM *vm_new(void);
void push(VM *vm, obj_t *item);
obj_t *pop(VM *vm);
void popn(VM *vm, int n);

void stack_print(VM *vm);

VM *vm;

#endif