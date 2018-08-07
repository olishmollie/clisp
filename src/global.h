#ifndef GLOBAL_H
#define GLOBAL_H

#include "object.h"

#define STDLIB "lib/lib.fig"
#define UNITTESTS "lib/tests.fig"

#define CASSERT(args, cond, fmt, ...)                                          \
    {                                                                          \
        if (!(cond)) {                                                         \
            obj *err = obj_err(fmt, ##__VA_ARGS__);                            \
            obj_delete(args);                                                  \
            return err;                                                        \
        }                                                                      \
    }

#define NARGCHECK(args, fun, num)                                              \
    {                                                                          \
        if (args->nargs != num) {                                              \
            obj *err = obj_err(                                                \
                "incorrect number of arguments to %s. expected %d, got %d",    \
                fun, num, args->nargs);                                        \
            obj_delete(args);                                                  \
            return err;                                                        \
        }                                                                      \
    }

#define TARGCHECK(args, fun, typ)                                              \
    {                                                                          \
        obj *cur = args;                                                       \
        for (int i = 0; i < args->nargs; i++) {                                \
            if (obj_car(cur)->type != typ) {                                   \
                obj *err = obj_err("argument to %s is not of type %s, got %s", \
                                   fun, obj_typename(typ),                     \
                                   obj_typename(obj_car(cur)->type));          \
                obj_delete(args);                                              \
                return err;                                                    \
            }                                                                  \
            cur = obj_cdr(cur);                                                \
        }                                                                      \
    }

#define ERRCHECK(args)                                                         \
    {                                                                          \
        obj *cur = args;                                                       \
        for (int i = 0; i < args->nargs; i++) {                                \
            if (obj_car(cur)->type == OBJ_ERR) {                               \
                obj *err = obj_err(obj_car(cur)->err);                         \
                obj_delete(args);                                              \
                return err;                                                    \
            } else if (obj_cdr(cur)->type == OBJ_ERR) {                        \
                obj *err = obj_err(obj_cdr(cur)->err);                         \
                obj_delete(args);                                              \
                return err;                                                    \
            }                                                                  \
            cur = obj_cdr(cur);                                                \
        }                                                                      \
    }

#define INCR_OBJ(o)                                                            \
    { numobj++; }

#define DECR_OBJ(o)                                                            \
    { numobj--; }

#define PRINT(name, o)                                                         \
    {                                                                          \
        printf("%s = ", name);                                                 \
        obj_println(o);                                                        \
    }

int numobj;

#endif