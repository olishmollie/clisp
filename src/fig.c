#include "eval.h"
#include "builtins.h"
#include "init.h"
#include "read.h"

void repl_println(obj *object) {
    if (object) {
        printf("=> ");
        println(object);
    }
}

void repl() {

    printf("fig version 0.1\n\n");

    while (1) {
        printf("> ");
        reader *rdr = reader_new(stdin);

        /* if user hits enter with no data */
        int c = getc(stdin);
        if (c == '\n')
            continue;
        ungetc(c, stdin);

        obj *object = eval(universe, read(rdr));
        repl_println(object);
        reader_delete(rdr);
    }
}

int main(int argc, char **argv) {

    init();

    if (argc > 1) {
        obj *f = mk_string(argv[1]);
        builtin_load(f);
    } else {
        repl();
    }

    return 0;
}
