#include "src/env.h"
#include "src/errors.h"
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
    DEF,
    QUOTE,
    TICK,
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
    if (strcmp(sym, "def") == 0)
        return token_new(DEF, sym);
    if (strcmp(sym, "quote") == 0)
        return token_new(QUOTE, sym);

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
    } else if (cur == '\'') {
        nextchar(input);
        return token_new(TICK, "'");
    } else
        return lexsymbol(input);
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

void count_args(obj *o) {
    obj *cur = o;
    int nargs = 0;
    while (cur->type != OBJ_NIL) {
        nargs++;
        cur = obj_cdr(cur);
    }
    o->count = nargs;
}

obj *read_list(char *input) {
    if (peektok.type == END)
        return obj_cons(obj_err("unexpected eof, expected ')'"), obj_nil());
    if (peektok.type == RPAREN) {
        nexttok(input);
        return obj_nil();
    }

    obj *car = read(input);
    obj *cdr = read_list(input);

    obj *cons = obj_cons(car, cdr);

    count_args(cons);

    return cons;
}

obj *expand_quote(char *input) {
    obj *quote = obj_keyword("quote");
    obj *data = obj_cons(read(input), obj_nil());
    count_args(data);
    obj *res = obj_cons(quote, data);
    return res;
}

obj *read(char *input) {
    curtok = nexttok(input);
    token tok;
    switch (curtok.type) {
    case INT:
        return read_long(curtok);
    case SYM:
        return read_sym(curtok);
    case NIL:
        token_delete(curtok);
        return obj_nil();
    case LPAREN:
        token_delete(curtok);
        return read_list(input);
    case TRU:
    case FALS:
        tok = curtok;
        obj *b = obj_bool(strcmp(tok.val, "true") == 0 ? TRUE : FALSE);
        token_delete(tok);
        return b;
    case TICK:
        token_delete(curtok);
        return expand_quote(input);
    case DEF:
    case QUOTE:
        tok = curtok;
        obj *k = obj_keyword(tok.val);
        token_delete(tok);
        return k;
    case RPAREN:
        return obj_err("unexpected ')'");
    default:
        return obj_err("unknown token '%s'", curtok.val);
    }
}

/* EVAL --------------------------------------------------------------------- */

obj *eval(env *, obj *);

int nestlevel;

void eval_init() { nestlevel = 0; }

obj *eval_quote(env *e, obj *args) {
    NARGCHECK(args, "quote", 1);
    obj *quote = obj_popcar(&args);
    obj_delete(args);
    return quote;
}

obj *eval_def(env *e, obj *args) {
    NARGCHECK(args, "define", 2);
    CASSERT(args, obj_car(args)->type == OBJ_SYM,
            "first arg to define must be symbol");

    obj *k = obj_popcar(&args);
    obj *v = eval(e, obj_popcar(&args));
    env_insert(e, k, v);

    obj_delete(k);
    obj_delete(v);
    obj_delete(args);
    return obj_nil();
}

obj *eval_keyword(env *e, obj *o) {
    obj *res;
    obj *k = obj_popcar(&o);
    ERRCHECK(o);
    if (strcmp(k->keyword, "quote") == 0)
        res = eval_quote(e, o);
    else if (strcmp(k->keyword, "def") == 0) {
        res = nestlevel == 0 ? eval_def(e, o)
                             : obj_err("improper context for define");
    }
    obj_delete(k);
    return res;
}

obj *eval_sexpr(env *e, obj *o) {
    /* check for keyword */
    if (obj_car(o)->type == OBJ_KEYWORD) {
        return eval_keyword(e, o);
    }

    nestlevel++;

    /* evaluate all children */
    obj *cur = o;
    for (int i = 0; i < o->count; i++) {
        cur->cons->car = eval(e, cur->cons->car);
        cur = obj_cdr(cur);
    }

    ERRCHECK(o);
    CASSERT(o, obj_car(o)->type == OBJ_FUN,
            "first obj in s-expr is not a function");

    obj *f = obj_popcar(&o);
    obj *res = f->fun->proc(e, o);

    obj_delete(f);

    return res;
}

obj *eval(env *e, obj *o) {
    if (o->type == OBJ_SYM)
        return env_lookup(e, o);
    if (o->type == OBJ_CONS)
        return eval_sexpr(e, o);
    if (o->type == OBJ_KEYWORD) {
        obj *err = obj_err("invalid syntax %s", o->keyword);
        obj_delete(o);
        return err;
    }
    return o;
}

/* REPL --------------------------------------------------------------------- */
#include <editline/readline.h>

env *global;
void cleanup() { env_delete(global); }

int main(void) {
    printf("clisp version 0.1\n\n");
    global = env_init();

    while (1) {
        char *input = readline("> ");
        add_history(input);

        lex_init(input);
        eval_init();

        obj *o = eval(global, read(input));
        // obj *o = read(input);
        obj_println(o);
        // printf("o->count = %d\n", o->count);

        lex_cleanup();
        obj_delete(o);
        free(input);
        // env_print(global);
    }

    cleanup();

    return 0;
}
