#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <gmp.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include <editline/readline.h>

#define BUFSIZE 99
#define EOS '\0'

#define STDLIB "lib/lib.fig"
#define UNITTESTS "lib/tests.fig"

// #define _DEBUG_LEX
// #define _DEBUG_READ

typedef struct obj obj;

/* TODO -----------------------------------------------------------------------
 * - lexer errors
 * - multiline repl input
 * - floating point/rational arithmetic
 */

/* errors ------------------------------------------------------------------ */

#define CASSERT(args, cond, fmt, ...)                                          \
    {                                                                          \
        if (!(cond)) {                                                         \
            obj *err = obj_err(fmt, ##__VA_ARGS__);                            \
            obj_delete(args);                                                  \
            return err;                                                        \
        }                                                                      \
    }

#define NARGCHECK(args, fun, num)                                              \
    {                                                                          \
        if (args->nargs != num) {                                              \
            obj *err = obj_err(                                                \
                "incorrect number of arguments to %s. expected %d, got %d",    \
                fun, num, args->nargs);                                        \
            obj_delete(args);                                                  \
            return err;                                                        \
        }                                                                      \
    }

#define TARGCHECK(args, fun, typ)                                              \
    {                                                                          \
        obj *cur = args;                                                       \
        for (int i = 0; i < args->nargs; i++) {                                \
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

#define ERRCHECK(args)                                                         \
    {                                                                          \
        obj *cur = args;                                                       \
        for (int i = 0; i < args->nargs; i++) {                                \
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
    TOK_STR,
    TOK_SYM,
    TOK_CONST,
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

int curchar, numtok;
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

int nextchar(FILE *f) {
    curchar = fgetc(f);
    return curchar;
}

void skipspaces(FILE *f) {
    while (isspace(curchar)) {
        nextchar(f);
    }
}

token lexnum(FILE *f) {
    int rat = 0;
    char num[BUFSIZE];

    int i = 0;
    if (curchar == '+' || curchar == '-') {
        num[i++] = curchar;
        nextchar(f);
    }

    while (!feof(f) && isdigit(curchar)) {
        num[i++] = curchar;
        nextchar(f);
    }

    if (curchar == '/') {
        rat = 1;
        num[i++] = curchar;
        nextchar(f);
        if (curchar == '+' || curchar == '-') {
            num[i++] = curchar;
            nextchar(f);
        }
        while (!feof(f) && isdigit(curchar)) {
            num[i++] = curchar;
            nextchar(f);
        }
    }
    num[i] = EOS;

    ungetc(curchar, f);

    if (rat)
        return token_new(TOK_RAT, num);

    return token_new(TOK_INT, num);
}

token lexsymbol(FILE *f) {
    if (curchar == '+' || curchar == '-') {
        int c = fgetc(f);
        if (isdigit(c)) {
            ungetc(c, f);
            return lexnum(f);
        }
        ungetc(c, f);
    }

    char sym[BUFSIZE];

    int i = 0;
    while (!feof(f) && !isspace(curchar) && curchar != ')') {
        sym[i++] = curchar;
        nextchar(f);
    }
    sym[i] = EOS;

    ungetc(curchar, f);

    if (strcmp(sym, "nil") == 0)
        return token_new(TOK_NIL, sym);
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

token lexstring(FILE *f) {
    nextchar(f);

    char sym[BUFSIZE];

    int i = 0;
    while (curchar != '"') {

        if (feof(f)) {
            fprintf(stderr, "expected '\"'");
            break;
        }

        if (curchar == '\\') {

            nextchar(f);

            switch (curchar) {
            case 'n':
                sym[i++] = '\n';
                break;
            case 't':
                sym[i++] = '\t';
                break;
            case 'f':
                sym[i++] = '\f';
                break;
            default:
                sym[i++] = curchar;
            }

        } else {
            sym[i++] = curchar;
        }

        nextchar(f);
    }
    sym[i] = EOS;

    return token_new(TOK_STR, sym);
}

token lexconst(FILE *f) {
    char sym[BUFSIZE];
    int i = 0;
    while (!feof(f) && !isspace(curchar) && curchar != ')') {
        sym[i++] = curchar;
        nextchar(f);
    }
    sym[i] = EOS;

    ungetc(curchar, f);

    return token_new(TOK_CONST, sym);
}

token lex(FILE *f) {

    nextchar(f);
    skipspaces(f);

    if (feof(f))
        return token_new(TOK_END, "end");
    else if (isdigit(curchar))
        return lexnum(f);
    else if (curchar == '(') {
        return token_new(TOK_LPAREN, "(");
    } else if (curchar == ')') {
        return token_new(TOK_RPAREN, ")");
    } else if (curchar == '.') {
        return token_new(TOK_DOT, ".");
    } else if (curchar == '\'') {
        return token_new(TOK_TICK, "'");
    } else if (curchar == '"') {
        return lexstring(f);
    } else if (curchar == '#') {
        return lexconst(f);
    }

    return lexsymbol(f);
}

void lex_init(FILE *f) {
    numtok = 0;
    peektok = lex(f);
}

void lex_cleanup(void) { token_delete(peektok); }

/* objects ----------------------------------------------------------------- */

typedef struct {
    token_t type;
    union {
        mpz_t val;
        mpq_t rat;
    };
} num_t;

typedef struct {
    obj *car;
    obj *cdr;
} cons_t;

typedef struct env env;
typedef obj *(*builtin)(env *, obj *);

typedef struct {
    char *name;
    builtin proc;
    env *e;
    obj *params;
    obj *body;
} fun_t;

typedef enum { CONST_CHAR, CONST_BOOL, CONST_ERR } const_type;
typedef enum { BOOL_T, BOOL_F } bool_t;

typedef struct {
    const_type type;
    char *repr;
    union {
        char c;
        bool_t bool;
        char *err;
    };
} const_t;

typedef enum {
    OBJ_NUM,
    OBJ_SYM,
    OBJ_STR,
    OBJ_CONS,
    OBJ_CONST,
    OBJ_FUN,
    OBJ_KEYWORD,
    OBJ_NIL,
    OBJ_ERR
} obj_t;

struct obj {
    obj_t type;
    int nargs;
    union {
        num_t *num;
        char *sym;
        char *str;
        cons_t *cons;
        const_t *constant;
        fun_t *fun;
        char *keyword;
        char *err;
    };
};

struct env {
    struct env *parent;
    int count;
    char **syms;
    obj **vals;
};

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

    /* copy name into lambda */
    if (v->type == OBJ_FUN && !v->fun->proc && !v->fun->name) {
        e->vals[e->count - 1]->fun->name =
            malloc(sizeof(char) * (strlen(k->sym) + 1));
        strcpy(e->vals[e->count - 1]->fun->name, k->sym);
    }
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

void init_rat(num_t *n, char *ratstr) {
    char *nstr, *dstr;

    nstr = strtok(ratstr, "/");
    dstr = strtok(NULL, "/");

    mpq_init(n->rat);
    mpz_init_set_str(mpq_numref(n->rat), nstr, 10);
    mpz_init_set_str(mpq_denref(n->rat), dstr, 10);

    mpq_canonicalize(n->rat);
}

num_t *mk_num(char *numstr, token_t ttype) {
    num_t *n = malloc(sizeof(num_t));
    n->type = ttype;
    switch (n->type) {
    case TOK_INT:
        mpz_init_set_str(n->val, numstr, 10);
        break;
    case TOK_RAT:
        init_rat(n, numstr);
        break;
    default:
        break;
    }
    return n;
}

cons_t *mk_cons(obj *car, obj *cdr) {
    cons_t *c = malloc(sizeof(cons_t));
    c->car = car;
    c->cdr = cdr;
    return c;
}

obj *obj_nil(void);

fun_t *mk_fun(char *name, builtin bltin, obj *params, obj *body) {
    fun_t *fun = malloc(sizeof(fun_t));
    fun->proc = bltin;
    fun->params = params;
    fun->body = body;

    if (!fun->proc)
        fun->e = env_new();

    if (name) {
        fun->name = malloc(sizeof(char) * (strlen(name) + 1));
        strcpy(fun->name, name);
    } else {
        fun->name = NULL;
    }
    return fun;
}

const_t *mk_const(char *c) {
    const_t *constant = malloc(sizeof(const_t));

    constant->repr = malloc(sizeof(char) * (strlen(c) + 1));
    strcpy(constant->repr, c);

    if (strcmp("#t", constant->repr) == 0) {
        constant->type = CONST_BOOL;
        constant->bool = BOOL_T;
    } else if (strcmp("#f", constant->repr) == 0) {
        constant->type = CONST_BOOL;
        constant->bool = BOOL_F;
    } else if (constant->repr[1] == '\\' && strlen(constant->repr) == 3) {
        constant->type = CONST_CHAR;
        constant->c = constant->repr[2];
    } else {
        constant->type = CONST_ERR;
        /* add length of "invalid constant", 16 */
        constant->err = malloc(sizeof(char) * (strlen(c) + 1 + 17));
        snprintf(constant->err, 17, "invalid constant %s", c);
    }

    return constant;
}

/* obj types --------------------------------------------------------------- */

obj *obj_num(char *numstr, token_t ttype) {
    obj *o = malloc(sizeof(obj));
    o->type = OBJ_NUM;
    o->num = mk_num(numstr, ttype);
    o->nargs = 0;
    return o;
}

obj *obj_sym(char *name) {
    obj *o = malloc(sizeof(obj));
    o->type = OBJ_SYM;
    o->sym = malloc(sizeof(char) * (strlen(name) + 1));
    strcpy(o->sym, name);
    o->nargs = 0;
    return o;
}

obj *obj_str(char *str) {
    obj *o = malloc(sizeof(obj));
    o->type = OBJ_STR;
    o->str = malloc(sizeof(char) * (strlen(str) + 1));
    strcpy(o->str, str);
    o->nargs = 0;
    return o;
}

obj *obj_cons(obj *car, obj *cdr) {
    obj *o = malloc(sizeof(obj));
    o->type = OBJ_CONS;
    o->cons = mk_cons(car, cdr);
    o->nargs = 0;
    return o;
}

obj *obj_builtin(char *name, builtin proc) {
    obj *o = malloc(sizeof(obj));
    o->type = OBJ_FUN;
    o->fun = mk_fun(name, proc, NULL, NULL);
    o->nargs = 0;
    return o;
}

obj *obj_lambda(obj *params, obj *body) {
    obj *o = malloc(sizeof(obj));
    o->type = OBJ_FUN;
    o->fun = mk_fun(NULL, NULL, params, body);
    o->nargs = 0;
    return o;
}

obj *obj_const(char *constant) {
    obj *o = malloc(sizeof(obj));
    o->type = OBJ_CONST;

    o->constant = mk_const(constant);
    if (o->constant->type == CONST_ERR) {
        obj *err = obj_err(o->constant->err);
        free(o->constant->err);
        obj_delete(o);
        return err;
    }

    o->nargs = 0;
    return o;
}

obj *obj_char(char c) {
    char *repr = malloc(sizeof(char) * 4);
    sprintf(repr, "#\\%c", c);
    obj *constant = obj_const(repr);
    free(repr);
    return constant;
}

obj *obj_bool(bool_t type) {
    return type == BOOL_T ? obj_const("#t") : obj_const("#f");
}

int obj_isfalse(obj *c) {
    return c->type == OBJ_CONST && c->constant->type == CONST_BOOL &&
           c->constant->bool == BOOL_F;
}

int obj_istrue(obj *c) { return !obj_isfalse(c); }

obj *obj_keyword(char *name) {
    obj *o = malloc(sizeof(obj));
    o->type = OBJ_KEYWORD;
    o->keyword = malloc(sizeof(char) * (strlen(name) + 1));
    strcpy(o->keyword, name);
    o->nargs = 0;
    return o;
}

obj *obj_nil(void) {
    obj *o = malloc(sizeof(obj));
    o->type = OBJ_NIL;
    o->nargs = 0;
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
    o->nargs = 0;

    return o;
}

int obj_isatom(obj *o) { return o->type != OBJ_CONS && o->type != OBJ_FUN; }

char *obj_typename(obj_t type) {
    switch (type) {
    case OBJ_NUM:
        return "number";
    case OBJ_SYM:
        return "symbol";
    case OBJ_STR:
        return "string";
    case OBJ_CONS:
        return "cons";
    case OBJ_CONST:
        return "constant";
    case OBJ_FUN:
        return "function";
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

obj *obj_car(obj *o) { return o->cons->car; }

obj *obj_cdr(obj *o) { return o->cons->cdr; }

obj *obj_cadr(obj *o) { return obj_car(obj_cdr(o)); }

obj *obj_popcar(obj **o) {
    obj *car = obj_car(*o);
    obj *cdr = obj_cdr(*o);
    int count = (*o)->nargs;
    *o = cdr;
    (*o)->nargs = count - 1;
    return car;
}

/* copying ----------------------------------------------------------------- */

obj *cpy_const(obj *o) {
    char *repr = malloc(sizeof(char) * (strlen(o->constant->repr) + 1));
    strcpy(repr, o->constant->repr);
    obj *c = obj_const(repr);
    free(repr);
    return c;
}

obj *obj_cpy(obj *o) {
    obj *res;
    char *num;
    switch (o->type) {
    case OBJ_NUM:
        num = mpz_get_str(NULL, 10, o->num->val);
        res = obj_num(num, TOK_INT);
        free(num);
        return res;
    case OBJ_SYM:
        res = obj_sym(o->sym);
        break;
    case OBJ_STR:
        res = obj_str(o->str);
        break;
    case OBJ_CONS:
        res = obj_cons(obj_cpy(o->cons->car), obj_cpy(o->cons->cdr));
        break;
    case OBJ_FUN:
        if (o->fun->proc)
            res = obj_builtin(o->fun->name, o->fun->proc);
        else {
            res = obj_lambda(obj_cpy(o->fun->params), obj_cpy(o->fun->body));
            if (o->fun->name) {
                res->fun->name =
                    malloc(sizeof(char) * (strlen(o->fun->name) + 1));
                strcpy(res->fun->name, o->fun->name);
            }
        }
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
    case OBJ_CONST:
        res = cpy_const(o);
    }
    res->nargs = o->nargs;
    return res;
}

/* printing ---------------------------------------------------------------- */

void obj_print(obj *o);

void print_num(obj *o) {
    switch (o->num->type) {
    case TOK_INT:
        mpz_out_str(stdout, 10, o->num->val);
        break;
    case TOK_RAT:
        mpq_out_str(stdout, 10, o->num->rat);
        break;
    case TOK_FLOAT:
        break;
    default:
        break;
    }
}

void print_rawstr(char *str) {
    printf("\"");
    int len = strlen(str);
    for (int i = 0; i < len; i++) {
        switch (str[i]) {
        case '\n':
            printf("\\n");
            break;
        case '\t':
            printf("\\t");
            break;
        case '\f':
            printf("\\f");
            break;
        case '\"':
            printf("\\\"");
            break;
        default:
            printf("%c", str[i]);
        }
    }
    printf("\"");
}

void print_cons(obj *o) {
    putchar('(');
    obj *p = o;
    while (1) {
        obj_print(obj_car(p));
        obj *cdr = obj_cdr(p);
        if (cdr->type != OBJ_CONS) {
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
        print_num(o);
        break;
    case OBJ_SYM:
        printf("%s", o->sym);
        break;
    case OBJ_STR:
        print_rawstr(o->str);
        break;
    case OBJ_CONS:
        print_cons(o);
        break;
    case OBJ_CONST:
        printf("%s", o->constant->repr);
        break;
    case OBJ_FUN:
        if (o->fun->name)
            printf("<function '%s'>", o->fun->name);
        else {
            printf("<function>");
        }
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

void clear_num(obj *o) {
    switch (o->num->type) {
    case TOK_INT:
        mpz_clear(o->num->val);
        break;
    case TOK_RAT:
        mpq_clear(o->num->rat);
        break;
    case TOK_FLOAT:
    default:
        break;
    }
    free(o->num);
}

void obj_delete(obj *o) {
    switch (o->type) {
    case OBJ_NUM:
        clear_num(o);
        break;
    case OBJ_SYM:
        free(o->sym);
        break;
    case OBJ_STR:
        free(o->str);
        break;
    case OBJ_CONS:
        obj_delete(obj_car(o));
        obj_delete(obj_cdr(o));
        free(o->cons);
        break;
    case OBJ_ERR:
        free(o->err);
        break;
    case OBJ_FUN:
        free(o->fun->name);
        if (o->fun->params)
            obj_delete(o->fun->params);
        if (o->fun->body)
            obj_delete(o->fun->body);
        free(o->fun);
        break;
    case OBJ_KEYWORD:
        free(o->keyword);
        break;
    case OBJ_CONST:
        free(o->constant->repr);
        free(o->constant);
        break;
    case OBJ_NIL:
        break;
    }
    free(o);
}

/* parsing ------------------------------------------------------------------ */

token nexttok(FILE *f) {
    curtok = peektok;
    peektok = lex(f);
    return curtok;
}

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
    default:
        return obj_err("unknown token '%s'", curtok.val);
    }
}

/* builtins ---------------------------------------------------------------- */

void int_int_add(obj *x, obj *y) {
    mpz_add(x->num->val, x->num->val, y->num->val);
}

void rat_rat_add(obj *x, obj *y) {
    mpq_add(x->num->rat, x->num->rat, y->num->rat);
}

obj *builtin_plus(env *e, obj *args) {
    CASSERT(args, args->nargs > 0, "plus passed no arguments");
    TARGCHECK(args, "plus", OBJ_NUM);
    obj *x = obj_popcar(&args);
    while (args->nargs > 0) {
        obj *y = obj_popcar(&args);
        mpz_add(x->num->val, x->num->val, y->num->val);
        obj_delete(y);
    }
    obj_delete(args);
    return x;
}

obj *builtin_minus(env *e, obj *args) {
    CASSERT(args, args->nargs > 0, "minus passed no arguments");
    TARGCHECK(args, "minus", OBJ_NUM);
    obj *x = obj_popcar(&args);
    while (args->nargs > 0) {
        obj *y = obj_popcar(&args);
        mpz_sub(x->num->val, x->num->val, y->num->val);
        obj_delete(y);
    }
    obj_delete(args);
    return x;
}

obj *builtin_times(env *e, obj *args) {
    CASSERT(args, args->nargs > 0, "times passed no arguments");
    TARGCHECK(args, "times", OBJ_NUM);
    obj *x = obj_popcar(&args);
    while (args->nargs > 0) {
        obj *y = obj_popcar(&args);
        mpz_mul(x->num->val, x->num->val, y->num->val);
        obj_delete(y);
    }
    obj_delete(args);
    return x;
}

obj *builtin_divide(env *e, obj *args) {
    CASSERT(args, args->nargs > 0, "times passed no arguments");
    TARGCHECK(args, "divide", OBJ_NUM);
    obj *x = obj_popcar(&args);
    while (args->nargs > 0) {
        obj *y = obj_popcar(&args);
        if (mpz_sgn(y->num->val) == 0) {
            obj_delete(x);
            x = obj_err("division by zero");
            obj_delete(y);
            break;
        }
        mpz_div(x->num->val, x->num->val, y->num->val);
        obj_delete(y);
    }
    return x;
    obj_delete(args);
}

obj *builtin_remainder(env *e, obj *args) {
    CASSERT(args, args->nargs > 0, "times passed no arguments");
    TARGCHECK(args, "remainder", OBJ_NUM);
    obj *x = obj_popcar(&args);
    while (args->nargs > 0) {
        obj *y = obj_popcar(&args);
        if (mpz_sgn(y->num->val) == 0) {
            obj_delete(x);
            x = obj_err("division by zero");
            obj_delete(y);
            break;
        }
        mpz_mod(x->num->val, x->num->val, y->num->val);
        obj_delete(y);
    }
    obj_delete(args);
    return x;
}

int cmp_nums(obj *x, obj *y) {
    switch (x->num->type) {
    case TOK_INT:
        return mpz_cmp(x->num->val, y->num->val);
    default:
        return 0;
    }
}

obj *builtin_gt(env *e, obj *args) {
    CASSERT(args, args->nargs > 0, "gt passed no arguments");
    NARGCHECK(args, "gt", 2);
    TARGCHECK(args, "gt", OBJ_NUM);

    obj *x = obj_popcar(&args);
    obj *y = obj_popcar(&args);

    int res = cmp_nums(x, y);

    obj_delete(x);
    obj_delete(y);
    obj_delete(args);

    return res > 0 ? obj_bool(BOOL_T) : obj_bool(BOOL_F);
}

obj *builtin_gte(env *e, obj *args) {
    CASSERT(args, args->nargs > 0, "gte passed no arguments");
    NARGCHECK(args, "gte", 2);
    TARGCHECK(args, "gte", OBJ_NUM);

    obj *x = obj_popcar(&args);
    obj *y = obj_popcar(&args);

    int res = cmp_nums(x, y);

    obj_delete(x);
    obj_delete(y);
    obj_delete(args);

    return res >= 0 ? obj_bool(BOOL_T) : obj_bool(BOOL_F);
}

obj *builtin_lt(env *e, obj *args) {
    CASSERT(args, args->nargs > 0, "lt passed no arguments");
    NARGCHECK(args, "lt", 2);
    TARGCHECK(args, "lt", OBJ_NUM);

    obj *x = obj_popcar(&args);
    obj *y = obj_popcar(&args);

    int res = cmp_nums(x, y);

    obj_delete(x);
    obj_delete(y);
    obj_delete(args);

    return res < 0 ? obj_bool(BOOL_T) : obj_bool(BOOL_F);
}

obj *builtin_lte(env *e, obj *args) {
    CASSERT(args, args->nargs > 0, "lte passed no arguments");
    NARGCHECK(args, "lte", 2);
    TARGCHECK(args, "lte", OBJ_NUM);

    obj *x = obj_popcar(&args);
    obj *y = obj_popcar(&args);

    int res = cmp_nums(x, y);

    obj_delete(x);
    obj_delete(y);
    obj_delete(args);

    return res <= 0 ? obj_bool(BOOL_T) : obj_bool(BOOL_F);
}

obj *builtin_eq(env *e, obj *args) {
    NARGCHECK(args, "eq", 2);

    obj *x = obj_popcar(&args);
    obj *y = obj_popcar(&args);

    if (!obj_isatom(x) || !obj_isatom(y)) {
        obj *err = obj_err("arguments passed to eq must be atomic");
        obj_delete(x);
        obj_delete(y);
        obj_delete(args);
        return err;
    }

    if (x->type != y->type) {
        obj_delete(x);
        obj_delete(y);
        obj_delete(args);
        return obj_bool(BOOL_F);
    }

    obj *res;
    switch (x->type) {
    case OBJ_NUM:
        res = cmp_nums(x, y) == 0 ? obj_bool(BOOL_T) : obj_bool(BOOL_F);
        break;
    case OBJ_SYM:
        res = strcmp(x->sym, y->sym) == 0 ? obj_bool(BOOL_T) : obj_bool(BOOL_F);
        break;
    case OBJ_STR:
        res = strcmp(x->str, y->str) == 0 ? obj_bool(BOOL_T) : obj_bool(BOOL_F);
        break;
    case OBJ_CONST:
        res = strcmp(x->constant->repr, y->constant->repr) == 0
                  ? obj_bool(BOOL_T)
                  : obj_bool(BOOL_F);
        break;
    case OBJ_NIL:
        res = obj_bool(BOOL_T);
        break;
    case OBJ_KEYWORD:
        res = strcmp(x->keyword, y->keyword) == 0 ? obj_bool(BOOL_T)
                                                  : obj_bool(BOOL_F);
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

obj *builtin_cons(env *e, obj *args) {
    NARGCHECK(args, "cons", 2);

    obj *car = obj_popcar(&args);
    obj *cdr = obj_popcar(&args);

    obj *res = obj_cons(car, cdr);
    obj_delete(args);
    return res;
}

obj *builtin_car(env *e, obj *args) {
    NARGCHECK(args, "car", 1);
    if (obj_car(args)->type != OBJ_CONS) {
        obj *err = obj_err("argument to car must be cons, got %s",
                           obj_typename(obj_car(args)->type));
        obj_delete(args);
        return err;
    }

    obj *car = obj_popcar(&args);
    obj *res = obj_popcar(&car);
    obj_delete(args);
    obj_delete(car);
    return res;
}

obj *builtin_cdr(env *e, obj *args) {
    NARGCHECK(args, "cdr", 1);
    if (obj_car(args)->type != OBJ_CONS) {
        obj *err = obj_err("argument to cdr must be a cons, got %s",
                           obj_typename(obj_car(args)->type));
        obj_delete(args);
        return err;
    }

    obj *list = obj_popcar(&args);
    obj *car = obj_popcar(&list);

    obj_delete(args);
    obj_delete(car);
    return list;
}

obj *builtin_list(env *e, obj *args) { return args; }

obj *builtin_atom(env *e, obj *args) {
    NARGCHECK(args, "atom", 1);
    obj *x = obj_popcar(&args);

    obj *res = obj_isatom(x) ? obj_bool(BOOL_T) : obj_bool(BOOL_F);

    obj_delete(x);
    obj_delete(args);

    return res;
}

obj *builtin_strtolist(env *e, obj *args) {
    NARGCHECK(args, "string->list", 1);
    TARGCHECK(args, "string->list", OBJ_STR);

    obj *arg = obj_popcar(&args);

    int len = strlen(arg->str);
    obj *list = obj_nil();
    for (int i = 0; i < len; i++) {
        if (list->nargs) {
            obj *cur = list;
            for (int i = 0; i < list->nargs - 1; i++) {
                cur = obj_cdr(cur);
            }
            cur->cons->cdr = obj_cons(obj_char(arg->str[i]), cur->cons->cdr);
        } else {
            list = obj_cons(obj_char(arg->str[i]), list);
        }
        list->nargs++;
    }

    obj_delete(arg);
    obj_delete(args);

    return list;
}

obj *builtin_listtostr(env *e, obj *args) {
    NARGCHECK(args, "list->string", 1);
    TARGCHECK(args, "list->string", OBJ_CONS);

    obj *arg = obj_popcar(&args);

    char *str = malloc(sizeof(char) * (arg->nargs + 1));

    /* every obj in list must be char type */
    obj *cur = arg;
    for (int i = 0; i < arg->nargs; i++) {
        obj *c = obj_car(cur);
        if (c->type != OBJ_CONST || c->constant->type != CONST_CHAR) {
            obj *err = obj_err("list->string cannot take non character type %s",
                               obj_typename(c->type));
            obj_delete(arg);
            obj_delete(args);
            return err;
        }
        str[i] = c->constant->c;
        cur = obj_cdr(cur);
    }

    obj_delete(arg);
    obj_delete(args);

    return obj_str(str);
}

obj *builtin_type(env *e, obj *args) {
    NARGCHECK(args, "type", 1);
    obj *item = obj_popcar(&args);
    obj *res = obj_str(obj_typename(item->type));
    obj_delete(item);
    obj_delete(args);
    return res;
}

obj *builtin_print(env *e, obj *args) {
    while (args->nargs > 0) {
        obj *item = obj_popcar(&args);
        switch (item->type) {
        case OBJ_STR:
            printf("%s", item->str);
            break;
        default:
            obj_print(item);
        }
        obj_delete(item);
    }

    return args;
}

obj *builtin_println(env *e, obj *args) {
    obj *res = builtin_print(e, args);
    printf("\n");
    return res;
}

obj *builtin_err(env *e, obj *args) {
    NARGCHECK(args, "err", 1);
    TARGCHECK(args, "err", OBJ_STR);

    obj *msg = obj_popcar(&args);
    obj_delete(args);
    return obj_err(msg->str);
}

obj *builtin_exit(env *, obj *);

/* EVAL --------------------------------------------------------------------- */

obj *eval(env *, obj *);

obj *builtin_eval(env *e, obj *args) {
    NARGCHECK(args, "eval", 1);
    obj *expr = eval(e, obj_popcar(&args));
    obj_delete(args);
    return expr;
}

obj *eval_def(env *e, obj *args) {
    NARGCHECK(args, "define", 2);

    obj *car = obj_car(args);
    CASSERT(args, car->type == OBJ_SYM,
            "first arg to def must be symbol, got %s", obj_typename(car->type));

    obj *k = obj_popcar(&args);
    obj *v = eval(e, obj_popcar(&args));
    env_insert(e, k, v);

    obj_delete(k);
    obj_delete(v);
    obj_delete(args);
    return obj_nil();
}

obj *eval_cond(env *e, obj *args) {

    TARGCHECK(args, "cond", OBJ_CONS);

    while (args->nargs > 0) {
        obj *arg = obj_popcar(&args);

        CASSERT(args, arg->nargs == 2,
                "arguments to cond must themselves have two arguments");

        obj *pred = eval(e, obj_popcar(&arg));

        if (pred->type == OBJ_ERR) {
            obj_delete(arg);
            obj_delete(args);
            return pred;
        }

        if (!obj_isfalse(pred)) {
            obj *res = obj_popcar(&arg);
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
    obj *quote = obj_popcar(&args);
    obj_delete(args);
    return quote;
}

obj *eval_lambda(env *e, obj *args) {
    // NARGCHECK(args, "lambda", 2);
    CASSERT(args,
            obj_car(args)->type == OBJ_CONS || obj_car(args)->type == OBJ_NIL,
            "first argument should be a list, got %s",
            obj_typename(obj_car(args)->type));

    /* check param list for non-symbols */
    obj *params = obj_popcar(&args);
    obj *cur = params;
    for (int i = 0; i < params->nargs; i++) {
        obj *sym = obj_car(cur);
        CASSERT(args, sym->type == OBJ_SYM,
                "lambda param list must contain only symbols, got %s",
                obj_typename(sym->type));
        cur = obj_cdr(cur);
    }

    obj *res = obj_lambda(params, args);

    return res;
}

obj *eval_keyword(env *e, obj *o) {
    obj *res;
    obj *k = obj_popcar(&o);
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

    /* check builtin */
    if (f->fun->proc)
        return f->fun->proc(e, args);

    NARGCHECK(args, f->fun->name ? f->fun->name : "lambda",
              f->fun->params->nargs);

    /* bind args to params */
    f->fun->e->parent = e;
    while (f->fun->params->nargs > 0) {
        obj *param = obj_popcar(&f->fun->params);
        obj *arg = obj_popcar(&args);
        env_insert(f->fun->e, param, arg);
        obj_delete(param);
        obj_delete(arg);
    }
    obj_delete(args);

    /* evaluate each expression in lambda body but one */
    while (f->fun->body->nargs > 1) {
        obj *expr = obj_popcar(&f->fun->body);
        obj *res = eval(f->fun->e, expr);
        obj_delete(res);
    }

    /* last expression evaluated is return value */
    obj *expr = obj_popcar(&f->fun->body);
    obj *res = eval(f->fun->e, expr);

    return res;
}

obj *eval_list(env *e, obj *o) {
    /* check for keyword */
    if (obj_car(o)->type == OBJ_KEYWORD) {
        return eval_keyword(e, o);
    }

    /* evaluate all children and check for errors*/
    obj *cur = o;
    for (int i = 0; i < o->nargs; i++) {
        cur->cons->car = eval(e, cur->cons->car);
        cur = obj_cdr(cur);
    }
    ERRCHECK(o);

    /* make sure first object is callable */
    obj *f = obj_car(o);
    CASSERT(o, f->type == OBJ_FUN, "object of type %s is not callable",
            obj_typename(f->type));

    /* pop first object and evaluate */
    f = obj_popcar(&o);
    obj *res = eval_call(e, f, o);

    obj_delete(f);

    return res;
}

obj *eval(env *e, obj *o) {
    if (o->type == OBJ_SYM)
        return env_lookup(e, o);
    if (o->type == OBJ_CONS)
        return eval_list(e, o);
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

env *global_env(void) {
    env *e = env_new();
    register_builtin(e, builtin_plus, "+");
    register_builtin(e, builtin_minus, "-");
    register_builtin(e, builtin_times, "*");
    register_builtin(e, builtin_divide, "/");
    register_builtin(e, builtin_remainder, "%");

    register_builtin(e, builtin_gt, ">");
    register_builtin(e, builtin_gte, ">=");
    register_builtin(e, builtin_lt, "<");
    register_builtin(e, builtin_lte, "<=");

    register_builtin(e, builtin_cons, "cons");
    register_builtin(e, builtin_car, "car");
    register_builtin(e, builtin_cdr, "cdr");
    register_builtin(e, builtin_list, "list");
    register_builtin(e, builtin_eq, "eq");
    register_builtin(e, builtin_atom, "atom");
    register_builtin(e, builtin_eval, "eval");

    register_builtin(e, builtin_strtolist, "string->list");
    register_builtin(e, builtin_listtostr, "list->string");

    register_builtin(e, builtin_type, "type");
    register_builtin(e, builtin_print, "print");
    register_builtin(e, builtin_println, "println");
    register_builtin(e, builtin_err, "err");
    register_builtin(e, builtin_exit, "exit");
    return e;
}

/* REPL --------------------------------------------------------------------- */

env *universe;

void finit(char *fname, FILE **f) {
    *f = fopen(fname, "r");
    if (!(*f)) {
        fprintf(stderr, "unable to locate file %s\n", fname);
        exit(1);
    }
    lex_init(*f);
}

void cleanup(char *input, FILE *stream) {
    if (input)
        free(input);
    lex_cleanup();
    fclose(stream);
}

void readfile(char *fname) {

    FILE *infile;

    finit(fname, &infile);

    while (!feof(infile)) {
        obj *o = eval(universe, read(infile));
        if (o->type == OBJ_ERR) {
            obj_println(o);
            obj_delete(o);
            break;
        }
        obj_delete(o);
    }

    cleanup(NULL, infile);
}

FILE *stream;
char *input;

obj *builtin_exit(env *e, obj *args) {
    NARGCHECK(args, "exit", 0);
    env_delete(e);
    obj_delete(args);
    cleanup(input, stream);
    exit(0);
    return NULL;
}

void repl_println(obj *o) {
    printf("=> ");
    obj_println(o);
}

void repl() {

    printf("fig version 0.1\n\n");

    while (1) {
        input = readline("> ");
        if (strlen(input) == 0)
            continue;
        add_history(input);
        stream = fmemopen(input, strlen(input), "r");
        lex_init(stream);
#ifdef _DEBUG_LEX

        while (peektok.type != TOK_END) {
            nexttok(stream);
            token_println(curtok);
            token_delete(curtok);
        }
        printf("numtok = %d\n", numtok);

#elif defined _DEBUG_READ

        obj *o = read(stream);
        obj_println(o);
        printf("o->nargs = %d\n", o->nargs);
        obj_delete(o);

#else
        obj *o = eval(universe, read(stream));
        repl_println(o);
        obj_delete(o);
#endif
        cleanup(input, stream);
    }
}

int main(int argc, char **argv) {

    universe = global_env();

    readfile(STDLIB);

    if (argc > 1)
        readfile(argv[1]);
    else {
        repl();
    }

    env_delete(universe);

    return 0;
}
