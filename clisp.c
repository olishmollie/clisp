#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFSIZE 99
#define EOS '\0'

/* errors ------------------------------------------------------------------ */

#define CASSERT(args, cond, msg)                                               \
    {                                                                          \
        if (!(cond)) {                                                         \
            obj_delete(args);                                                  \
            return obj_err(msg);                                               \
        }                                                                      \
    }

#define NARGCHECK(args, fun, num)                                              \
    {                                                                          \
        if (args->list->count != num) {                                        \
            obj *err = obj_err(                                                \
                "incorrect number of arguments to %s. expected %d, got %d",    \
                fun, num, args->list->count);                                  \
            obj_delete(args);                                                  \
            return err;                                                        \
        }                                                                      \
    }

#define TARGCHECK(args, fun, typ)                                              \
    {                                                                          \
        obj *cur = args->list->head;                                           \
        for (int i = 0; i < args->list->count; i++) {                          \
            if (obj_car(cur)->type != typ) {                                   \
                obj *err = obj_err("argument to %s is not of type %s, got %s", \
                                   fun, obj_typename(typ),                     \
                                   obj_typename(obj_car(cur)->type));          \
                obj_delete(args);                                              \
                return err;                                                    \
            }                                                                  \
            cur = obj_cdr(cur);                                                \
        }                                                                      \
    }

// TODO: visit every node in error check
#define ERRCHECK(args)                                                         \
    {                                                                          \
        obj *cur = args->list->head;                                           \
        for (int i = 0; i < args->list->count; i++) {                          \
            if (obj_car(cur)->type == OBJ_ERR) {                               \
                obj *err = obj_err(obj_car(cur)->err);                         \
                obj_delete(args);                                              \
                return err;                                                    \
            } else if (obj_cdr(cur)->type == OBJ_ERR) {                        \
                obj *err = obj_err(obj_cdr(cur)->err);                         \
                obj_delete(args);                                              \
                return err;                                                    \
            }                                                                  \
            cur = obj_cdr(cur);                                                \
        }                                                                      \
    }

/* LEXING ------------------------------------------------------------------- */

typedef enum {
    TOK_INT,
    TOK_FLOAT,
    TOK_RAT,
    TOK_SYM,
    TOK_TRUE,
    TOK_FALSE,
    TOK_NIL,
    TOK_LPAREN,
    TOK_RPAREN,
    TOK_DOT,
    TOK_DEF,
    TOK_COND,
    TOK_QUOTE,
    TOK_TICK,
    TOK_LAMBDA,
    TOK_END
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
    return token_new(TOK_INT, num);
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
        return token_new(TOK_NIL, sym);
    if (strcmp(sym, "true") == 0)
        return token_new(TOK_TRUE, sym);
    if (strcmp(sym, "false") == 0)
        return token_new(TOK_FALSE, sym);
    if (strcmp(sym, "def") == 0)
        return token_new(TOK_DEF, sym);
    if (strcmp(sym, "quote") == 0)
        return token_new(TOK_QUOTE, sym);
    if (strcmp(sym, "cond") == 0)
        return token_new(TOK_COND, sym);
    if (strcmp(sym, "lambda") == 0)
        return token_new(TOK_LAMBDA, sym);

    return token_new(TOK_SYM, sym);
}

token lexan(char *input) {
    skipspaces(input);
    int cur = curchar(input);
    if (cur == EOS)
        return token_new(TOK_END, "end");
    else if (isdigit(cur))
        return lexdigit(input);
    else if (cur == '(') {
        nextchar(input);
        return token_new(TOK_LPAREN, "(");
    } else if (cur == ')') {
        nextchar(input);
        return token_new(TOK_RPAREN, ")");
    } else if (cur == '.') {
        nextchar(input);
        return token_new(TOK_DOT, ".");
    } else if (cur == '\'') {
        nextchar(input);
        return token_new(TOK_TICK, "'");
    } else
        return lexsymbol(input);
}

