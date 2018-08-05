#ifndef PARSE_H
#define PARSE_H

#include "lex.h"
#include "object.h"

#include <stdio.h>

typedef struct {
    lexer *l;
    token curtok;
    token peektok;
} parser;

parser *parser_new(FILE *);
void parser_delete(parser *);
obj *read(parser *);

#endif