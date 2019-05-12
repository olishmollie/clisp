#ifndef READ_H
#define READ_H

#include "object.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct {
    int cur;
    int linenum;
    FILE *in;
} Reader;

typedef struct VM VM;

int is_delim(int c);

Reader *reader_new(FILE *in);
obj_t *read(VM *vm, Reader *rdr);
void reader_delete(Reader *rdr);
int reader_eof(Reader *rdr);

#endif