void lex_init(char *input) {
    numtok = 0;
    pos = 0;
    peektok = lexan(input);
}

void lex_cleanup(void) { token_delete(peektok); }

/* objects ----------------------------------------------------------------- */

typedef struct {
    long val;
} num_t;

typedef struct obj obj;

typedef struct {
    obj *car;
    obj *cdr;
} cons_t;

typedef struct {
    obj *head;
    int count;
} list_t;

typedef struct env env;
typedef obj *(*builtin)(env *, obj *);
typedef struct {
    char *name;
    builtin proc;
} builtin_t;

typedef struct {
    env *e;
    obj *params;
    obj *body;
} lambda_t;

typedef enum {
    OBJ_NUM,
    OBJ_SYM,
    OBJ_CONS,
    OBJ_LIST,
    OBJ_BOOL,
    OBJ_BUILTIN,
    OBJ_LAMBDA,
    OBJ_KEYWORD,
    OBJ_NIL,
    OBJ_ERR
} obj_t;

typedef enum { TRUE, FALSE } bool_t;

struct obj {
    obj_t type;
    union {
        num_t *num;
        char *sym;
        cons_t *cons;
        list_t *list;
        bool_t bool;
        builtin_t *bltin;
        lambda_t *lambda;
        char *keyword;
        char *err;
    };
};

typedef struct env {
    struct env *parent;
    int count;
    char **syms;
    obj **vals;
} env;

env *env_new(void) {
    env *e = malloc(sizeof(env));
    e->count = 0;
    e->parent = NULL;
    e->syms = NULL;
    e->vals = NULL;
    return e;
}

obj *obj_cpy(obj *);
obj *obj_err(char *, ...);
void obj_delete(obj *);

obj *env_lookup(env *e, obj *k) {
    /* search local env */
    for (int i = 0; i < e->count; i++) {
        if (strcmp(e->syms[i], k->sym) == 0)
            return obj_cpy(e->vals[i]);
    }

    /* search parent env */
    if (e->parent)
        return env_lookup(e->parent, k);

    return obj_err("unbound symbol '%s'", k->sym);
}

void env_insert(env *e, obj *k, obj *v) {

    /* Overwrite an exisiting symbol if exists */
    for (int i = 0; i < e->count; i++) {
        if (strcmp(e->syms[i], k->sym) == 0) {
            obj_delete(e->vals[i]);
            e->vals[i] = obj_cpy(v);
            return;
        }
    }

    /* Insert new object */
    ++e->count;
    e->vals = realloc(e->vals, sizeof(obj *) * e->count);
    e->syms = realloc(e->syms, sizeof(char *) * e->count);

    e->vals[e->count - 1] = obj_cpy(v);
    e->syms[e->count - 1] = malloc(sizeof(char) * (strlen(k->sym) + 1));
    strcpy(e->syms[e->count - 1], k->sym);
}

void env_delete(env *e) {
    if (e) {
        for (int i = 0; i < e->count; i++) {
            free(e->syms[i]);
            obj_delete(e->vals[i]);
        }
        free(e->syms);
        free(e->vals);
        free(e);
    }
}

void obj_println(obj *);

void env_print(env *e) {
    printf("{\n");
    for (int i = 0; i < e->count; i++) {
        printf("\t%s: ", e->syms[i]);
        obj_println(e->vals[i]);
    }
    printf("}\n");
}

/* interior types ---------------------------------------------------------- */

num_t *mk_num(long val) {
    num_t *n = malloc(sizeof(num_t));
    n->val = val;
    return n;
}

cons_t *mk_cons(obj *car, obj *cdr) {
    cons_t *c = malloc(sizeof(cons_t));
    c->car = car;
    c->cdr = cdr;
    return c;
}

obj *obj_nil(void);

