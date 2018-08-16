#include "global.h"
#include "lex.h"
#include "object.h"
#include "builtins.h"
#include "parse.h"
#include "eval.h"
#include "init.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include <editline/readline.h>

/*
 * TODO:
 * - multiline repl input
 * - compile to bytecode
 */

void repl_println(obj *o) {
    if (o) {
        printf("=> ");
        println(o);
    }
}

void repl() {

    FILE *stream;

    printf("fig version 0.1\n\n");

    while (1) {
        input = readline("> ");

        if (strlen(input) == 0)
            continue;

        add_history(input);
        stream = fmemopen(input, strlen(input), "r");

        repl_parser = parser_new(stream);

        obj *o = read(repl_parser);
        o = eval(universe, o);
        repl_println(o);

        free(input);
        parser_delete(repl_parser);
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
