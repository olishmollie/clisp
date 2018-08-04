#include "lex.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int curchar, numtok;
token curtok, peektok;

token token_new(token_t type, char *val) {
    token t;
    t.type = type;
    t.val = malloc(sizeof(char) * strlen(val) + 1);
    strcpy(t.val, val);
    return t;
}

void token_delete(token t) { free(t.val); }

void token_println(token t) {
    printf("<type: %d, val: '%s'>\n", t.type, t.val);
}

int nextchar(FILE *f) {
    curchar = fgetc(f);
    return curchar;
}

void skipspaces(FILE *f) {
    while (isspace(curchar)) {
        nextchar(f);
    }
}

token lexnum(FILE *f) {
    int rat = 0;
    char num[BUFSIZE];

    int i = 0;
    if (curchar == '-') {
        num[i++] = curchar;
        nextchar(f);
    }

    while (!feof(f) && isdigit(curchar)) {
        num[i++] = curchar;
        nextchar(f);
    }

    if (curchar == '/') {
        rat = 1;
        num[i++] = curchar;
        nextchar(f);
        if (curchar == '-') {
            num[i++] = curchar;
            nextchar(f);
        }
        while (!feof(f) && isdigit(curchar)) {
            num[i++] = curchar;
            nextchar(f);
        }
    }
    num[i] = EOS;

    ungetc(curchar, f);

    if (rat)
        return token_new(TOK_RAT, num);

    return token_new(TOK_INT, num);
}

token lexsymbol(FILE *f) {
    char sym[BUFSIZE];

    int i = 0;
    if (curchar == '-') {
        int c = fgetc(f);
        if (isdigit(c)) {
            ungetc(c, f);
            return lexnum(f);
        }
        ungetc(c, f);
    }

    sym[i++] = curchar;
    nextchar(f);

    while (!feof(f) && !isspace(curchar) && curchar != ')') {
        sym[i++] = curchar;
        nextchar(f);
    }
    sym[i] = EOS;

    ungetc(curchar, f);

    if (strcmp(sym, "nil") == 0)
        return token_new(TOK_NIL, sym);
    if (strcmp(sym, "def") == 0)
        return token_new(TOK_DEF, sym);
    if (strcmp(sym, "quote") == 0)
        return token_new(TOK_QUOTE, sym);
    if (strcmp(sym, "cond") == 0)
        return token_new(TOK_COND, sym);
    if (strcmp(sym, "lambda") == 0)
        return token_new(TOK_LAMBDA, sym);

    return token_new(TOK_SYM, sym);
}

token lexstring(FILE *f) {
    nextchar(f);

    char sym[BUFSIZE];

    int i = 0;
    while (curchar != '"') {

        if (feof(f)) {
            fprintf(stderr, "expected '\"'");
            break;
        }

        if (curchar == '\\') {

            nextchar(f);

            switch (curchar) {
            case 'n':
                sym[i++] = '\n';
                break;
            case 't':
                sym[i++] = '\t';
                break;
            case 'f':
                sym[i++] = '\f';
                break;
            default:
                sym[i++] = curchar;
            }

        } else {
            sym[i++] = curchar;
        }

        nextchar(f);
    }
    sym[i] = EOS;

    return token_new(TOK_STR, sym);
}

token lexconst(FILE *f) {
    char sym[BUFSIZE];
    int i = 0;
    while (!feof(f) && !isspace(curchar) && curchar != ')') {
        sym[i++] = curchar;
        nextchar(f);
    }
    sym[i] = EOS;

    ungetc(curchar, f);

    return token_new(TOK_CONST, sym);
}

token lex(FILE *f) {

    nextchar(f);
    skipspaces(f);

    if (feof(f))
        return token_new(TOK_END, "end");
    else if (isdigit(curchar))
        return lexnum(f);
    else if (curchar == '(') {
        return token_new(TOK_LPAREN, "(");
    } else if (curchar == ')') {
        return token_new(TOK_RPAREN, ")");
    } else if (curchar == '.') {
        return token_new(TOK_DOT, ".");
    } else if (curchar == '\'') {
        return token_new(TOK_TICK, "'");
    } else if (curchar == '"') {
        return lexstring(f);
    } else if (curchar == '#') {
        return lexconst(f);
    }

    return lexsymbol(f);
}