list_t *mk_list(void) {
    list_t *l = malloc(sizeof(list_t));
    l->head = obj_nil();
    l->count = 0;
    return l;
}

builtin_t *mk_builtin(char *name, builtin bltin) {
    builtin_t *b = malloc(sizeof(builtin_t));
    b->proc = bltin;
    b->name = malloc(sizeof(char) * (strlen(name) + 1));
    strcpy(b->name, name);
    return b;
}

lambda_t *mk_lambda(obj *params, obj *body) {
    lambda_t *l = malloc(sizeof(lambda_t));
    l->e = env_new();
    l->params = params;
    l->body = body;
    return l;
}

/* obj types --------------------------------------------------------------- */

obj *obj_num(long val) {
    obj *o = malloc(sizeof(obj));
    o->type = OBJ_NUM;
    o->num = mk_num(val);
    return o;
}

obj *obj_sym(char *name) {
    obj *o = malloc(sizeof(obj));
    o->type = OBJ_SYM;
    o->sym = malloc(sizeof(char) * (strlen(name) + 1));
    strcpy(o->sym, name);
    return o;
}

obj *obj_cons(obj *car, obj *cdr) {
    obj *o = malloc(sizeof(obj));
    o->type = OBJ_CONS;
    o->cons = mk_cons(car, cdr);
    return o;
}

obj *obj_list(void) {
    obj *o = malloc(sizeof(obj));
    o->type = OBJ_LIST;
    o->list = mk_list();
    return o;
}

obj *obj_builtin(char *name, builtin proc) {
    obj *o = malloc(sizeof(obj));
    o->type = OBJ_BUILTIN;
    o->bltin = mk_builtin(name, proc);
    return o;
}

obj *obj_lambda(obj *params, obj *body) {
    obj *o = malloc(sizeof(obj));
    o->type = OBJ_LAMBDA;
    o->lambda = mk_lambda(params, body);
    return o;
}

obj *obj_bool(bool_t b) {
    obj *o = malloc(sizeof(obj));
    o->type = OBJ_BOOL;
    o->bool = b;
    return o;
}

obj *obj_keyword(char *name) {
    obj *o = malloc(sizeof(obj));
    o->type = OBJ_KEYWORD;
    o->keyword = malloc(sizeof(char) * (strlen(name) + 1));
    strcpy(o->keyword, name);
    return o;
}

obj *obj_nil(void) {
    obj *o = malloc(sizeof(obj));
    o->type = OBJ_NIL;
    return o;
}

obj *obj_err(char *fmt, ...) {
    obj *o = malloc(sizeof(obj));
    o->type = OBJ_ERR;

    va_list args;
    va_start(args, fmt);
    o->err = malloc(sizeof(char) * 512);
    vsnprintf(o->err, 511, fmt, args);
    va_end(args);

    return o;
}

int obj_isatom(obj *o) { return o->type != OBJ_CONS && o->type != OBJ_LIST; }

char *obj_typename(obj_t type) {
    switch (type) {
    case OBJ_NUM:
        return "number";
    case OBJ_SYM:
        return "symbol";
    case OBJ_CONS:
        return "cons";
    case OBJ_LIST:
        return "list";
    case OBJ_BOOL:
        return "bool";
    case OBJ_BUILTIN:
        return "function";
    case OBJ_LAMBDA:
        return "lambda";
    case OBJ_KEYWORD:
        return "keyword";
    case OBJ_NIL:
        return "nil";
    case OBJ_ERR:
        return "error";
    default:
        return "unknown";
    }
}

/* list fns ---------------------------------------------------------------- */

obj *obj_car(obj *o) {
    if (o->type == OBJ_LIST) {
        return o->list->head->cons->car;
    }
    return o->cons->car;
}

obj *obj_cdr(obj *o) {
    if (o->type == OBJ_LIST) {
        return o->list->head->cons->cdr;
    }
    return o->cons->cdr;
}

