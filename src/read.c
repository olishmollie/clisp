#include "read.h"

Reader *reader_new(FILE *in) {
    Reader *rdr = malloc(sizeof(Reader));
    rdr->cur = 0;
    rdr->line = 0;
    rdr->in = in;
    return rdr;
}

int reader_eof(Reader *rdr) {
    return rdr->cur == EOF;
}

static int get_next_char(Reader *rdr) {
    rdr->cur = getc(rdr->in);
    return rdr->cur;
}

static int get_peek_char(Reader *rdr) {
    int c = getc(rdr->in);
    ungetc(c, rdr->in);
    return c;
}

void reader_delete(Reader *rdr) {
    /* flush input buffer */
    while (get_next_char(rdr) != '\n' && !reader_eof(rdr)) {
    }
    if (rdr->in != stdin) {
        fclose(rdr->in);
    }
    free(rdr);
}

int is_delim(int c) {
    return isspace(c) || c == EOF || c == '(' || c == ')' || c == '"' ||
           c == ';' || c == '\0';
}

int is_initial(int c) {
    char *allowed = "+-*/%!?<>=&^|";
    return isalpha(c) || strchr(allowed, c);
}

void skip_whitespace_and_comments(Reader *rdr) {
    while (get_next_char(rdr) != EOF) {
        if (isspace(rdr->cur)) {
            continue;
        } else if (rdr->cur == ';') {
            while (get_next_char(rdr) != EOF && rdr->cur != '\n') {
            }
            continue;
        }
        ungetc(rdr->cur, rdr->in);
        break;
    }
}

int expected_string(Reader *rdr, char *str) {
    while (*str != '\0') {
        get_next_char(rdr);
        if (*str++ != rdr->cur)
            return 0;
    }
    if (!is_delim(get_peek_char(rdr)))
        return 0;
    return 1;
}

obj_t *read_character(VM *vm, Reader *rdr) {
    get_next_char(rdr);

    obj_t *res;
    switch (rdr->cur) {
    case 'n':
        if (expected_string(rdr, "ewline"))
            return mk_char(vm, '\n');
        res = mk_char(vm, 'n');
        ungetc(rdr->cur, rdr->in);
        break;
    case 't':
        if (expected_string(rdr, "ab"))
            return mk_char(vm, '\t');
        res = mk_char(vm, 't');
        ungetc(rdr->cur, rdr->in);
        break;
    case 's':
        if (expected_string(rdr, "pace"))
            return mk_char(vm, ' ');
        res = mk_char(vm, 's');
        ungetc(rdr->cur, rdr->in);
        break;
    default:
        res = mk_char(vm, rdr->cur);
    }

    if (!is_delim(get_peek_char(rdr)))
        return mk_err(vm, "invalid constant");

    return res;
}

obj_t *read_constant(VM *vm, Reader *rdr) {
    get_next_char(rdr);
    if (rdr->cur == '\\') {
        return read_character(vm, rdr);
    } else if (rdr->cur == 't') {
        return true;
    } else if (rdr->cur == 'f') {
        return false;
    }

    return mk_err(vm, "invalid constant");
}

obj_t *read_symbol(VM *vm, Reader *rdr) {
    char sym[MAX_STRING_LENGTH];
    int i = 0;

    if (rdr->cur == '|') {
        get_next_char(rdr);
        while (!reader_eof(rdr) && rdr->cur != '|') {
            sym[i++] = rdr->cur;
            get_next_char(rdr);
        }
        get_next_char(rdr); /* eat the '|' */

        /* special case '||' */
        if (i == 0) {
            return mk_sym(vm, "||");
        }
    }
    else {
        /* regular symbol */
        while (!is_delim(rdr->cur)) {
            sym[i++] = rdr->cur;
            get_next_char(rdr);
        }
    }

    sym[i] = '\0';

    ungetc(rdr->cur, rdr->in);
    return mk_sym(vm, sym);
}

obj_t *read_string(VM *vm, Reader *rdr) {
    char str[MAX_STRING_LENGTH];
    int i = 0;

    get_next_char(rdr);
    while (rdr->cur != '"') {
        str[i++] = rdr->cur;
        get_next_char(rdr);
        if (reader_eof(rdr)) {
            return mk_err(vm, "unclosed string literal");
        }
    }
    str[i] = '\0';

    return mk_string(vm, str);
}

