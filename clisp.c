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
    DEFINE,
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
    if (strcmp(sym, "define") == 0)
        return token_new(DEFINE, sym);
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

obj *read_sexpr(char *input) {
    obj *o = obj_sexpr();
    while (peektok.type != RPAREN) {
        if (curtok.type == END) {
            obj_delete(o);
            return obj_err("unexpected eof, expected ')'");
        }
        obj_add(o, read(input));
    }
    nexttok(input); /* take ')' */
    return o;
}

obj *read_qexpr(char *input);

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
        return read_sexpr(input);
    case TICK:
        token_delete(curtok);
        return obj_qexpr(read(input));
    case TRU:
    case FALS:
        tok = curtok;
        obj *b = obj_bool(tok.val);
        token_delete(tok);
        return b;
    case DEFINE:
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

int context;

void eval_init() { context = 0; }

obj *eval_define(env *e, obj *args) {
    LASSERT(args, args->sexpr->count == 2,
            "incorrect number of arguments for define. expected %d, got %d", 2,
            args->sexpr->count);
    LASSERT(args, args->sexpr->cell[0]->type == OBJ_SYM,
            "first argument in define should be a symbol");

    obj *k = obj_pop(args, 0);
    obj *v = eval(e, obj_pop(args, 0));

    env_insert(e, k, v);

    obj_delete(k);
    obj_delete(v);
    obj_delete(args);
    return obj_nil();
}

obj *eval_quote(env *e, obj *args) {
    LASSERT(args, args->sexpr->count == 1,
            "incorrect number of args for quote. expected %d, got %d", 1,
            args->sexpr->count);
    return obj_qexpr(obj_take(args, 0));
}

obj *eval_keyword(env *e, obj *o) {
    obj *res;
    obj *k = obj_pop(o, 0);
    if (strcmp(k->keyword, "quote") == 0)
        res = eval_quote(e, o);
    else if (strcmp(k->keyword, "define") == 0) {
        res = context == 0 ? eval_define(e, o)
                           : obj_err("improper context for define");
    }
    obj_delete(k);
    return res;
}

obj *eval_sexpr(env *e, obj *o) {

    /* check for keyword */
    if (o->sexpr->cell[0]->type == OBJ_KEYWORD) {
        return eval_keyword(e, o);
    }

    context++;

    /* evaluate all children */
    for (int i = 0; i < o->sexpr->count; i++) {
        o->sexpr->cell[i] = eval(e, o->sexpr->cell[i]);
        if (o->sexpr->cell[i]->type == OBJ_ERR) {
            return obj_take(o, i);
        }
    }

    /* first obj in list should be function */
    obj *f = obj_pop(o, 0);
    if (f->type != OBJ_FUN) {
        obj_delete(f);
        obj_delete(o);
        return obj_err("first obj in s-expr not a function");
    }

    obj *res = f->fun->proc(e, o);

    obj_delete(f);

    return res;
}

obj *eval(env *e, obj *o) {
    switch (o->type) {
    case OBJ_NUM:
    case OBJ_NIL:
    case OBJ_BOOL:
    case OBJ_CONS:
    case OBJ_ERR:
    case OBJ_FUN:
    case OBJ_QEXPR:
        return o;
    case OBJ_SEXPR:
        return eval_sexpr(e, o);
    case OBJ_SYM:
        return env_lookup(e, o);
    case OBJ_KEYWORD:;
        obj *err = obj_err("invalid syntax %s", o->keyword);
        obj_delete(o);
        return err;
    }
    return NULL;
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
        // printf("type = %s\n", obj_typename(o->type));
        obj_println(o);

        lex_cleanup();
        obj_delete(o);
        free(input);
        // env_print(global);
    }

    cleanup();

    return 0;
}
