#ifndef _AST_H
#define _AST_H

#include "token.h"

typedef struct ast
{
    token tok;
    int numchldrn;
    struct ast **children;
} ast;

ast *ast_new(token tok, int numchlrdn, ast **children);
void ast_delete(ast *a);

void ast_print(ast *a, int offset);

#endif