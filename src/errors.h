#ifndef _ERRORS_H
#define _ERRORS_H

#define LASSERT(args, cond, fmt, ...)                                          \
    {                                                                          \
        if (!(cond)) {                                                         \
            obj_delete(args);                                                  \
            return obj_err(fmt, ##__VA_ARGS__);                                \
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