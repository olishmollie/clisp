#include "src/object.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFSIZE 99
#define EOS '\0'

/* LEXING ------------------------------------------------------------------- */
typedef enum {
    INT,
    FLOAT,
    RAT,
    SYM,
    TRU,
    FALS,
    NIL,
    LPAREN,
    RPAREN,
    END
} token_t;

typedef struct {
    token_t type;
    char *val;
} token;

int pos, numtok;
token curtok, peektok;

token token_new(token_t type, char *val) {
    token t;
    t.type = type;
    t.val = malloc(sizeof(char) * strlen(val) + 1);
    strcpy(t.val, val);
    numtok++;
    return t;
}

void token_delete(token t) {
    free(t.val);
    numtok--;
}

void token_println(token t) {
    printf("<type: %d, val: '%s'>\n", t.type, t.val);
}

int nextchar(char *input) { return input[++pos]; }

int curchar(char *input) { return input[pos]; }

int backup(char *input) { return input[pos--]; }

int peekchar(char *input) {
    int c = nextchar(input);
    backup(input);
    return c;
}

void skipspaces(char *input) {
    while (isspace(curchar(input))) {
        nextchar(input);
    }
}

token lexdigit(char *input) {
    char num[BUFSIZE];
    int i = 0;
    if (curchar(input) == '+' || curchar(input) == '-') {
        num[i++] = curchar(input);
        nextchar(input);
    }
    while (curchar(input) && isdigit(curchar(input))) {
        num[i++] = curchar(input);
        nextchar(input);
    }
    num[i] = EOS;
    return token_new(INT, num);
}

token lexsymbol(char *input) {
    if (curchar(input) == '+' || curchar(input) == '-') {
        if (isdigit(peekchar(input)))
            return lexdigit(input);
    }
    char sym[BUFSIZE];
    int i = 0;
    while (curchar(input) && !isspace(curchar(input)) &&
           curchar(input) != ')') {
        sym[i++] = curchar(input);
        nextchar(input);
    }
    sym[i] = EOS;
    if (strcmp(sym, "nil") == 0)
        return token_new(NIL, sym);
    if (strcmp(sym, "true") == 0)
        return token_new(TRU, sym);
    if (strcmp(sym, "false") == 0)
        return token_new(FALS, sym);

    return token_new(SYM, sym);
}

token lexan(char *input) {
    skipspaces(input);
    int cur = curchar(input);
    if (cur == EOS)
        return token_new(END, "end");
    else if (isdigit(cur))
        return lexdigit(input);
    else if (cur == '(') {
        nextchar(input);
        return token_new(LPAREN, "(");
    } else if (cur == ')') {
        nextchar(input);
        return token_new(RPAREN, ")");
    } else {
        return lexsymbol(input);
    }
}

void lex_init(char *input) {
    numtok = 0;
    pos = 0;
    peektok = lexan(input);
}

void lex_cleanup(void) { token_delete(peektok); }

/* PARSING ------------------------------------------------------------------ */

token nexttok(char *input) {
    curtok = peektok;
    peektok = lexan(input);
    return curtok;
}

int take(token_t type, char *input) {
    if (curtok.type == type) {
        curtok = lexan(input);
        return 1;
    }
    fprintf(stderr, "whooops! unexpected token type %d\n", curtok.type);
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

obj *read_sexpr(char *input) {
    if (curtok.type == END)
        return obj_err("unexpected eof, expected ')'");
    obj *o = obj_sexpr();
    while (peektok.type != RPAREN) {
        obj_add(o, read(input));
    }
    return o;
}

obj *read(char *input) {
    curtok = nexttok(input);
    token tok;
    switch (curtok.type) {
    case INT:
        // tok = curtok;
        // take(INT, input);
        return read_long(curtok);
    case SYM:
        // tok = curtok;
        // take(SYM, input);
        return read_sym(curtok);
    case NIL:
        // tok = curtok;
        // take(NIL, input);
        token_delete(curtok);
        return obj_nil();
    case LPAREN:
        // tok = curtok;
        // take(LPAREN, input);
        token_delete(curtok);
        // curtok = lexan(input);
        return read_sexpr(input);
    case TRU:
    case FALS:
        tok = curtok;
        // take(curtok.type, input);
        obj *b = obj_bool(tok.val);
        token_delete(tok);
        return b;
    case RPAREN:
        return obj_err("unexpected ')'");
    default:
        return obj_err("unknown token '%s'", tok.val);
    }
}

/* EVAL --------------------------------------------------------------------- */

obj *eval(obj *o);

obj *eval_sexpr(obj *o) {

    /* evaluate all children */
    for (int i = 0; i < o->sexpr.count; i++) {
        o->sexpr.cell[i] = eval(o->sexpr.cell[i]);
        if (o->sexpr.cell[i]->type == OBJ_ERR) {
            return obj_take(o, i);
        }
    }

    /* first obj in list should be symbol */
    // obj *f = obj_pop(o, 0);
    // if (f->type != OBJ_SYM) {
    //     obj_delete(f);
    //     obj_delete(o);
    //     return obj_err("first obj in s-expr not a symbol");
    // }

    return o;
}

obj *eval(obj *o) {
    switch (o->type) {
    case OBJ_NUM:
    case OBJ_NIL:
    case OBJ_BOOL:
    case OBJ_SYM:
    case OBJ_CONS:
    case OBJ_ERR:
        return o;
    case OBJ_SEXPR:
        return eval_sexpr(o);
    }
    return NULL;
}

/* REPL --------------------------------------------------------------------- */
#include <editline/readline.h>

int main(void) {
    printf("clisp version 0.1\n\n");

    while (1) {
        char *input = readline("> ");
        add_history(input);

        lex_init(input);

        obj *o = read(input);
        obj_println(o);

        lex_cleanup();
        obj_delete(o);
        free(input);
    }

    return 0;
}
