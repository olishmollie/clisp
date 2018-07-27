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
        if (args->list->count != num) {                                        \
            obj *err = obj_err(                                                \
                "incorrect number of arguments to %s. expected %d, got %d",    \
                fun, num, args->list->count);                                  \
            obj_delete(args);                                                  \
            return err;                                                        \
        }                                                                      \
    }

#define TARGCHECK(args, fun, typ)                                              \
    {                                                                          \
        obj *cur = args->list->head;                                           \
        for (int i = 0; i < args->list->count; i++) {                          \
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

// TODO: visit every node in error check
#define ERRCHECK(args)                                                         \
    {                                                                          \
        obj *cur = args->list->head;                                           \
        for (int i = 0; i < args->list->count; i++) {                          \
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