#ifndef ASSERT_H
#define ASSERT_H

#define FIG_ASSERT(vm, cond, fmt, ...)                                         \
    {                                                                          \
        if (!(cond))                                                           \
            raise(vm, fmt, ##__VA_ARGS__);                             \
    }

#define ARG_NUMCHECK(vm, args, name, num)                                      \
    {                                                                          \
        int count = 0;                                                         \
        obj_t *tmp = args;                                                     \
        while (tmp != the_empty_list) {                                        \
            count++;                                                           \
            tmp = cdr(tmp);                                                    \
        }                                                                      \
        if (count != num) {                                                    \
            raise(                                                     \
                vm,                                                            \
                "incorrect argument count for %s. expected %d, got %d", name,  \
                num, count);                                                   \
        }                                                                      \
    }

#define ARG_TYPECHECK(vm, args, name, typ)                                     \
    {                                                                          \
        obj_t *tmp = args;                                                     \
        while (tmp != the_empty_list) {                                        \
            if (car(tmp)->type != typ) {                                       \
                raise(vm, "%s can only operate on type %s", name,      \
                              type_name(typ));                                 \
            }                                                                  \
            tmp = cdr(tmp);                                                    \
        }                                                                      \
    }

#endif