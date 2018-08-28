#include "eval.h"
#include "builtins.h"
#include "init.h"
#include "read.h"

void repl_println(obj_t *object) {
    if (object) {
        printf("=> ");
        println(object);
    }
}

obj_t *interpret(VM *vm, reader *rdr) {
    int sp = vm->sp;
    obj_t *object = eval(vm, universe, read(vm, rdr));
    popn(vm, vm->sp - sp);
    return object;
}

void repl(VM *vm) {

    printf("fig version 0.1\n\n");

    while (1) {
        printf("> ");
        reader *rdr = reader_new(stdin);

        /* if user hits enter with no data */
        int c = getc(stdin);
        if (c == '\n')
            continue;
        ungetc(c, stdin);

        obj_t *object = interpret(vm, rdr);

        repl_println(object);
        reader_delete(rdr);
    }
}

int main(int argc, char **argv) {

    vm = vm_new();

    init();

    if (argc > 1) {
        // obj_t *f = mk_string(vm, argv[1]);
        // builtin_load(f);
    } else {
        repl(vm);
    }

    return 0;
}
