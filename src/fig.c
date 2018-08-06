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
 * - compile to bytecode
 */

void register_builtin(env *e, builtin fun, char *name) {
    obj *k = obj_sym(name);
    obj *v = obj_builtin(name, fun);
    env_insert(e, k, v);
    obj_delete(k);
    obj_delete(v);
}

obj *builtin_printenv(env *e, obj *args);
obj *builtin_load(env *e, obj *args);
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
    register_builtin(e, builtin_error, "error");

    register_builtin(e, builtin_printenv, "printenv");
    register_builtin(e, builtin_load, "load");
    register_builtin(e, builtin_exit, "exit");
    return e;
}

/*
 * these are globals so that builtin_exit
 * can clean up after itself
 */
env *universe;
FILE *stream;
char *input;
parser *repl_parser;

obj *readfile(char *fname) {

    FILE *infile;
    infile = fopen(fname, "r");

    if (!infile)
        return obj_err("could not open %s", fname);

    parser *p = parser_new(infile);

    while (!feof(infile)) {
        obj *o = eval(universe, read(p));
        if (o && o->type == OBJ_ERR) {
            obj_println(o);
            obj_delete(o);
            break;
        }
        obj_delete(o);
    }

    parser_delete(p);
    fclose(infile);

    return NULL;
}

void repl_println(obj *o) {
    if (o) {
        printf("=> ");
        obj_println(o);
    }
}

obj *builtin_printenv(env *e, obj *args) {
    NARGCHECK(args, "printenv", 0);
    obj_delete(args);
    env_print(e);
    return NULL;
}

obj *builtin_load(env *e, obj *args) {
    NARGCHECK(args, "load", 1);
    obj *f = obj_popcar(&args);
    char *filename = f->str;
    obj *res = readfile(filename);
    obj_delete(f);
    obj_delete(args);
    return res;
}

obj *builtin_exit(env *e, obj *args) {
    NARGCHECK(args, "exit", 0);
    printf("exiting...numobj = %d\n", numobj);
    env_delete(universe);
    obj_delete(args);
    parser_delete(repl_parser);
    free(input);
    fclose(stream);
    printf("numobj = %d\n", numobj);
    exit(0);
    return NULL;
}

int numobj = 0;

void repl() {

    printf("fig version 0.1\n\n");

    while (1) {
        printf("numobj = %d\n", numobj);
        input = readline("> ");

        if (strlen(input) == 0)
            continue;

        add_history(input);
        stream = fmemopen(input, strlen(input), "r");

        repl_parser = parser_new(stream);
        obj *o = eval(universe, read(repl_parser));
        repl_println(o);

        obj_delete(o);
        free(input);
        parser_delete(repl_parser);
    }
}

int main(int argc, char **argv) {

    universe = global_env();
    // readfile(STDLIB);

    if (argc > 1)
        readfile(argv[1]);
    else {
        repl();
    }

    env_delete(universe);

    return 0;
}
