#include "global.h"
#include "eval.h"
#include "builtins.h"
#include "init.h"
#include "read.h"

#include <editline/readline.h>

/*
 * TODO:
 * - compile to bytecode
 */

void repl_println(obj *o) {
    if (o) {
        printf("=> ");
        println(o);
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

        obj *o = eval(universe, read(rdr));
        repl_println(o);
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
