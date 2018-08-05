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
    TOK_END,
    TOK_ERR
} token_t;

typedef struct {
    token_t type;
    char *val;
} token;

typedef struct {
    FILE *infile;
    int linenum;
    char curchar;
} lexer;

lexer *lexer_new(FILE *);
void lexer_delete(lexer *);

token lex(lexer *l);

token token_new(token_t type, char *val);
void token_println(token t);
void token_delete(token t);

#endif
