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

obj_t *interpret(VM *vm, Reader *rdr) {
    int sp = vm->sp;
    obj_t *object = eval(vm, universe, read(vm, rdr));
    popn(vm, vm->sp - sp);
    return object;
}

void repl(VM *vm) {

    printf("fig version 0.2\n\n");

    while (1) {
        printf("> ");
        Reader *rdr = reader_new(stdin);

        /* Hack. User hits enter with no data */
        int c = getc(stdin);
        if (c == '\n')
            continue;
        ungetc(c, stdin);

        obj_t *object = interpret(vm, rdr);

        int done = reader_eof(rdr);

        repl_println(object);
        reader_delete(rdr);

        if (done) break;
    }
}

int main(int argc, char **argv) {

    init();

    if (argc > 1) {
        readfile(vm, argv[1]);
    } else {
        repl(vm);
    }

    printf("\n");
    return 0;
}
