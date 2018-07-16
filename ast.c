#include "ast.h"
#include "global.h"

#include <stdio.h>
#include <stdlib.h>

ast *ast_new(int type, int val, int numchldrn, ast **children)
{
    ast *a = malloc(sizeof(ast));
    a->type = type;
    a->val = val;
    a->numchldrn = numchldrn;
    a->children = children;
    return a;
}

void ast_delete(ast *a)
{
    for (int i = 0; i < a->numchldrn; i++)
    {
        free(a->children[i]);
    }
    free(a);
}

void ast_print(ast *a, int offset)
{
    char *sp = malloc(sizeof(char) * offset);
    for (int i = 0; i < offset; i++)
    {
        sp[i] = ' ';
    }
    switch (a->type)
    {
    case NUM:
        printf("%s<type: NUM, val: %d>\n", sp, a->val);
        break;
    case SYM:
        printf("%s<type: SYM, val: %s>\n", sp, symtable[a->val].symbol);
        break;
    case EXP:
        printf("%s<type: EXP, val: %s, numchldrn: %d>\n", sp, symtable[a->val].symbol, a->numchldrn);
        for (int i = 0; i < a->numchldrn; i++)
        {
            ast_print(a->children[i], offset + 4);
        }
        break;
    }
    free(sp);
}