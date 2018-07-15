#include "table.h"

#include <ctype.h>
#include <editline/readline.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFSIZE 99

#define NUM 256
#define SYM 257
#define END 258
#define NONE -1
#define EOS '\0'

int tokenval, pos, eof = -1;

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
        return END;
    else if (isdigit(cur))
    {
        tokenval = lexdigit(input);
        return NUM;
    }
    else if (isalpha(cur))
    {
        tokenval = lexsymbol(input);
        return SYM;
    }
    else
    {
        tokenval = NONE;
        nextchar(input);
        return cur;
    }
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
        int tok = lexan(input);
        while (tok != END)
        {
            printf("tokenval = %d\n", tokenval);
            tok = lexan(input);
        }
        free(input);
    }

    return 0;
}