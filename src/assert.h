#ifndef ASSERT_H
#define ASSERT_H

#define FIG_ASSERT(cond, fmt, ...)                                             \
    {                                                                          \
        if (!(cond))                                                           \
            return mk_err(fmt, ##__VA_ARGS__);                                 \
    }

#define ARG_NUMCHECK(args, name, num)                                          \
    {                                                                          \
        int count = 0;                                                         \
        obj_t *tmp = args;                                                     \
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
        obj_t *tmp = args;                                                     \
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
                obj_t *tmp = args;                                             \
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

#endif