obj *obj_cadr(obj *o) { return obj_car(obj_cdr(o)); }

obj *obj_add(obj *l, obj *x) {
    if (l->list->count) {
        obj *cur = l->list->head;
        while (obj_cdr(cur)->type != OBJ_NIL) {
            cur = obj_cdr(cur);
        }
        cur->cons->cdr = obj_cons(x, cur->cons->cdr);
    } else {
        l->list->head = obj_cons(x, l->list->head);
    }
    l->list->count++;
    return x;
}

obj *obj_popcar(obj *o) {
    obj *car;
    if (o->type == OBJ_CONS) {
        car = obj_car(o);
        o->cons = mk_cons(obj_cdr(o), obj_nil());
    } else if (o->type == OBJ_LIST) {
        car = obj_car(o->list->head);
        o->list->head = obj_cdr(o->list->head);
        o->list->count--;
    }
    return car;
}

obj *obj_popcdr(obj *o) {
    obj *cdr;
    if (o->type == OBJ_CONS) {
        cdr = obj_cdr(o);
        o->cons = mk_cons(obj_car(o), obj_nil());
    } else if (o->type == OBJ_LIST) {
        cdr = obj_cdr(o->list->head);
        o->list->head = obj_cons(obj_car(o->list->head), obj_nil());
        o->list->count--;
    }
    return cdr;
}

/* copying ----------------------------------------------------------------- */

obj *cpy_list(obj *l) {
    obj *res = obj_list();
    obj *cur = l->list->head;
    while (cur->type != OBJ_NIL) {
        obj_add(res, obj_cpy(obj_car(cur)));
        cur = obj_cdr(cur);
    }
    return res;
}

obj *obj_cpy(obj *o) {
    obj *res;
    switch (o->type) {
    case OBJ_NUM:
        res = obj_num(o->num->val);
        break;
    case OBJ_SYM:
        res = obj_sym(o->sym);
        break;
    case OBJ_CONS:
        res = obj_cons(obj_cpy(o->cons->car), obj_cpy(o->cons->cdr));
        break;
    case OBJ_LIST:
        return cpy_list(o);
    case OBJ_BUILTIN:
        res = obj_builtin(o->bltin->name, o->bltin->proc);
        break;
    case OBJ_LAMBDA:
        res = obj_lambda(obj_cpy(o->lambda->params), obj_cpy(o->lambda->body));
        break;
    case OBJ_ERR:
        res = obj_err(o->err);
        break;
    case OBJ_NIL:
        res = obj_nil();
        break;
    case OBJ_KEYWORD:
        res = obj_keyword(o->keyword);
        break;
    case OBJ_BOOL:
        res = obj_bool(o->bool);
    }

    return res;
}

/* printing ---------------------------------------------------------------- */

void obj_print(obj *o);

void print_cons(obj *o) {
    putchar('(');
    obj *p = o;
    while (1) {
        obj_print(obj_car(p));
        obj *cdr = obj_cdr(p);
        if (cdr->type != OBJ_CONS && cdr->type != OBJ_LIST) {
            if (cdr->type != OBJ_NIL) {
                printf(" . ");
                obj_print(cdr);
            }
            putchar(')');
            break;
        }
        putchar(' ');
        p = obj_cdr(p);
    }
}

void obj_print(obj *o) {
    switch (o->type) {
    case OBJ_NUM:
        printf("%li", o->num->val);
        break;
    case OBJ_SYM:
        printf("%s", o->sym);
        break;
    case OBJ_CONS:
        print_cons(o);
        break;
    case OBJ_LIST:
        print_cons(o->list->head);
        // print_list(o);
        break;
    case OBJ_BOOL:
        printf("%s", o->bool == TRUE ? "true" : "false");
        break;
    case OBJ_BUILTIN:
        printf("<function %s>", o->bltin->name);
        break;
    case OBJ_LAMBDA:
        printf("<lambda ");
        obj_print(o->lambda->params);
        printf(" ");
        obj_print(o->lambda->body);
        printf(">");
        break;
    case OBJ_ERR:
        printf("Error: %s", o->err);
        break;
    case OBJ_KEYWORD:
        printf("%s", o->keyword);
        break;
    case OBJ_NIL:
        printf("()");
        break;
    default:
        printf("Cannot print unknown obj type\n");
    }
}

