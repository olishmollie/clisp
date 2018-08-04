#ifndef LEX_H
#define LEX_H

#include <stdio.h>

#define BUFSIZE 999
#define EOS '\0'

typedef enum {
    TOK_INT,
    TOK_FLOAT,
    TOK_RAT,
    TOK_STR,
    TOK_SYM,
    TOK_CONST,
    TOK_NIL,
    TOK_LPAREN,
    TOK_RPAREN,
    TOK_DOT,
    TOK_DEF,
    TOK_COND,
    TOK_QUOTE,
    TOK_TICK,
    TOK_LAMBDA,
    TOK_END
} token_t;

typedef struct {
    token_t type;
    char *val;
} token;

token lex(FILE *f);

token token_new(token_t type, char *val);
void token_delete(token t);

#endif
