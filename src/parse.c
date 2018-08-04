#include "parse.h"
#include "lex.h"
#include "object.h"
#include "global.h"

token curtok, peektok;

token nexttok(FILE *f) {
    curtok = peektok;
    peektok = lex(f);
    return curtok;
}

void parse_init(FILE *f) { peektok = lex(f); }
void parse_cleanup(void) { token_delete(peektok); }

obj *read_sym(token tok) {
    obj *o = obj_sym(tok.val);
    token_delete(tok);
    return o;
}

obj *read(FILE *f);

obj *read_list(FILE *f) {

    obj *list = obj_nil();
    while (peektok.type != TOK_RPAREN) {

        if (peektok.type == TOK_END) {
            obj_delete(list);
            return obj_err("expected ')'");
        }

        obj *car = read(f);

        if (peektok.type == TOK_DOT) {
            /* eat '.' */
            nexttok(f);
            token_delete(curtok);

            obj *cdr = read(f);
            if (peektok.type != TOK_RPAREN) {
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

            nexttok(f);
            token_delete(curtok);

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
    nexttok(f);
    token_delete(curtok);

    return list;
}

obj *expand_quote(FILE *f) {
    obj *quote = obj_keyword("quote");
    obj *res = obj_cons(quote, obj_cons(read(f), obj_nil()));
    res->nargs = 2;
    return res;
}

obj *read(FILE *f) {
    curtok = nexttok(f);
    token tok;
    switch (curtok.type) {
    case TOK_INT:
    case TOK_RAT:
    case TOK_FLOAT:
        tok = curtok;
        obj *n = obj_num(curtok.val, curtok.type);
        token_delete(tok);
        return n;
    case TOK_SYM:
        return read_sym(curtok);
    case TOK_STR:
        tok = curtok;
        obj *s = obj_str(tok.val);
        token_delete(tok);
        return s;
    case TOK_NIL:
        token_delete(curtok);
        return obj_nil();
    case TOK_LPAREN:
        token_delete(curtok);
        return read_list(f);
    case TOK_CONST:
        tok = curtok;
        obj *c = obj_const(tok.val);
        token_delete(tok);
        return c;
    case TOK_TICK:
        token_delete(curtok);
        return expand_quote(f);
    case TOK_DEF:
    case TOK_COND:
    case TOK_LAMBDA:
    case TOK_QUOTE:
        tok = curtok;
        obj *k = obj_keyword(tok.val);
        token_delete(tok);
        return k;
    case TOK_RPAREN:
        return obj_err("unexpected ')'");
    case TOK_DOT:
        return obj_err("unexpected '.'");
    case TOK_ERR:
        tok = curtok;
        obj *res = obj_err(tok.val);
        token_delete(tok);
        return res;
    default:
        return obj_err("unknown token '%s'", curtok.val);
    }
}
