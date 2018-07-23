#ifndef _ERRORS_H
#define _ERRORS_H

#define CASSERT(args, cond, fmt, ...)                                          \
    {                                                                          \
        if (!(cond)) {                                                         \
            obj_delete(args);                                                  \
            return obj_err(fmt, ##__VA_ARGS__);                                \
        }                                                                      \
    }

#define NARGCHECK(args, fun, num)                                              \
    {                                                                          \
        if (args->count != num) {                                              \
            obj *err = obj_err(                                                \
                "incorrect number of arguments to %s. expected %d, got %d",    \
                fun, num, args->count);                                        \
            obj_delete(args);                                                  \
            return err;                                                        \
        }                                                                      \
    }

#define TARGCHECK(args, typ)                                                   \
    {                                                                          \
        obj *cur = args;                                                       \
        for (int i = 0; i < args->count; i++) {                                \
            if (obj_car(cur)->type != typ) {                                   \
                obj *err = obj_err("argument is not of type %s, got %s",       \
                                   obj_typename(typ), obj_car(cur)->type);     \
                obj_delete(args);                                              \
                return err;                                                    \
            }                                                                  \
            cur = obj_cdr(cur);                                                \
        }                                                                      \
    }

#define ERRCHECK(args)                                                         \
    {                                                                          \
        obj *cur = args;                                                       \
        for (int i = 0; i < args->count; i++) {                                \
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

#endif