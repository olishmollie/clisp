#ifndef _TABLE_H
#define _TABLE_H

#define MAXCHAR 999
#define MAXSYM 100

char lexemes[MAXCHAR];

typedef struct
{
    char *symbol;
} entry;

entry symtable[MAXSYM];

int insert(char *symbol);
int lookup(char *symbol);

#endif