#include "ast.h"
#include "global.h"
#include "object.h"
#include "table.h"
#include "token.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFSIZE 99
#define MAXCHILDREN 10

/* ============= LEXING ============== */
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

/* ============= PARSING ============== */
token peektok, curtok;

void lex_advance(char *input) {
    curtok = peektok;
    peektok = lexan(input);
}

int match(token_t type, char *input) {
    if (peektok.type == type) {
        lex_advance(input);
        return 1;
    }
    // fprintf(stderr, "expected token type %d, got %d\n", type, peektok.type);
    return 0;
}

// TODO: parser errors
ast *parse(char *input) {
    switch (peektok.type) {
    case INT:
        match(INT, input);
        return ast_new(AST_NUM, curtok, 0, 0);
    case SYM:
        match(SYM, input);
        return ast_new(AST_SYM, curtok, 0, 0);
    case LPAREN:
        match(LPAREN, input);
        token lparen = curtok;

        ast *tmp_children[MAXCHILDREN];
        int childpos = 0;
        while (peektok.type != RPAREN) {
            tmp_children[childpos++] = parse(input);
        }

        match(RPAREN, input);
        token_delete(curtok); /* don't need ')' */

        ast **children = malloc(sizeof(ast *) * childpos);
        memcpy(children, tmp_children, sizeof(ast *) * childpos);

        return ast_new(AST_EXP, lparen, childpos, children);
    default:
        fprintf(stderr, "error: returning null from ast *parse()...\n");
        return NULL;
    }
}

/* ============= EVALUATION ============== */
object *eval_op(object *x, char *op, object *y) {

    if (x->type == OBJ_ERROR)
        return x;
    if (y->type == OBJ_ERROR)
        return y;

    if (strcmp("+", op) == 0)
        return object_long(x->lval + y->lval);
    if (strcmp("-", op) == 0)
        return object_long(x->lval - y->lval);
    if (strcmp("*", op) == 0)
        return object_long(x->lval * y->lval);
    if (strcmp("/", op) == 0) {
        return y->lval == 0 ? object_error("division by zero")
                            : object_long(x->lval / y->lval);
    }
    if (strcmp("%", op) == 0) {
        return y->lval == 0 ? object_error("division by zero")
                            : object_long(x->lval % y->lval);
    }
    return object_error("unknown operator");
}

object *eval(ast *root) {
    if (root->type == AST_NUM) {
        errno = 0;
        long x = strtol(root->tok.val, NULL, 10);
        return errno != ERANGE ? object_long(x)
                               : object_error("bad number syntax");
    }

    if (root->type == AST_SYM) {
        return object_sym(root->tok.val);
    }

    if (root->type == AST_EXP) {
        object *x = object_sexp();
        for (int i = 0; i < root->numchldrn; i++) {
            x = object_add(x, eval(root->children[i]));
        }

        return x;
    }

    printf("Error, returning null");
    return NULL;
}

#include <editline/readline.h>

/* ============= REPL ============== */
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
        object *res = eval(prog);
        object_println(res);
        object_delete(res);
        ast_delete(prog);

        free(input);
    }

    return 0;
}