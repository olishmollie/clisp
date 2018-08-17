#ifndef GLOBAL_H
#define GLOBAL_H

#include "parse.h"
#include "object.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STDLIB mk_cons(mk_string("lib/lib.fig"), the_empty_list)
#define UNITTESTS mk_cons(mk_string("lib/tests.fig"), the_empty_list);

#define FIG_ASSERT(cond, fmt, ...)                                             \
    {                                                                          \
        if (!(cond))                                                           \
            return mk_err(fmt, ##__VA_ARGS__);                                 \
    }

#define ARG_NUMCHECK(args, name, num)                                          \
    {                                                                          \
        int count = 0;                                                         \
        obj *tmp = args;                                                       \
        while (tmp != the_empty_list) {                                        \
            count++;                                                           \
            tmp = cdr(tmp);                                                    \
        }                                                                      \
        if (count != num) {                                                    \
            return mk_err(                                                     \
                "incorrect argument count for %s. expected %d, got %d", name,  \
                num, count);                                                   \
        }                                                                      \
    }

#define ARG_TYPECHECK(args, name, typ)                                         \
    {                                                                          \
        obj *tmp = args;                                                       \
        while (tmp != the_empty_list) {                                        \
            if (car(tmp)->type != typ) {                                       \
                return mk_err("%s can only operate on type %s", name,          \
                              type_name(typ));                                 \
            }                                                                  \
            tmp = cdr(tmp);                                                    \
        }                                                                      \
    }

#define FIG_ERRORCHECK(args)                                                   \
    {                                                                          \
        if (args) {                                                            \
            if (!is_pair(args)) {                                              \
                if (is_error(args))                                            \
                    return args;                                               \
            } else {                                                           \
                obj *tmp = args;                                               \
                while (!is_the_empty_list(tmp)) {                              \
                    if (is_error(tmp))                                         \
                        return tmp;                                            \
                    if (is_error(car(tmp)))                                    \
                        return car(tmp);                                       \
                    tmp = cdr(tmp);                                            \
                }                                                              \
            }                                                                  \
        }                                                                      \
    }

obj *symbol_table;
obj *the_empty_list;
obj * true;
obj * false;
obj *quote_sym;
obj *define_sym;
obj *set_sym;
obj *if_sym;
obj *lambda_sym;
obj *begin_sym;

obj *universe;
char *input;
parser *repl_parser;

#endif