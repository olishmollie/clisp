#ifndef PARSE_H
#define PARSE_H

#include "lex.h"
#include "object.h"

#include <stdio.h>

token nexttok(FILE *);
void parse_init(FILE *);
obj *read(FILE *);
void parse_cleanup(void);

#endif