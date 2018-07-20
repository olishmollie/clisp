#include "src/global.h"
#include "src/object.h"
#include "src/table.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFSIZE 99
#define MAXCHILDREN 10

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
token curtok;

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
    curtok = lexan(input);
}

void lex_cleanup(void) { token_delete(curtok); /* delete 'end' token */ }

/* PARSING ------------------------------------------------------------------ */

void lex_advance(char *input) { curtok = lexan(input); }

int take(token_t type, char *input) {
    if (curtok.type == type) {
        lex_advance(input);
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

obj *read_list(char *input) {
    if (curtok.type == END)
        return obj_err("unexpected eof, expected ')'");
    if (curtok.type == RPAREN) {
        token tok = curtok;
        take(RPAREN, input);
        token_delete(tok);
        return obj_nil();
    }
    if (curtok.type == NIL) {
        take(NIL, input);
        return obj_nil();
    }

    obj *car = read(input);
    obj *cdr = read_list(input);

    return obj_cons(car, cdr);
}

obj *read(char *input) {
    token tok;
    switch (curtok.type) {
    case INT:
        tok = curtok;
        take(INT, input);
        return read_long(tok);
    case SYM:
        tok = curtok;
        take(SYM, input);
        return read_sym(tok);
    case NIL:
        tok = curtok;
        take(NIL, input);
        token_delete(tok);
        return obj_nil();
    case LPAREN:
        tok = curtok;
        take(LPAREN, input);
        token_delete(tok);
        return read_list(input);
    case TRU:
    case FALS:
        tok = curtok;
        take(curtok.type, input);
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

obj *builtin_plus(obj *x, obj *y) {
    obj *res = obj_num(x->num.val + y->num.val);
    obj_delete(x);
    obj_delete(y);
    return res;
}

obj *builtin_minus(obj *x, obj *y) {
    obj *res = obj_num(x->num.val - y->num.val);
    obj_delete(x);
    obj_delete(y);
    return res;
}

obj *builtin_times(obj *x, obj *y) {
    obj *res = obj_num(x->num.val * y->num.val);
    obj_delete(x);
    obj_delete(y);
    return res;
}

obj *builtin_divide(obj *x, obj *y) {
    obj *res = y->num.val ? obj_num(x->num.val / y->num.val)
                          : obj_err("division by zero");
    obj_delete(x);
    obj_delete(y);
    return res;
}

obj *builtin_remainder(obj *x, obj *y) {
    obj *res = y->num.val ? obj_num(x->num.val % y->num.val)
                          : obj_err("division by zero");
    obj_delete(x);
    obj_delete(y);
    return res;
}

obj *builtin_cons(obj *x, obj *y) {
    x = eval(x);
    y = eval(y);
    return obj_cons(x, y);
}

obj *builtin_atom(obj *l) {
    obj *res = l->type != OBJ_CONS ? obj_bool("true") : obj_bool("false");
    obj_delete(l);
    return res;
}

obj *builtin_eq(obj *x, obj *y) {
    if (!builtin_atom(x) || !builtin_atom(y)) {
        obj_delete(x);
        obj_delete(y);
        return obj_err("parameter passed to eq is not an atom");
    }
    if (x->type != y->type)
        return obj_bool("false");

    switch (x->type) {
    case OBJ_NUM:
        return x->num.val == y->num.val ? obj_bool("true") : obj_bool("false");
    case OBJ_SYM:
        break;
    case OBJ_NIL:
    case OBJ_BOOL:
        return obj_bool("true");
    default:
        return obj_bool("false");
    }

    return obj_bool("false");
}

obj *builtin_car(obj *l) { return _car(l); }
obj *builtin_cdr(obj *l) { return _cdr(l); }

obj *builtin_op(obj *args, obj *f) {

    /* ensure all args are numbers */
    obj *cur = args;
    while (cur->type != OBJ_NIL) {
        if (_car(cur)->type != OBJ_NUM) {
            // TODO: handle printing symbol or error in error msg
            obj *err = obj_err("cannot operate on non-number");
            obj_delete(f);
            obj_delete(args);
            return err;
        }
        cur = _cdr(cur);
    }

    printf("got past number check\n");
    /* if no args and sum then unary negation */
    if ((strcmp(f->sym.name, "-") == 0) && _cdr(args)->type == OBJ_NIL) {
        obj *res = obj_pop(&args);
        res->num.val = -res->num.val;
        obj_delete(args);
        obj_delete(f);
        return res;
    }

    /* get first arg */
    obj *x = obj_pop(&args);

    while (args->type != OBJ_NIL) {

        obj *y = obj_pop(&args);

        if (strcmp(f->sym.name, "+") == 0)
            x = builtin_plus(x, y);
        if (strcmp(f->sym.name, "-") == 0)
            x = builtin_minus(x, y);
        if (strcmp(f->sym.name, "*") == 0)
            x = builtin_times(x, y);
        if (strcmp(f->sym.name, "/") == 0) {
            if (!y->num.val) {
                obj_delete(x);
                obj_delete(y);
                x = obj_err("division by zero");
                break;
            }
            x = builtin_divide(x, y);
        }
        if (strcmp(f->sym.name, "%") == 0) {
            if (!y->num.val) {
                obj_delete(x);
                obj_delete(y);
                x = obj_err("division by zero");
                break;
            }
            x = builtin_remainder(x, y);
        }
    }

    obj_delete(f);
    obj_delete(args);

    return x;
}

obj *eval_cons(obj *o) {

    /* evaluate all children */
    obj *cur = o;
    while (cur->type != OBJ_NIL) {
        if (cur->type == OBJ_ERR) {
            obj *err = obj_err(cur->err);
            obj_delete(o);
            return err;
        }
        cur->cons.car = eval(_car(cur));
        cur = _cdr(cur);
    }

    /* first obj in list should be symbol */
    if (_car(o)->type != OBJ_SYM) {
        obj_delete(o);
        return obj_err("first obj in s-expr not a symbol");
    }

    obj *f = obj_pop(&o);
    if (table_lookup(f->sym.name) == -1) {
        obj *err = obj_err("symbol '%s' is not defined", f->sym.name);
        obj_delete(o);
        obj_delete(f);
        return err;
    }

    obj *res = builtin_op(o, f);

    return res;
}

obj *eval(obj *o) {
    switch (o->type) {
    case OBJ_NUM:
    case OBJ_NIL:
    case OBJ_BOOL:
        return o;
    case OBJ_SYM:
        if (table_lookup(o->sym.name) == -1) {
            obj_delete(o);
            return obj_err("symbol is not defined");
        }
        return o;
    default:
        return eval_cons(o);
    }
    return NULL;
}

/* REPL --------------------------------------------------------------------- */
#include <editline/readline.h>

int main(void) {
    table_init();
    printf("clisp version 0.1\n\n");

    while (1) {
        char *input = readline("> ");
        add_history(input);

        lex_init(input);

        obj *o = eval(read(input));
        obj_println(o);

        lex_cleanup();
        obj_delete(o);
        free(input);
    }

    return 0;
}