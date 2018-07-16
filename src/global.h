#ifndef _GLOBAL_H
#define _GLOBAL_H

#define NONE -1
#define EOS '\0'

#define MAXSYM 100

typedef struct {
    char *symbol;
} entry;

entry symtable[MAXSYM];

#endif