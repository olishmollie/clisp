#include "ast.h"
#include "global.h"
#include "table.h"
#include "token.h"

#include <ctype.h>
#include <editline/readline.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFSIZE 99
#define MAXCHILDREN 10

int pos;

int nextchar(char *input) { return input[++pos]; }

int curchar(char *input) { return input[pos]; }

void skipspaces(char *input) {
    while (isspace(curchar(input))) {
        nextchar(input);
    }
}

token lexdigit(char *input) {
    char num[BUFSIZE];
    int i = 0;
    while (curchar(input) && isdigit(curchar(input))) {
        num[i++] = curchar(input);
        nextchar(input);
    }
    num[i] = EOS;
    return token_new(INT, num);
}

token lexsymbol(char *input) {
    char sym[BUFSIZE];
    int i = 0;
    while (curchar(input) && !isspace(curchar(input)) &&
           curchar(input) != ')') {
        sym[i++] = curchar(input);
        nextchar(input);
    }
    sym[i] = EOS;
    if (table_lookup(sym) < 0)
        table_insert(sym);
    return token_new(SYM, sym);
}

token lexan(char *input) {
    skipspaces(input);
    int cur = curchar(input);
    if (cur == EOS)
        return token_new(END, "end");
    else if (cur == '(') {
        nextchar(input);
        return token_new(LPAREN, "(");
    } else if (cur == ')') {
        nextchar(input);
        return token_new(RPAREN, ")");
    } else if (isdigit(cur)) {
        return lexdigit(input);
    } else {
        return lexsymbol(input);
    }
}

token peektok, curtok;

int match(token_t type, char *input) {
    if (peektok.type == type) {
        curtok = peektok;
        peektok = lexan(input);
        return 1;
    }
    fprintf(stderr, "expected token type %d, got %d\n", type, peektok.type);
    return 0;
}

ast *parse(char *input) {
    switch (peektok.type) {
    case INT:
        match(INT, input);
        return ast_new(curtok, 0, 0);
    case SYM:
        match(SYM, input);
        return ast_new(curtok, 0, 0);
    case LPAREN:
        match(LPAREN, input);
        token_delete(curtok);

        match(SYM, input);
        token op = curtok;

        ast *tmp_children[MAXCHILDREN];
        int childpos = 0;
        while (peektok.type != RPAREN) {
            tmp_children[childpos++] = parse(input);
        }

        match(RPAREN, input);
        token_delete(curtok);

        ast **children = malloc(sizeof(ast *) * childpos);
        memcpy(children, tmp_children, sizeof(ast *) * childpos);

        return ast_new(op, childpos, children);
    default:
        fprintf(stderr, "error: returning null from ast *parse()...\n");
        return NULL;
    }
}

long eval_op(long x, char *op, long y) {
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

long eval(ast *root) {
    if (root->tok.type == INT)
        return atol(root->tok.val);

    char *op = root->tok.val;

    long x = eval(root->children[0]);
    for (int i = 1; i < root->numchldrn; i++) {
        x = eval_op(x, op, eval(root->children[i]));
    }

    return x;
}

int main(void) {
    table_init();
    printf("clisp version 0.1\n\n");

    while (1) {
        pos = 0;
        char *input = readline("> ");
        add_history(input);

        // Initialize the lexer.
        peektok = lexan(input);

        ast *prog = parse(input);
        // ast_print(prog, 0);
        long res = eval(prog);
        printf("%li\n", res);
        ast_delete(prog);

        free(input);
    }

    return 0;
}