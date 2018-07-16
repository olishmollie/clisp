#include "ast.h"
#include "global.h"

#include <stdio.h>
#include <stdlib.h>

ast *ast_new(token tok, int numchldrn, ast **children)
{
    ast *a = malloc(sizeof(ast));
    a->tok = tok;
    a->numchldrn = numchldrn;
    a->children = children;
    return a;
}

void ast_delete(ast *a)
{
    for (int i = 0; i < a->numchldrn; i++)
    {
        free(a->children[i]->tok.val);
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
    switch (a->tok.type)
    {
    case INT:
        printf("%s<type: NUM, val: %s>\n", sp, a->tok.val);
        break;
    case SYM:
        if (a->numchldrn)
        {
            printf("%s<type: SYM, val: %s>\n", sp, a->tok.val);
            for (int i = 0; i < a->numchldrn; i++)
            {
                ast_print(a->children[i], offset + 4);
            }
        }
        else
            printf("%s<type: SYM, val: %s>\n", sp, a->tok.val);
        break;
    default:
        printf("%s<type: EXP, val: %s, numchldrn: %d>\n", sp, a->tok.val, a->numchldrn);
        for (int i = 0; i < a->numchldrn; i++)
        {
            ast_print(a->children[i], offset + 4);
        }
        break;
    }
    free(sp);
}