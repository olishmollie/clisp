#ifndef DEBUG_H
#define DEBUG_H

#define PRINT(name, args)                                                      \
    {                                                                          \
        printf("%s", name);                                                    \
        obj_println(args);                                                     \
    }

#endif