void obj_println(obj *o) {
    obj_print(o);
    putchar('\n');
}

/* deleting ---------------------------------------------------------------- */

void obj_delete(obj *o);

void delete_list(obj *o) {
    while (o->list->count > 0) {
        obj_delete(obj_popcar(o));
    }
    obj_delete(o->list->head);
    free(o->list);
}

void obj_delete(obj *o) {
    switch (o->type) {
    case OBJ_NUM:
        free(o->num);
        break;
    case OBJ_SYM:
        free(o->sym);
        break;
    case OBJ_CONS:
        obj_delete(obj_car(o));
        obj_delete(obj_cdr(o));
        free(o->cons);
        break;
    case OBJ_LIST:
        delete_list(o);
        break;
    case OBJ_ERR:
        free(o->err);
        break;
    case OBJ_BUILTIN:
        free(o->bltin->name);
        free(o->bltin);
        break;
    case OBJ_LAMBDA:
        env_delete(o->lambda->e);
        obj_delete(o->lambda->body);
        obj_delete(o->lambda->params);
        free(o->lambda);
        break;
    case OBJ_KEYWORD:
        free(o->keyword);
        break;
    case OBJ_BOOL:
    case OBJ_NIL:
        break;
    }
    free(o);
}

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

obj *read_list(char *input) {
    obj *list = obj_list();

    while (peektok.type != TOK_RPAREN) {

        if (peektok.type == TOK_END)
            return obj_err("expected ')'");

        obj *car = read(input);

        if (peektok.type == TOK_DOT) {
            nexttok(input);
            obj *cdr = read(input);
            if (peektok.type != TOK_RPAREN) {
                obj_delete(cdr);
                return obj_err("expected ')'");
            }
            nexttok(input); /* eat ')' */
            return obj_cons(car, cdr);
        }

        obj_add(list, car);
    }

    nexttok(input); /* eat ')' */

    /* empty list */
    if (list->list->count == 0) {
        obj_delete(list);
        return obj_nil();
    }

    return list;
}

obj *expand_quote(char *input) {
    obj *quote = obj_keyword("quote");
    obj *res = obj_list();
    obj_add(res, quote);
    obj_add(res, read(input));
    return res;
}

obj *read(char *input) {
    curtok = nexttok(input);
    token tok;
    switch (curtok.type) {
    case TOK_INT:
        return read_long(curtok);
    case TOK_SYM:
        return read_sym(curtok);
    case TOK_NIL:
        token_delete(curtok);
        return obj_nil();
    case TOK_LPAREN:
        token_delete(curtok);
        return read_list(input);
    case TOK_TRUE:
    case TOK_FALSE:
        tok = curtok;
        obj *b = obj_bool(strcmp(tok.val, "true") == 0 ? TRUE : FALSE);
        token_delete(tok);
        return b;
    case TOK_TICK:
        token_delete(curtok);
        return expand_quote(input);
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
    default:
        return obj_err("unknown token '%s'", curtok.val);
    }
}

/* builtins ---------------------------------------------------------------- */

obj *builtin_plus(env *e, obj *args) {
    CASSERT(args, args->list->count > 0, "plus passed no arguments");
    TARGCHECK(args, "plus", OBJ_NUM);
    obj *x = obj_popcar(args);
    while (args->list->count > 0) {
        obj *y = obj_popcar(args);
        x->num->val += y->num->val;
        obj_delete(y);
    }
    obj_delete(args);
    return x;
}

