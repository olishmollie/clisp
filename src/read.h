#ifndef READ_H
#define READ_H

#include "global.h"

typedef struct {
    int cur;
    int linenum;
    FILE *in;
} reader;

reader *reader_new(FILE *in);
obj *read(reader *rdr);
void reader_delete(reader *rdr);

#endif