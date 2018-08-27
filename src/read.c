#include "read.h"

reader *reader_new(FILE *in) {
    reader *r = malloc(sizeof(reader));
    r->cur = 0;
    r->linenum = 0;
    r->in = in;
    return r;
}

void reader_delete(reader *rdr) {
    /* flush input buffer */
    while ((rdr->cur = getc(rdr->in)) != '\n' && rdr->cur != EOF) {
    }
    if (rdr->in != stdin)
        fclose(rdr->in);
    free(rdr);
}

int peek(reader *rdr) {
    int c = getc(rdr->in);
    ungetc(c, rdr->in);
    return c;
}

int is_delim(int c) {
    return isspace(c) || c == EOF || c == '(' || c == ')' || c == '"' ||
           c == ';';
}

int is_initial(int c) {
    char *allowed = "+-*/%!?<>=&^";
    return isalpha(c) || strchr(allowed, c);
}

void skipwhitespace(reader *rdr) {
    while ((rdr->cur = getc(rdr->in)) != EOF) {
        if (isspace(rdr->cur)) {
            continue;
        } else if (rdr->cur == ';') {
            while (((rdr->cur = getc(rdr->in)) != EOF) && (rdr->cur != '\n')) {
                ;
            }
            continue;
        }
        ungetc(rdr->cur, rdr->in);
        break;
    }
}

int expected_string(reader *rdr, char *str) {
    while (*str != '\0') {
        rdr->cur = getc(rdr->in);
        if (*str++ != rdr->cur)
            return 0;
    }
    if (!is_delim(peek(rdr)))
        return 0;
    return 1;
}

obj_t *read_character(VM *vm, reader *rdr) {
    rdr->cur = getc(rdr->in);

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

    if (!is_delim(peek(rdr)))
        return mk_err("invalid constant");

    return res;
}

obj_t *read_constant(VM *vm, reader *rdr) {
    rdr->cur = getc(rdr->in);
    if (rdr->cur == '\\') {
        return read_character(vm, rdr);
    } else if (rdr->cur == 't') {
        return true;
    } else if (rdr->cur == 'f') {
        return false;
    }

    return mk_err("invalid constant");
}

obj_t *read_symbol(VM *vm, reader *rdr) {
    char sym[MAXSTRLEN];
    int i = 0;

    while (!is_delim(rdr->cur)) {
        sym[i++] = rdr->cur;
        rdr->cur = getc(rdr->in);
    }
    sym[i] = '\0';

    ungetc(rdr->cur, rdr->in);
    return mk_sym(vm, sym);
}

obj_t *read_string(VM *vm, reader *rdr) {
    char str[MAXSTRLEN];
    int i = 0;

    rdr->cur = getc(rdr->in);
    while (rdr->cur != '"') {
        str[i++] = rdr->cur;
        rdr->cur = getc(rdr->in);
        if (feof(rdr->in))
            return mk_err("unclosed string literal");
    }
    str[i] = '\0';

    return mk_string(vm, str);
}

obj_t *read_number(VM *vm, reader *rdr) {
    char num[MAXSTRLEN];
    int i = 0;

    if (rdr->cur == '-') {
        num[i++] = rdr->cur;
        rdr->cur = getc(rdr->in);
    }

    while (isdigit(rdr->cur) && i < MAXSTRLEN) {
        num[i++] = rdr->cur;
        rdr->cur = getc(rdr->in);
    }
    num[i] = '\0';

    if (i == MAXSTRLEN)
        return mk_err("string exceeds max length");

    if (is_delim(rdr->cur)) {
        ungetc(rdr->cur, rdr->in);
        return mk_num_from_str(vm, num);
    }

    return mk_err("invalid number syntax");
}

obj_t *read_quote(VM *vm, reader *rdr) {
    int sp = vm->sp;
    obj_t *quoted_expr = read(vm, rdr);
    obj_t *quote = mk_cons(vm, quoted_expr, the_empty_list);
    popn(vm, vm->sp - sp);
    mk_cons(vm, quote_sym, quote);
    return pop(vm);
}

obj_t *read_list(VM *vm, reader *rdr) {
    skipwhitespace(rdr);
    if (rdr->cur == EOF)
        return mk_err("expected ')'");

    rdr->cur = getc(rdr->in);
    if (rdr->cur == ')') {
        push(vm, the_empty_list);
        return the_empty_list;
    }
    ungetc(rdr->cur, rdr->in);

    int sp = vm->sp;
    obj_t *car_obj;
    obj_t *cdr_obj;

    car_obj = read(vm, rdr);
    if (is_error(car_obj))
        return car_obj;

    skipwhitespace(rdr);

    if (rdr->cur == '.') {
        rdr->cur = getc(rdr->in);
        cdr_obj = read(vm, rdr);
        if (is_error(cdr_obj))
            return car_obj;

        if (rdr->cur != ')') {
            return mk_err("expected ')'");
        }

        rdr->cur = getc(rdr->in);

        return mk_cons(vm, car_obj, cdr_obj);
    }

    cdr_obj = read_list(vm, rdr);
    if (is_error(cdr_obj))
        return cdr_obj;

    popn(vm, vm->sp - sp);

    mk_cons(vm, car_obj, cdr_obj);

    return pop(vm);
}

obj_t *read(VM *vm, reader *rdr) {

    skipwhitespace(rdr);
    rdr->cur = getc(rdr->in);

    obj_t *result;
    if (rdr->cur == '#') {
        result = read_constant(vm, rdr);
    } else if (isdigit(rdr->cur) || (rdr->cur == '-' && isdigit(peek(rdr)))) {
        result = read_number(vm, rdr);
    } else if (is_initial(rdr->cur)) {
        result = read_symbol(vm, rdr);
    } else if (rdr->cur == '"') {
        result = read_string(vm, rdr);
    } else if (rdr->cur == '(') {
        result = read_list(vm, rdr);
    } else if (rdr->cur == '\'') {
        result = read_quote(vm, rdr);
        result = pop(vm);
    } else if (rdr->cur == ')') {
        result = mk_err("unexpected ')'");
    } else if (rdr->cur == '.') {
        result = mk_err("unexpected '.'");
    } else {
        result = mk_err("unknown character %c", rdr->cur);
    }

    return result;
}
