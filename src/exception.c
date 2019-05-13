#include "exception.h"

#include <stdarg.h>

#define MAX_ERR_LEN 251

void raise(VM *vm, char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    char msg[MAX_ERR_LEN];
    vsnprintf(msg, MAX_ERR_LEN, fmt, ap);

    exc = mk_err(vm, msg);

    longjmp(exc_env, 1);

    va_end(ap);
}