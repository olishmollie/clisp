#include "parse.h"
#include "lex.h"
#include "object.h"
#include "global.h"

#include <stdlib.h>

parser *parser_new(FILE *f) {
    lexer *l = lexer_new(f);
    parser *p = malloc(sizeof(parser));
    p->l = l;
    p->peektok = lex(l);
    return p;
}

void parser_delete(parser *p) {
    token_delete(p->peektok);
    lexer_delete(p->l);
    free(p);
}

token nexttok(parser *p) {
    p->curtok = p->peektok;
    p->peektok = lex(p->l);
    return p->curtok;
}

obj *read_sym(parser *p) {
    obj *res;

    if (strcmp(p->curtok.val, "quote") == 0)
        res = quote_sym;
    else if (strcmp(p->curtok.val, "define") == 0)
        res = define_sym;
    else if (strcmp(p->curtok.val, "set!") == 0)
        res = set_sym;
    else if (strcmp(p->curtok.val, "if") == 0)
        res = if_sym;
    else {
        res = mk_sym(p->curtok.val);
    }

    token_delete(p->curtok);

    return res;
}

obj *read_constant(parser *p) {
    obj *res;
    if (strcmp(p->curtok.val, "#t") == 0)
        res = true;
    else if (strcmp(p->curtok.val, "#f") == 0)
        res = false;
    else {
        res = mk_const(p->curtok.val);
    }
    token_delete(p->curtok);

    return res;
}

obj *read_num(parser *p) {
    obj *res = mk_num(p->curtok.val);
    token_delete(p->curtok);
    return res;
}

obj *read_string(parser *p) {
    obj *res = mk_string(p->curtok.val);
    token_delete(p->curtok);
    return res;
}

obj *read(parser *p);

obj *read_list(parser *p) {

    obj *car_obj;
    obj *cdr_obj;

    if (p->peektok.type == TOK_END) {
        return mk_err("expected ')'");
    }

    if (p->peektok.type == TOK_RPAREN) {
        /* eat ')' */
        nexttok(p);
        token_delete(p->curtok);
        return the_empty_list;
    }

    car_obj = read(p);

    if (p->peektok.type == TOK_DOT) {
        /* eat '.' */
        nexttok(p);
        token_delete(p->curtok);

        cdr_obj = read(p);
        if (p->peektok.type != TOK_RPAREN) {
            obj_delete(cdr_obj);
            return mk_err("expected ')'");
        }

        nexttok(p);
        token_delete(p->curtok);

        return mk_cons(car_obj, cdr_obj);
    }

    cdr_obj = read_list(p);

    return mk_cons(car_obj, cdr_obj);
}

obj *expand_quote(parser *p) {
    return mk_cons(quote_sym, mk_cons(read(p), the_empty_list));
}

obj *read(parser *p) {
    nexttok(p);
    token tok;
    switch (p->curtok.type) {
    case TOK_INT:
    case TOK_RAT:
    case TOK_FLOAT:
        return read_num(p);
    case TOK_SYM:
        return read_sym(p);
    case TOK_CONST:
        return read_constant(p);
    case TOK_STR:
        return read_string(p);
    case TOK_LPAREN:
        token_delete(p->curtok);
        return read_list(p);
    case TOK_TICK:
        token_delete(p->curtok);
        return expand_quote(p);
    case TOK_RPAREN:
        token_delete(p->curtok);
        return mk_err("unexpected ')'");
    case TOK_DOT:
        token_delete(p->curtok);
        return mk_err("unexpected '.'");
    case TOK_ERR:
        tok = p->curtok;
        obj *res = mk_err(tok.val);
        token_delete(tok);
        return res;
    default:
        tok = p->curtok;
        obj *err = mk_err("unknown token '%s'", tok.val);
        token_delete(tok);
        return err;
    }
}
