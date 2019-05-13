#ifndef READ_H
#define READ_H

#include "common.h"

typedef struct {
    int cur;
    int line;
    FILE *in;
} Reader;

typedef struct VM VM;

int is_delim(int c);

Reader *reader_new(FILE *in);
void reader_flush(Reader *rdr);
void reader_delete(Reader *rdr);
int reader_eof(Reader *rdr);

obj_t *interpret(VM *vm, Reader *rdr);
obj_t *read(VM *vm, Reader *rdr);
obj_t *read_file(VM *vm, char *fname);

#endif