obj_t *read_number(VM *vm, Reader *rdr) {
    char num[MAX_STRING_LENGTH];
    int i = 0, is_decimal = 0, is_fractional = 0;

    if (rdr->cur == '-') {
        num[i++] = rdr->cur;
        get_next_char(rdr);
    }

    while (isdigit(rdr->cur) && i < MAX_STRING_LENGTH) {
        num[i++] = rdr->cur;
        get_next_char(rdr);
    }

    if (i == MAX_STRING_LENGTH)
        return mk_err(vm, "string exceeds max length");

    if (rdr->cur == '/' || rdr->cur == '.') {
        is_decimal = rdr->cur == '.';
        is_fractional = rdr->cur == '/';

        num[i++] = rdr->cur;
        get_next_char(rdr);
        while (isdigit(rdr->cur)) {
            num[i++] = rdr->cur;
            get_next_char(rdr);
        }
    }

    num[i] = '\0';

    if (is_delim(rdr->cur)) {
        ungetc(rdr->cur, rdr->in);
        return mk_num_from_str(vm, num, is_decimal, is_fractional);
    }

    return mk_err(vm, "invalid number syntax");
}

obj_t *read_quote(VM *vm, Reader *rdr, obj_t *quote_sym) {
    int sp = vm->sp;
    obj_t *quoted_expr = read(vm, rdr);
    obj_t *quote = mk_cons(vm, quoted_expr, the_empty_list);
    popn(vm, vm->sp - sp);
    return mk_cons(vm, quote_sym, quote);
}

obj_t *read_list(VM *vm, Reader *rdr) {
    skip_whitespace_and_comments(rdr);
    if (rdr->cur == EOF)
        return mk_err(vm, "unexpected EOF");

    get_next_char(rdr);
    if (rdr->cur == ')') {
        push(vm, the_empty_list);
        return the_empty_list;
    }
    ungetc(rdr->cur, rdr->in);

    int sp = vm->sp;
    obj_t *car_obj;
    obj_t *cdr_obj;

    car_obj = read(vm, rdr);
    if (!car_obj) {
        return mk_err(vm, "unexpected EOF");
    }
    if (is_error(car_obj)) {
        return car_obj;
    }

    skip_whitespace_and_comments(rdr);

    if (rdr->cur == '.') {
        get_next_char(rdr);
        cdr_obj = read(vm, rdr);
        if (is_error(cdr_obj))
            return car_obj;

        if (rdr->cur != ')') {
            return mk_err(vm, "expected ')'");
        }

        get_next_char(rdr);

        return mk_cons(vm, car_obj, cdr_obj);
    }

    cdr_obj = read_list(vm, rdr);
    if (!cdr_obj) {
        return mk_err(vm, "unexpected EOF");
    }
    if (is_error(cdr_obj)) {
        return cdr_obj;
    }

    popn(vm, vm->sp - sp);

    return mk_cons(vm, car_obj, cdr_obj);
}

obj_t *readfile(VM *vm, char *fname) {

    FILE *infile;
    infile = fopen(fname, "r");

    if (!infile) {
        return mk_err(vm, "could not open %s", fname);
    }

    Reader *rdr = reader_new(infile);

    while (!feof(infile)) {
        int sp = vm->sp;
        obj_t *object = eval(vm, universe, read(vm, rdr));
        if (object && object->type == OBJ_ERR) {
            popn(vm, vm->sp - sp);
            println(object);
            break;
        }
        popn(vm, vm->sp - sp);
    }

    reader_delete(rdr);

    return NULL;
}

obj_t *read(VM *vm, Reader *rdr) {

    skip_whitespace_and_comments(rdr);
    get_next_char(rdr);

    if (reader_eof(rdr))
        return NULL;

    obj_t *result;
    if (rdr->cur == '#') {
        result = read_constant(vm, rdr);
    } else if (isdigit(rdr->cur) || (rdr->cur == '-' && isdigit(get_peek_char(rdr)))) {
        result = read_number(vm, rdr);
    } else if (is_initial(rdr->cur)) {
        result = read_symbol(vm, rdr);
    } else if (rdr->cur == '"') {
        result = read_string(vm, rdr);
    } else if (rdr->cur == '(') {
        result = read_list(vm, rdr);
    } else if (rdr->cur == '\'') {
        result = read_quote(vm, rdr, quote_sym);
    } else if (rdr->cur == '`') {
        result = read_quote(vm, rdr, quasiquote_sym);
    } else if (rdr->cur == ',') {
        result = read_quote(vm, rdr, unquote_sym);
    } else if (rdr->cur == ')') {
        result = mk_err(vm, "unexpected ')'");
    } else if (rdr->cur == '.') {
        result = mk_err(vm, "unexpected '.'");
    } else {
        result = mk_err(vm, "unknown character %c", rdr->cur);
    }

    return result;
}
