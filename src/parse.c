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
    obj *o = obj_sym(p->curtok.val);
    token_delete(p->curtok);
    return o;
}

obj *read(parser *p);

obj *read_list(parser *p) {

    obj *list = obj_nil();
    while (p->peektok.type != TOK_RPAREN) {

        if (p->peektok.type == TOK_END) {
            obj_delete(list);
            return obj_err("expected ')'");
        }

        obj *car = read(p);

        if (p->peektok.type == TOK_DOT) {
            /* eat '.' */
            nexttok(p);
            token_delete(p->curtok);

            obj *cdr = read(p);
            if (p->peektok.type != TOK_RPAREN) {
                obj_delete(cdr);
                obj_delete(list);
                return obj_err("expected ')'");
            }

            if (list->nargs) {
                obj *cur = list;
                for (int i = 0; i < list->nargs - 1; i++) {
                    cur = obj_cdr(cur);
                }
                obj_delete(cur->cons->cdr); /* delete obj_nil() */
                cur->cons->cdr = obj_cons(car, cdr);
                list->nargs++;
            } else {
                obj_delete(list);
                list = obj_cons(car, cdr);
                list->nargs = 2;
            }

            nexttok(p);
            token_delete(p->curtok);

            return list;
        }

        if (list->nargs) {
            obj *cur = list;
            for (int i = 0; i < list->nargs - 1; i++) {
                cur = obj_cdr(cur);
            }
            cur->cons->cdr = obj_cons(car, cur->cons->cdr);
        } else {
            list = obj_cons(car, list);
        }

        list->nargs++;
    }

    /* eat ')' */
    nexttok(p);
    token_delete(p->curtok);

    return list;
}

obj *expand_quote(parser *p) {
    obj *quote = obj_keyword("quote");
    obj *res = obj_cons(quote, obj_cons(read(p), obj_nil()));
    res->nargs = 2;
    return res;
}

obj *read(parser *p) {
    nexttok(p);
    token tok;
    switch (p->curtok.type) {
    case TOK_INT:
    case TOK_RAT:
    case TOK_FLOAT:
        tok = p->curtok;
        obj *n = obj_num(p->curtok.val, p->curtok.type);
        token_delete(tok);
        return n;
    case TOK_SYM:
        return read_sym(p);
    case TOK_STR:
        tok = p->curtok;
        obj *s = obj_str(tok.val);
        token_delete(tok);
        return s;
    case TOK_NIL:
        token_delete(p->curtok);
        return obj_nil();
    case TOK_LPAREN:
        token_delete(p->curtok);
        return read_list(p);
    case TOK_CONST:
        tok = p->curtok;
        obj *c = obj_const(tok.val);
        token_delete(tok);
        return c;
    case TOK_TICK:
        token_delete(p->curtok);
        return expand_quote(p);
    case TOK_DEF:
    case TOK_COND:
    case TOK_LAMBDA:
    case TOK_QUOTE:
    case TOK_ELSE:
        tok = p->curtok;
        obj *k = obj_keyword(tok.val);
        token_delete(tok);
        return k;
    case TOK_RPAREN:
        token_delete(p->curtok);
        return obj_err("unexpected ')'");
    case TOK_DOT:
        token_delete(p->curtok);
        return obj_err("unexpected '.'");
    case TOK_ERR:
        tok = p->curtok;
        obj *res = obj_err(tok.val);
        token_delete(tok);
        return res;
    default:
        tok = p->curtok;
        obj *err = obj_err("unknown token '%s'", tok.val);
        token_delete(tok);
        return err;
    }
}
