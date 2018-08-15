#include "lex.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

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

lexer *lexer_new(FILE *f) {
    lexer *l = malloc(sizeof(lexer));
    l->infile = f;
    l->curchar = 0;
    l->linenum = 0;
    return l;
}

void lexer_delete(lexer *l) {
    fclose(l->infile);
    free(l);
}

int nextchar(lexer *l) {
    l->curchar = fgetc(l->infile);
    if (l->curchar == '\n')
        l->linenum++;
    return l->curchar;
}

token lexnum(lexer *l) {
    int rat = 0, frac = 0;
    char num[BUFSIZE];

    int i = 0;
    if (l->curchar == '-') {
        num[i++] = l->curchar;
        nextchar(l);
    }

    while (!feof(l->infile) && isdigit(l->curchar)) {
        num[i++] = l->curchar;
        nextchar(l);
    }

    if (l->curchar == '/') {
        rat = 1;
        num[i++] = l->curchar;
        nextchar(l);
        if (l->curchar == '-') {
            num[i++] = l->curchar;
            nextchar(l);
        }
        while (!feof(l->infile) && isdigit(l->curchar)) {
            num[i++] = l->curchar;
            nextchar(l);
        }
    } else if (l->curchar == '.') {
        frac = 1;
        num[i++] = l->curchar;
        nextchar(l);
        while (!feof(l->infile) && isdigit(l->curchar)) {
            num[i++] = l->curchar;
            nextchar(l);
        }
    }
    num[i] = EOS;

    ungetc(l->curchar, l->infile);

    if (rat)
        return token_new(TOK_RAT, num);
    if (frac)
        return token_new(TOK_FLOAT, num);

    return token_new(TOK_INT, num);
}

token lexsymbol(lexer *l) {
    char sym[BUFSIZE];

    int i = 0;
    if (l->curchar == '-') {
        int c = fgetc(l->infile);
        if (isdigit(c)) {
            ungetc(c, l->infile);
            return lexnum(l);
        }
        ungetc(c, l->infile);
    }

    sym[i++] = l->curchar;
    nextchar(l);

    while (!feof(l->infile) && !isspace(l->curchar) && l->curchar != ')') {
        sym[i++] = l->curchar;
        nextchar(l);
    }
    sym[i] = EOS;

    ungetc(l->curchar, l->infile);

    return token_new(TOK_SYM, sym);
}

token lexstring(lexer *l) {
    nextchar(l);

    char sym[BUFSIZE];

    int i = 0;
    while (l->curchar != '"') {

        if (feof(l->infile))
            return token_new(TOK_ERR, "unclosed quotation");

        if (l->curchar == '\\') {

            nextchar(l);

            switch (l->curchar) {
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
                sym[i++] = l->curchar;
            }

        } else {
            sym[i++] = l->curchar;
        }

        nextchar(l);
    }
    sym[i] = EOS;

    return token_new(TOK_STR, sym);
}

token lexconst(lexer *l) {
    char sym[BUFSIZE];
    int i = 0;
    while (!feof(l->infile) && !isspace(l->curchar) && l->curchar != ')') {
        sym[i++] = l->curchar;
        nextchar(l);
    }
    sym[i] = EOS;

    ungetc(l->curchar, l->infile);

    return token_new(TOK_CONST, sym);
}

void skip_spaces_and_comments(lexer *l) {
    while (!feof(l->infile)) {
        if (isspace(l->curchar)) {
            nextchar(l);
            continue;
        } else if (l->curchar == ';') {
            while (!feof(l->infile) && l->curchar != '\n') {
                nextchar(l);
            }
            continue;
        }
        break;
    }
}

token lex(lexer *l) {

    nextchar(l);

    // skipspaces(l);
    // if (l->curchar == ';')
    skip_spaces_and_comments(l);

    if (feof(l->infile))
        return token_new(TOK_END, "end");
    else if (isdigit(l->curchar))
        return lexnum(l);
    else if (l->curchar == '(')
        return token_new(TOK_LPAREN, "(");
    else if (l->curchar == ')')
        return token_new(TOK_RPAREN, ")");
    else if (l->curchar == '.')
        return token_new(TOK_DOT, ".");
    else if (l->curchar == '\'')
        return token_new(TOK_TICK, "'");
    else if (l->curchar == '"')
        return lexstring(l);
    else if (l->curchar == '#')
        return lexconst(l);

    return lexsymbol(l);
}