obj *builtin_minus(env *e, obj *args) {
    CASSERT(args, args->list->count > 0, "minus passed no arguments");
    TARGCHECK(args, "minus", OBJ_NUM);
    obj *x = obj_popcar(args);
    while (args->list->count > 0) {
        obj *y = obj_popcar(args);
        x->num->val -= y->num->val;
        obj_delete(y);
    }
    obj_delete(args);
    return x;
}

obj *builtin_times(env *e, obj *args) {
    CASSERT(args, args->list->count > 0, "times passed no arguments");
    TARGCHECK(args, "times", OBJ_NUM);
    obj *x = obj_popcar(args);
    while (args->list->count > 0) {
        obj *y = obj_popcar(args);
        x->num->val *= y->num->val;
        obj_delete(y);
    }
    obj_delete(args);
    return x;
}

obj *builtin_divide(env *e, obj *args) {
    CASSERT(args, args->list->count > 0, "times passed no arguments");
    TARGCHECK(args, "divide", OBJ_NUM);
    obj *x = obj_popcar(args);
    while (args->list->count > 0) {
        obj *y = obj_popcar(args);
        if (y->num->val == 0) {
            x = obj_err("division by zero");
            obj_delete(y);
            break;
        }
        x->num->val /= y->num->val;
        obj_delete(y);
    }
    return x;
    obj_delete(args);
}

obj *builtin_remainder(env *e, obj *args) {
    CASSERT(args, args->list->count > 0, "times passed no arguments");
    TARGCHECK(args, "remainder", OBJ_NUM);
    obj *x = obj_popcar(args);
    while (args->list->count > 0) {
        obj *y = obj_popcar(args);
        if (y->num->val == 0) {
            x = obj_err("division by zero");
            obj_delete(y);
            break;
        }
        x->num->val %= y->num->val;
        obj_delete(y);
    }
    obj_delete(args);
    return x;
}

obj *builtin_cons(env *e, obj *args) {
    NARGCHECK(args, "cons", 2);

    obj *car = obj_popcar(args);
    obj *cdr = obj_popcar(args);

    obj *res;
    if (cdr->type == OBJ_LIST) {
        res = obj_list();
        obj_add(res, car);
        while (cdr->list->count > 0)
            obj_add(res, obj_popcar(cdr));
    } else {
        res = obj_cons(car, cdr);
    }

    obj_delete(args);
    return res;
}

obj *builtin_car(env *e, obj *args) {
    NARGCHECK(args, "car", 1);
    if (obj_car(args)->type != OBJ_LIST && obj_car(args)->type != OBJ_CONS) {
        obj *err = obj_err("argument to car must be a cons or a list, got %s",
                           obj_typename(obj_car(args)->type));
        obj_delete(args);
        return err;
    }

    obj *car = obj_popcar(args);
    obj *res = obj_popcar(car);
    obj_delete(args);
    obj_delete(car);
    return res;
}

obj *builtin_cdr(env *e, obj *args) {
    NARGCHECK(args, "cdr", 1);
    if (obj_car(args)->type != OBJ_LIST && obj_car(args)->type != OBJ_CONS) {
        obj *err = obj_err("argument to cdr must be a cons or a list, got %s",
                           obj_typename(obj_car(args)->type));
        obj_delete(args);
        return err;
    }

    obj *car = obj_popcar(args);
    obj *res = obj_popcdr(car);
    obj_delete(args);
    obj_delete(car);
    return res;
}

obj *builtin_list(env *e, obj *args) { return args; }

