#include "table.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int lastsym = 0, lastchar = 0;

int insert(char *symbol)
{
    int tmp = lastsym;
    int len = strlen(symbol);
    if (lastchar + len + 1 > MAXCHAR)
    {
        fprintf(stderr, "out of lexeme space\n");
        exit(1);
    }
    if (lastsym > MAXSYM)
    {
        fprintf(stderr, "out of symbol space\n");
        exit(1);
    }

    strcpy(&lexemes[lastchar], symbol);
    entry e;
    e.symbol = &lexemes[lastchar];
    lastchar += len + 1;
    symtable[lastsym++] = e;
    return tmp;
}

int lookup(char *symbol)
{
    for (int i = 0; i < lastsym; i++)
    {
        if (strcmp(symtable[i].symbol, symbol) == 0)
        {
            return i;
        }
    }
    return -1;
}
