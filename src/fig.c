#include "common.h"
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

void repl(VM *vm) {

    printf("fig version "VERSION"\n\n");

    Reader *rdr = reader_new(stdin);

    while (1) {

        int done = 0;

        /* an exception has been raised */
        if (setjmp(exc_env)) {

            println(exc);

        } else {

            printf("> ");

            /* Hack. User hits enter with no data */
            int c = getc(stdin);
            if (c == '\n')
                continue;
            ungetc(c, stdin);
            /* ------------------- */

            obj_t *object = interpret(vm, rdr);

            done = reader_eof(rdr);

            repl_println(object);
        }

        reader_delete(rdr);

        if (done) break;

        rdr = reader_new(stdin);
    }

    printf("\n");
}

int main(int argc, char **argv) {

    init();

    if (argc > 1) {
        read_file(vm, argv[1]);
    } else {
        repl(vm);
    }

    return 0;
}
