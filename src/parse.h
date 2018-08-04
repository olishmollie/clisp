#ifndef PARSE_H
#define PARSE_H

#include "object.h"

#include <stdio.h>

void parse_init(FILE *);

obj *read(FILE *);

void parse_cleanup(void);

#endif