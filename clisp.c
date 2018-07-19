#include "src/global.h"
#include "src/object.h"
#include "src/table.h"
#include "src/token.h"

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
    if (curtok.type == type) {
        lex_advance(input);
        return 1;
    }
    // fprintf(stderr, "expected token type %d, got %d\n", type, peektok.type);
    return 0;
}

obj *read_long(token tok) {
    errno = 0;
    long x = strtol(tok.val, NULL, 10);
    token_delete(tok);
    return errno != ERANGE ? obj_num(x) : obj_err("bad number syntax");
}

obj *read_sym(token tok) {
    obj *o = obj_sym(tok.val);
    token_delete(tok);
    return o;
}

obj *read(char *input);

obj *read_list(char *input) {
    if (curtok.type == END)
        return obj_err("unexpected eof, expected ')'");
    if (curtok.type == RPAREN)
        return obj_nil();

    obj *car = read(input);
    obj *cdr = read_list(input);

    return obj_cons(car, cdr);
}

obj *read(char *input) {
    token tok;
    switch (curtok.type) {
    case INT:
        tok = curtok;
        match(INT, input);
        return read_long(tok);
    case SYM:
        tok = curtok;
        match(SYM, input);
        return read_sym(tok);
    case LPAREN:
        tok = curtok;
        match(LPAREN, input);
        token_delete(tok);
        obj *list = read_list(input);
        tok = curtok;
        match(RPAREN, input);
        token_delete(tok);
        return list;
    case RPAREN:
        return obj_err("unexpected ')'");
    default:
        fprintf(stderr, "error: returning null from read()...\n");
        return NULL;
    }
}

/* ============= REPL ============== */
#include <editline/readline.h>

int main(void) {
    table_init();
    printf("clisp version 0.1\n\n");

    while (1) {
        pos = 0;
        char *input = readline("> ");
        add_history(input);

        // Initialize the lexer.
        curtok = lexan(input);
        peektok = lexan(input);

        obj *o = read(input);
        obj_println(o);

        obj_delete(o);
        free(input);
    }

    return 0;
}