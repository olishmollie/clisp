#include "global.h"
#include "lex.h"
#include "object.h"
#include "builtins.h"
#include "parse.h"
#include "eval.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <gmp.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include <editline/readline.h>

/* TODO -----------------------------------------------------------------------
 * - lexer errors
 * - multiline repl input
 * - floating point/rational arithmetic
 * - make lexer and parser self-initializing and self-destructive
 * - compile to bytecode
 */

void register_builtin(env *e, builtin fun, char *name) {
    obj *k = obj_sym(name);
    obj *v = obj_builtin(name, fun);
    env_insert(e, k, v);
    obj_delete(k);
    obj_delete(v);
}

obj *builtin_exit(env *e, obj *args);

env *global_env(void) {
    env *e = env_new();
    register_builtin(e, builtin_plus, "+");
    register_builtin(e, builtin_minus, "-");
    register_builtin(e, builtin_times, "*");
    register_builtin(e, builtin_divide, "/");
    register_builtin(e, builtin_remainder, "%");

    register_builtin(e, builtin_gt, ">");
    register_builtin(e, builtin_gte, ">=");
    register_builtin(e, builtin_lt, "<");
    register_builtin(e, builtin_lte, "<=");

    register_builtin(e, builtin_cons, "cons");
    register_builtin(e, builtin_car, "car");
    register_builtin(e, builtin_cdr, "cdr");
    register_builtin(e, builtin_list, "list");
    register_builtin(e, builtin_eq, "eq");
    register_builtin(e, builtin_atom, "atom");
    register_builtin(e, builtin_eval, "eval");

    register_builtin(e, builtin_strtolist, "string->list");
    register_builtin(e, builtin_listtostr, "list->string");

    register_builtin(e, builtin_type, "type");
    register_builtin(e, builtin_print, "print");
    register_builtin(e, builtin_println, "println");
    register_builtin(e, builtin_err, "err");
    register_builtin(e, builtin_exit, "exit");
    return e;
}

env *universe;
FILE *stream;
char *input;

void finit(char *fname, FILE **f) {
    *f = fopen(fname, "r");
    if (!(*f)) {
        fprintf(stderr, "unable to locate file %s\n", fname);
        exit(1);
    }
    parse_init(*f);
}

void cleanup(char *input, FILE *stream) {
    if (input)
        free(input);
    parse_cleanup();
    fclose(stream);
}

obj *builtin_exit(env *e, obj *args) {
    NARGCHECK(args, "exit", 0);
    env_delete(e);
    obj_delete(args);
    cleanup(input, stream);
    exit(0);
    return NULL;
}

void readfile(char *fname) {

    FILE *infile;

    finit(fname, &infile);

    while (!feof(infile)) {
        obj *o = eval(universe, read(infile));
        if (o->type == OBJ_ERR) {
            obj_println(o);
            obj_delete(o);
            break;
        }
        obj_delete(o);
    }

    cleanup(NULL, infile);
}

void repl_println(obj *o) {
    printf("=> ");
    obj_println(o);
}

void repl() {

    printf("fig version 0.1\n\n");

    while (1) {
        input = readline("> ");
        if (strlen(input) == 0)
            continue;
        add_history(input);
        stream = fmemopen(input, strlen(input), "r");
        parse_init(stream);
        obj *o = eval(universe, read(stream));
        repl_println(o);
        obj_delete(o);
        cleanup(input, stream);
    }
}

int main(int argc, char **argv) {

    universe = global_env();
    readfile(STDLIB);

    if (argc > 1)
        readfile(argv[1]);
    else {
        repl();
    }

    env_delete(universe);

    return 0;
}
