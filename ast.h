#ifndef _AST_H
#define _AST_H

typedef struct ast
{
    int type;
    int val;
    int numchldrn;
    struct ast **children;
} ast;

ast *ast_new(int type, int val, int numchlrdn, ast **children);
void ast_delete(ast *a);

void ast_print(ast *a, int offset);

#endif