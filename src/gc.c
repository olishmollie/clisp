#include "gc.h"
#include "global.h"

stack *stack_new(void) {
    stack *s = malloc(sizeof(stack));
    s->size = 0;
    return s;
}

void stack_push(stack *s, obj *item) {
    if (s->size >= STACK_MAX) {
        fprintf(stderr, "error: stack overflow");
        exit(1);
    }
    s->store[s->size++] = item;
}

obj *stack_pop(stack *s) {
    if (s->size == 0) {
        fprintf(stderr, "error: stack underflow");
        exit(1);
    }
    return s->store[--s->size];
}
