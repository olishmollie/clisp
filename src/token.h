#ifndef _TOKEN_H
#define _TOKEN_H

typedef enum { INT, FLOAT, RAT, SYM, LPAREN, RPAREN, END } token_t;

typedef struct {
    token_t type;
    char *val;
} token;

token token_new(token_t type, char *val);
void token_delete(token t);
void token_print(token t);

#endif