obj *builtin_eq(env *e, obj *args) {
    NARGCHECK(args, "eq", 2);
    obj *x = obj_popcar(args);
    obj *y = obj_popcar(args);
    CASSERT(args, x->type != OBJ_CONS && y->type != OBJ_LIST,
            "parameters passed to eq must be atomic");

    if (x->type != y->type) {
        obj_delete(x);
        obj_delete(y);
        obj_delete(args);
        return obj_bool(FALSE);
    }

    obj *res;
    switch (x->type) {
    case OBJ_NUM:
        res = x->num->val == y->num->val ? obj_bool(TRUE) : obj_bool(FALSE);
        break;
    case OBJ_SYM:
        res = strcmp(x->sym, y->sym) == 0 ? obj_bool(TRUE) : obj_bool(FALSE);
        break;
    case OBJ_BOOL:
        res = x->bool == y->bool ? obj_bool(TRUE) : obj_bool(FALSE);
        break;
    case OBJ_BUILTIN:
        res = strcmp(x->bltin->name, y->bltin->name) == 0 ? obj_bool(TRUE)
                                                          : obj_bool(FALSE);
        break;
    case OBJ_NIL:
        res = obj_bool(TRUE);
        break;
    case OBJ_KEYWORD:
        res = strcmp(x->keyword, y->keyword) == 0 ? obj_bool(TRUE)
                                                  : obj_bool(FALSE);
        break;
    default:
        res = obj_err("parameter passed to eq must be atomic, got %s",
                      obj_typename(x->type));
    }

    obj_delete(x);
    obj_delete(y);
    obj_delete(args);

    return res;
}

obj *builtin_atom(env *e, obj *args) {
    NARGCHECK(args, "atom", 1);
    obj *x = obj_popcar(args);

    obj *res = x->type != OBJ_CONS && x->type != OBJ_LIST ? obj_bool(TRUE)
                                                          : obj_bool(FALSE);

    obj_delete(x);
    obj_delete(args);

    return res;
}

obj *builtin_exit(env *e, obj *args) {
    NARGCHECK(args, "exit", 0);
    env_delete(e);
    exit(0);
    return NULL;
}

/* EVAL --------------------------------------------------------------------- */

obj *eval(env *, obj *);

int nestlevel;

void eval_init() { nestlevel = 0; }

obj *eval_def(env *e, obj *args) {
    CASSERT(args, nestlevel == 0, "improper context for define");
    NARGCHECK(args, "define", 2);
    obj *car = obj_car(args);
    CASSERT(args, car->type == OBJ_SYM, "first arg to define must be symbol");

    obj *k = obj_popcar(args);
    obj *v = eval(e, obj_popcar(args));
    env_insert(e, k, v);

    obj_delete(k);
    obj_delete(v);
    obj_delete(args);
    return obj_nil();
}

obj *eval_cond(env *e, obj *args) {

    TARGCHECK(args, "cond", OBJ_LIST);

    while (args->list->count > 0) {
        obj *arg = obj_popcar(args);
        CASSERT(args, arg->list->count == 2,
                "arguments to cond must themselves have two arguments");
        obj *pred = eval(e, obj_popcar(arg));

        if (pred->type != OBJ_BOOL ||
            (pred->type == OBJ_BOOL && pred->bool != FALSE)) {
            obj *res = obj_popcar(arg);
            obj_delete(pred);
            obj_delete(args);
            return eval(e, res);
        }

        obj_delete(pred);
    }

    obj_delete(args);

    return obj_nil();
}

obj *eval_quote(env *e, obj *args) {
    NARGCHECK(args, "quote", 1);
    obj *quote = obj_popcar(args);
    obj_delete(args);
    return quote;
}

obj *eval_lambda(env *e, obj *args) {
    NARGCHECK(args, "lambda", 2);
    CASSERT(args, obj_car(args)->type == OBJ_LIST,
            "first argument to lambda should be a list of parameters");
    CASSERT(args,
            obj_cadr(args)->type == OBJ_LIST || obj_isatom(obj_cadr(args)),
            "second argument to lambda must be a list or an atom");

    /* check param list for non-symbols */
    obj *params = obj_popcar(args);
    obj *cur = params;
    for (int i = 0; i < params->list->count; i++) {
        obj *sym = obj_car(cur);
        CASSERT(args, sym->type == OBJ_SYM, "invalid param list for lambda");
        cur = obj_cdr(cur);
    }

    obj *body = obj_popcar(args);
    obj_delete(args);

    return obj_lambda(params, body);
}

