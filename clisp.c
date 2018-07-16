#include "global.h"
#include "table.h"
#include "ast.h"

#include <ctype.h>
#include <editline/readline.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFSIZE 99

#define MAXCHILDREN 10

int curtok, tokenval, pos, eof = -1;

int nextchar(char *input)
{
    return input[++pos];
}

int curchar(char *input)
{
    return input[pos];
}

void skipspaces(char *input)
{
    while (isspace(curchar(input)))
    {
        nextchar(input);
    }
}

int lexdigit(char *input)
{
    int num = 0;
    while (curchar(input) && isdigit(curchar(input)))
    {
        num = num * 10 + (curchar(input) - '0');
        nextchar(input);
    }
    return num;
}

int lexsymbol(char *input)
{
    char sym[BUFSIZE];
    int i = 0;
    while (curchar(input) && !isdigit(curchar(input)) && !isspace(curchar(input)))
    {
        sym[i++] = curchar(input);
        nextchar(input);
    }
    sym[i] = EOS;
    int p = lookup(sym);
    if (p < 0)
        p = insert(sym);
    return p;
}

int lexan(char *input)
{
    skipspaces(input);
    int cur = curchar(input);
    if (cur == EOS)
        curtok = END;
    else if (cur == '(')
    {
        nextchar(input);
        tokenval = NONE;
        curtok = LPAR;
    }
    else if (cur == ')')
    {
        nextchar(input);
        tokenval = NONE;
        curtok = RPAR;
    }
    else if (isdigit(cur))
    {
        tokenval = lexdigit(input);
        curtok = NUM;
    }
    else
    {
        tokenval = lexsymbol(input);
        curtok = SYM;
    }
    return curtok;
}

int match(int type, char *input)
{
    if (curtok == type)
    {
        curtok = lexan(input);
        return 1;
    }
    fprintf(stderr, "expected token type %d, got %d", type, curtok);
    return 0;
}

ast *parse(char *input)
{
    ast *child_store[MAXCHILDREN];
    int child_store_pos = 0;
    int token = lexan(input);
    switch (token)
    {
    case NUM:
        return ast_new(NUM, tokenval, 0, 0);
    case SYM:
        return ast_new(SYM, tokenval, 0, 0);
    case LPAR:
        match(LPAR, input);
        int val = tokenval;
        while (curtok != RPAR)
        {
            ast *child = parse(input);
            if (child)
                child_store[child_store_pos++] = child;
        }
        match(RPAR, input);
        int numchldrn = child_store_pos;
        ast **children = malloc(sizeof(ast *) * numchldrn);
        for (int i = 0; i < numchldrn; i++)
        {
            children[i] = child_store[i];
        }
        return ast_new(EXP, val, numchldrn, children);
    }

    return NULL;
}

int main(void)
{
    init();
    printf("Welcome to clisp! Use ctrl+c to exit.\n");
    while (1)
    {
        pos = 0;
        char *input = readline("clisp> ");
        add_history(input);
        ast *prog = parse(input);
        if (prog)
        {
            ast_print(prog, 0);
            ast_delete(prog);
        }
        free(input);
    }

    return 0;
}