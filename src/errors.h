#ifndef _ERRORS_H
#define _ERRORS_H

#define LASSERT(args, cond, fmt, ...)                                          \
    {                                                                          \
        if (!(cond)) {                                                         \
            obj_delete(args);                                                  \
            return obj_err(fmt, ##__VA_ARGS__);                                \
        }                                                                      \
    }

#endif