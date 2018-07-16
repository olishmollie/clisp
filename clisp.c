#include "global.h"
#include "table.h"
#include "ast.h"
#include "token.h"

#include <ctype.h>
#include <editline/readline.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFSIZE 99

#define MAXCHILDREN 10

int pos;
token peektok, curtok;

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

token lexdigit(char *input)
{
    char num[BUFSIZE];
    int i = 0;
    while (curchar(input) && isdigit(curchar(input)))
    {
        num[i++] = curchar(input);
        nextchar(input);
    }
    num[i] = EOS;
    return token_new(INT, num);
}

token lexsymbol(char *input)
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
    if (lookup(sym) < 0)
        insert(sym);
    return token_new(SYM, sym);
}

token lexan(char *input)
{
    skipspaces(input);
    int cur = curchar(input);
    if (cur == EOS)
        return token_new(END, "end");
    else if (cur == '(')
    {
        nextchar(input);
        return token_new(LPAREN, "(");
    }
    else if (cur == ')')
    {
        nextchar(input);
        return token_new(RPAREN, ")");
    }
    else if (isdigit(cur))
    {
        return lexdigit(input);
    }
    else
    {
        return lexsymbol(input);
    }
}

int match(token_t type, char *input)
{
    if (peektok.type == type)
    {
        curtok = peektok;
        peektok = lexan(input);
        return 1;
    }
    fprintf(stderr, "expected token type %d, got %d\n", type, peektok.type);
    return 0;
}

ast *parse(char *input)
{
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
        curtok = lexan(input);
        while (curtok.type != END)
        {
            token_print(curtok);
            token_delete(curtok);
            curtok = lexan(input);
        }

        free(input);
    }

    return 0;
}