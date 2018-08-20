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

obj *read_character(reader *rdr) {
    char buf[MAXSTRLEN];
    int i = 0;
    rdr->cur = getc(rdr->in);

    while (isalnum(rdr->cur)) {
        buf[i++] = rdr->cur;
        rdr->cur = getc(rdr->in);
    }
    buf[i] = '\0';

    if (i == 1)
        return mk_char(buf[0]);
    else {
        if (strcmp(buf, "newline") == 0)
            return mk_char('\n');
        if (strcmp(buf, "tab") == 0)
            return mk_char('\t');

        ungetc(rdr->cur, rdr->in);
        return mk_err("invalid character constant");
    }
}

obj *read_constant(reader *rdr) {
    rdr->cur = getc(rdr->in);
    if (rdr->cur == '\\') {
        return read_character(rdr);
    } else if (rdr->cur == 't')
        return true;
    return false;
}

obj *read_symbol(reader *rdr) {
    char sym[MAXSTRLEN];
    int i = 0;

    while (!is_delim(rdr->cur)) {
        sym[i++] = rdr->cur;
        rdr->cur = getc(rdr->in);
    }
    sym[i] = '\0';

    ungetc(rdr->cur, rdr->in);
    return mk_sym(sym);
}

obj *read_string(reader *rdr) {
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

    return mk_string(str);
}

obj *read_number(reader *rdr) {
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
        return mk_num_from_str(num);
    }

    return mk_err("invalid number syntax");
}

obj *read_quote(reader *rdr) {
    obj *quote = cons(read(rdr), the_empty_list);
    return cons(quote_sym, quote);
}

obj *read_list(reader *rdr) {

    obj *car_obj;
    obj *cdr_obj;

    skipwhitespace(rdr);

    if (rdr->cur == EOF)
        return mk_err("expected ')'");

    rdr->cur = getc(rdr->in);
    if (rdr->cur == ')') {
        return the_empty_list;
    }
    ungetc(rdr->cur, rdr->in);

    car_obj = read(rdr);

    if (is_error(car_obj))
        return car_obj;

    skipwhitespace(rdr);

    if (rdr->cur == '.') {
        rdr->cur = getc(rdr->in);
        cdr_obj = read(rdr);
        if (is_error(cdr_obj))
            return car_obj;

        if (rdr->cur != ')') {
            return mk_err("expected ')'");
        }

        rdr->cur = getc(rdr->in);

        return cons(car_obj, cdr_obj);
    }

    cdr_obj = read_list(rdr);
    if (is_error(cdr_obj))
        return cdr_obj;

    return cons(car_obj, cdr_obj);
}

obj *read(reader *rdr) {

    skipwhitespace(rdr);

    if (rdr->cur != EOF) {
        rdr->cur = getc(rdr->in);

        if (rdr->cur == '#') {
            return read_constant(rdr);
        } else if (isdigit(rdr->cur) ||
                   (rdr->cur == '-' && isdigit(peek(rdr)))) {
            return read_number(rdr);
        } else if (is_initial(rdr->cur)) {
            return read_symbol(rdr);
        } else if (rdr->cur == '"') {
            return read_string(rdr);
        } else if (rdr->cur == '(') {
            return read_list(rdr);
        } else if (rdr->cur == '\'') {
            return read_quote(rdr);
        } else if (rdr->cur == ')') {
            return mk_err("unexpected ')'");
        } else if (rdr->cur == '.') {
            return mk_err("unexpected '.'");
        }

        return mk_err("unknown character %c", rdr->cur);
    }

    return NULL;
}