obj *eval_keyword(env *e, obj *o) {
    obj *res;
    obj *k = obj_popcar(o);
    ERRCHECK(o);
    if (strcmp(k->keyword, "quote") == 0)
        res = eval_quote(e, o);
    if (strcmp(k->keyword, "lambda") == 0)
        res = eval_lambda(e, o);
    else if (strcmp(k->keyword, "cond") == 0)
        res = eval_cond(e, o);
    else if (strcmp(k->keyword, "def") == 0) {
        res = eval_def(e, o);
    }
    obj_delete(k);
    return res;
}

obj *eval_call(env *e, obj *f, obj *args) {
    CASSERT(args, f->lambda->params->list->count == args->list->count,
            "number of params does not match number of args");

    f->lambda->e->parent = e;
    while (f->lambda->params->list->count > 0) {
        obj *param = obj_popcar(f->lambda->params);
        obj *arg = obj_popcar(args);
        env_insert(f->lambda->e, param, arg);
        obj_delete(param);
        obj_delete(arg);
    }
    obj_delete(args);

    return eval(f->lambda->e, obj_cpy(f->lambda->body));
}

obj *eval_list(env *e, obj *o) {
    /* check for keyword */
    if (obj_car(o->list->head)->type == OBJ_KEYWORD) {
        return eval_keyword(e, o);
    }

    nestlevel++; /* define should be top level */

    /* evaluate all children */
    obj *cur = o->list->head;
    for (int i = 0; i < o->list->count; i++) {
        cur->cons->car = eval(e, cur->cons->car);
        cur = obj_cdr(cur);
    }

    ERRCHECK(o);
    CASSERT(o,
            obj_car(o->list->head)->type == OBJ_LAMBDA ||
                obj_car(o->list->head)->type == OBJ_BUILTIN,
            "first obj in list is not a function");

    obj *f = obj_popcar(o);
    obj *res =
        f->type == OBJ_BUILTIN ? f->bltin->proc(e, o) : eval_call(e, f, o);

    obj_delete(f);

    return res;
}

obj *eval(env *e, obj *o) {
    if (o->type == OBJ_SYM)
        return env_lookup(e, o);
    if (o->type == OBJ_LIST)
        return eval_list(e, o);
    if (o->type == OBJ_CONS) {
        obj *err = obj_err("invalid syntax");
        obj_delete(o);
        return err;
    }
    if (o->type == OBJ_KEYWORD) {
        obj *err = obj_err("invalid syntax %s", o->keyword);
        obj_delete(o);
        return err;
    }
    return o;
}

void register_builtin(env *e, builtin fun, char *name) {
    obj *k = obj_sym(name);
    obj *v = obj_builtin(name, fun);
    env_insert(e, k, v);
    obj_delete(k);
    obj_delete(v);
}

env *env_init(void) {
    env *e = env_new();
    register_builtin(e, builtin_plus, "+");
    register_builtin(e, builtin_minus, "-");
    register_builtin(e, builtin_times, "*");
    register_builtin(e, builtin_divide, "/");
    register_builtin(e, builtin_remainder, "%");

    register_builtin(e, builtin_cons, "cons");
    register_builtin(e, builtin_car, "car");
    register_builtin(e, builtin_cdr, "cdr");
    register_builtin(e, builtin_list, "list");
    register_builtin(e, builtin_eq, "eq");
    register_builtin(e, builtin_atom, "atom");
    register_builtin(e, builtin_exit, "exit");
    return e;
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
        // printf("o->list->count = %d\n", o->list->count);

        lex_cleanup();
        obj_delete(o);
        free(input);
        // env_print(global);
    }

    cleanup();

    return 0;
}
