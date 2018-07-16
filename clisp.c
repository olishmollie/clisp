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

int peektok, curtok, tokenval, pos;

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
    while (curchar(input) && !isspace(curchar(input)) && curchar(input) != ')')
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
        return END;
    else if (cur == '(')
    {
        nextchar(input);
        tokenval = NONE;
        return LPAR;
    }
    else if (cur == ')')
    {
        nextchar(input);
        tokenval = NONE;
        return RPAR;
    }
    else if (isdigit(cur))
    {
        tokenval = lexdigit(input);
        return NUM;
    }
    else
    {
        tokenval = lexsymbol(input);
        return SYM;
    }
}

int match(int type, char *input)
{
    if (peektok == type)
    {
        curtok = peektok;
        peektok = lexan(input);
        return 1;
    }
    fprintf(stderr, "expected token type %d, got %d\n", type, peektok);
    return 0;
}

ast *parse(char *input)
{
    ast *child_store[MAXCHILDREN];
    int child_store_pos = 0;
    int val;
    switch (peektok)
    {
    case NUM:
        val = tokenval;
        match(NUM, input);
        return ast_new(NUM, val, 0, 0);
    case SYM:
        val = tokenval;
        match(SYM, input);
        return ast_new(SYM, val, 0, 0);
    case LPAR:
        match(LPAR, input);
        val = tokenval;
        match(SYM, input);
        while (peektok != RPAR)
        {
            ast *child = parse(input);
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

long eval_op(long x, char *op, long y)
{
    if (strcmp("+", op) == 0)
        return x + y;
    if (strcmp("-", op) == 0)
        return x - y;
    if (strcmp("*", op) == 0)
        return x * y;
    if (strcmp("/", op) == 0)
        return x / y;
    return 0;
}

long eval(ast *root)
{
    if (root->type == NUM)
    {
        return root->val;
    }
    else if (root->type == EXP)
    {
        char *op = symtable[root->val].symbol;
        long x = eval(root->children[0]);
        for (int i = 1; i < root->numchldrn; i++)
        {
            x = eval_op(x, op, eval(root->children[i]));
        }
        return x;
    }

    return -1;
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

        // Initialize the lexer.
        peektok = lexan(input);

        ast *prog = parse(input);
        if (prog)
        {
            // ast_print(prog, 0);
            long res = eval(prog);
            printf("%li\n", res);
            ast_delete(prog);
        }

        free(input);
    }

    return 0;
}