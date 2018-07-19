#include "token.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

token token_new(token_t type, char *val) {
    token t;
    t.type = type;
    t.val = malloc(sizeof(char) * strlen(val) + 1);
    strcpy(t.val, val);
    return t;
}

void token_delete(token t) { free(t.val); }

void token_println(token t) {
    printf("<type: %d, val: '%s'>\n", t.type, t.val);
}