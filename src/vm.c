#include "vm.h"

#define INITIAL_GC_THRESHOLD 177
VM *vm_new() {
    VM *vm = malloc(sizeof(VM));
    vm->alloc_list = NULL;
    vm->gc_threshold = INITIAL_GC_THRESHOLD;
    vm->sp = 0;
    vm->obj_count = 0;
    return vm;
}

void push(VM *vm, obj_t *item) { vm->stack[vm->sp++] = item; }
obj_t *pop(VM *vm) { return vm->stack[--vm->sp]; }
void popn(VM *vm, int n) {
    for (int i = 0; i < n; i++) {
        pop(vm);
    }
}

void stack_print(VM *vm) {
    printf("=========================\n");
    for (int i = 0; i < vm->sp; i++) {
        println(vm->stack[i]);
    }
    printf("=========================\n");
}