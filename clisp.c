#include "src/ast.h"
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
    if (peektok.type == type) {
        lex_advance(input);
        return 1;
    }
    // fprintf(stderr, "expected token type %d, got %d\n", type, peektok.type);
    return 0;
}

// TODO: parser errors
ast *parse(char *input) {
    switch (peektok.type) {
    case INT:
        match(INT, input);
        return ast_new(AST_NUM, curtok, 0, 0);
    case SYM:
        match(SYM, input);
        return ast_new(AST_SYM, curtok, 0, 0);
    case LPAREN:
        match(LPAREN, input);
        token lparen = curtok;

        ast *tmp_children[MAXCHILDREN];
        int childpos = 0;
        while (peektok.type != RPAREN) {
            tmp_children[childpos++] = parse(input);
        }

        if (match(RPAREN, input))
            token_delete(curtok);

        ast **children = malloc(sizeof(ast *) * childpos);
        memcpy(children, tmp_children, sizeof(ast *) * childpos);

        return ast_new(AST_EXP, lparen, childpos, children);
    default:
        fprintf(stderr, "error: returning null from ast *parse()...\n");
        return NULL;
    }
}

/* ============= EVALUATION ============== */
object *object_add(object *o, object *x) {
    list_insert(o->cell, x, list_size(o->cell));
    return o;
}

object *read_long(ast *a) {
    errno = 0;
    long x = strtol(a->tok.val, NULL, 10);
    return errno != ERANGE ? object_long(x) : object_error("bad number syntax");
}

object *object_read(ast *root);

object *read_sexp(ast *a) {
    object *x = object_sexp();
    for (int i = 0; i < a->numchldrn; i++) {
        x = object_add(x, object_read(a->children[i]));
    }
    return x;
}

object *object_read(ast *root) {
    if (root->type == AST_NUM) {
        return read_long(root);
    }

    if (root->type == AST_SYM) {
        return object_sym(root->tok.val);
    }

    if (root->type == AST_EXP) {
        return read_sexp(root);
    }

    fprintf(stderr, "error: returning null from object_read()\n");
    return NULL;
}

object *eval_op(object *x, char *op, object *y) {

    if (x->type == OBJ_ERROR)
        return x;
    if (y->type == OBJ_ERROR)
        return y;

    if (strcmp("+", op) == 0)
        return object_long(x->lval + y->lval);
    if (strcmp("-", op) == 0)
        return object_long(x->lval - y->lval);
    if (strcmp("*", op) == 0)
        return object_long(x->lval * y->lval);
    if (strcmp("/", op) == 0) {
        return y->lval == 0 ? object_error("division by zero")
                            : object_long(x->lval / y->lval);
    }
    if (strcmp("%", op) == 0) {
        return y->lval == 0 ? object_error("division by zero")
                            : object_long(x->lval % y->lval);
    }
    return object_error("unknown operator");
}

object *object_take(object *o, int index) {
    object *p = list_remove(o->cell, index);
    object_delete(o);
    return p;
}

object *builtin_op(object *o, char *sym) {

    /* Ensure all arguments are long */
    for (int i = 0; i < list_size(o->cell); i++) {
        object *p = list_at(o->cell, i);
        if (p->type != OBJ_LONG) {
            object_delete(o);
            return object_error("cannot operate on non-number");
        }
    }

    /* Pop the first element */
    object *x = list_remove(o->cell, 0);

    /* If no arguments and sum then perform unary neg */
    if ((strcmp(sym, "-") == 0) && list_size(o->cell) == 0) {
        x->lval = -x->lval;
    }

    /* While there are still elements remaining */
    while (list_size(o->cell) > 0) {
        object *y = list_remove(o->cell, 0);

        if (strcmp(sym, "+") == 0)
            x->lval += y->lval;
        if (strcmp(sym, "-") == 0)
            x->lval -= y->lval;
        if (strcmp(sym, "*") == 0)
            x->lval *= y->lval;
        if (strcmp(sym, "/") == 0) {
            if (y->lval == 0) {
                object_delete(x);
                object_delete(y);
                x = object_error("division by zero");
                break;
            }
            x->lval /= y->lval;
        }

        object_delete(y);
    }

    object_delete(o);

    return x;
}

object *eval_sexp(object *o);

object *object_eval(object *o) {
    if (o->type == OBJ_SEXP) {
        return eval_sexp(o);
    }
    return o;
}

object *eval_sexp(object *o) {
    /* Evaluate children */
    for (int i = 0; i < list_size(o->cell); i++) {
        object *p = list_at(o->cell, i);
        list_replace(o->cell, object_eval(p), i);
    }

    /* Error checking */
    for (int i = 0; i < list_size(o->cell); i++) {
        object *p = list_at(o->cell, i);
        if (p->type == OBJ_ERROR) {
            return object_take(o, i);
        }
    }

    /* Empty list */
    if (list_size(o->cell) == 0)
        return o;

    /* Single Expression */
    if (list_size(o->cell) == 1)
        return object_take(o, 0);

    /* Ensure first element is symbol */
    object *p = list_remove(o->cell, 0);

    if (p->type != OBJ_SYM) {
        object_delete(o);
        object_delete(p);
        return object_error("s-expression doesn't start with a symbol");
    }

    object *res = builtin_op(o, p->sym);

    object_delete(p);

    return res;
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
        peektok = lexan(input);

        ast *prog = parse(input);
        // ast_print(prog, 0);

        object *o = object_read(prog);

        object *res = object_eval(o);
        object_println(res);

        object_delete(res);
        ast_delete(prog);
        free(input);
    }

    return 0;
}