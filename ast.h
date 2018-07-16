#ifndef _AST_H
#define _AST_H

#include "token.h"

typedef enum { AST_EXP, AST_NUM, AST_SYM, AST_ERR } ast_t;

typedef struct ast {
    ast_t type;
    token tok;
    int numchldrn;
    struct ast **children;
} ast;

ast *ast_new(ast_t type, token tok, int numchlrdn, ast **children);
void ast_delete(ast *a);

void ast_print(ast *a, int offset);

#endif