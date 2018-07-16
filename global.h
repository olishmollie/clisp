#ifndef _GLOBAL_H
#define _GLOBAL_H

#define NUM 256
#define SYM 257
#define EXP 258
#define LPAR 259
#define RPAR 260
#define END 261

#define NONE -1
#define EOS '\0'

#define MAXSYM 100

typedef struct
{
    char *symbol;
} entry;

entry symtable[MAXSYM];

#endif