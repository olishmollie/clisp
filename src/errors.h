#ifndef _ERRORS_H
#define _ERRORS_H

#define LASSERT(args, cond, fmt, ...)                                          \
    {                                                                          \
        if (!(cond)) {                                                         \
            obj_delete(args);                                                  \
            return obj_err(fmt, ##__VA_ARGS__);                                \
        }                                                                      \
    }

#define NUMARGSASSERT(args, fun, num)                                          \
    {                                                                          \
        if (args->sexpr->count != num) {                                       \
            obj *err = obj_err(                                                \
                "incorrect number of arguments to %s. expected %d, got %d",    \
                fun, num, args->sexpr->count);                                 \
            obj_delete(args);                                                  \
            return err;                                                        \
        }                                                                      \
    }

#define TYPEASSERT(args, typ)                                                  \
    {                                                                          \
        for (int i = 0; i < args->sexpr->count; i++) {                         \
            if (args->sexpr->cell[i]->type != typ) {                           \
                obj *err =                                                     \
                    obj_err("argument is not of type %s", obj_typename(typ));  \
                obj_delete(args);                                              \
                return err;                                                    \
            }                                                                  \
        }                                                                      \
    }

#endif