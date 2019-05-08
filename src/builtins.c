#include "builtins.h"
#include "numbers.h"

obj_t *builtin_plus(VM *vm, obj_t *args) {
    obj_t *res = mk_num_from_long(vm, 0l, 1l);
    while (!is_the_empty_list(args)) {
        obj_t *x = car(args);
        if (!is_num(x)) {
            return mk_err(vm, "invalid argument passed to '+'");
        }
        res = num_add(vm, res, x);
        args = cdr(args);
    }
    return res;
}

obj_t *builtin_minus(VM *vm, obj_t *args) {
    if (args == the_empty_list)
        return mk_err(vm, "incorrect argument count for '-'");

    obj_t *res = car(args);
    if (!is_num(res)) {
        return mk_err(vm, "invalid argument passed to '-'");
    }

    /* unary minus */
    if (is_the_empty_list(cdr(args))) {
        long numer = -1 * res->numer;
        long denom = res->denom;
        return mk_num_from_long(vm, numer, denom);
    }

    args = cdr(args);
    while (!is_the_empty_list(args)) {
        obj_t *x = car(args);
        if (!is_num(x)) {
            return mk_err(vm, "invalid argument passed to '-'");
        }
        res = num_sub(vm, res, x);
        args = cdr(args);
    }

    return res;
}

obj_t *builtin_times(VM *vm, obj_t *args) {
    obj_t *res = mk_num_from_long(vm, 1l, 1l);
    while (!is_the_empty_list(args)) {
        obj_t *x = car(args);
        if (!is_num(x)) {
            return mk_err(vm, "invalid argument passed to '*'");
        }
        res = num_mul(vm, res, x);
        args = cdr(args);
    }

    return res;
}

obj_t *builtin_divide(VM *vm, obj_t *args) {
    if (args == the_empty_list) {
        return mk_err(vm, "incorrect argument count for '/'");
    }

    obj_t *res = car(args);
    if (!is_num(res)) {
        return mk_err(vm, "invalid argument passed to '/'");
    }

    args = cdr(args);
    while (!is_the_empty_list(args)) {
        obj_t *x = car(args);
        if (!is_num(x)) {
            return mk_err(vm, "invalid argument passed to '/'");
        }
        if (x->numer == 0) {
            return mk_err(vm, "division by zero");
        }
        res = num_div(vm, res, x);
        args = cdr(args);
    }

    return res;
}

obj_t *builtin_remainder(VM *vm, obj_t *args) {
    if (args == the_empty_list) {
        return mk_err(vm, "incorrect argument count for 'mod'");
    }

    obj_t *res = car(args);
    if (!is_num(res) || res->denom != 1) {
        return mk_err(vm, "invalid argument passed to 'mod'");
    }

    args = cdr(args);
    while (!is_the_empty_list(args)) {
        obj_t *x = car(args);
        if (!is_num(res) || res->denom != 1) {
            return mk_err(vm, "invalid argument passed to 'mod'");
        }
        if (res->numer == 0) {
            return mk_err(vm, "division by zero");
        }
        res = num_mod(vm, res, x);
        args = cdr(args);
    }

    return res;
}

obj_t *builtin_gt(VM *vm, obj_t *args) {
    ARG_TYPECHECK(vm, args, "gt", OBJ_NUM);
    ARG_NUMCHECK(vm, args, "gt", 2);

    obj_t *a = car(args);
    obj_t *b = cadr(args);

    return num_gt(vm, a, b);
}

obj_t *builtin_gte(VM *vm, obj_t *args) {
    ARG_TYPECHECK(vm, args, "gte", OBJ_NUM);
    ARG_NUMCHECK(vm, args, "gte", 2);

    obj_t *a = car(args);
    obj_t *b = cadr(args);

    return num_gte(vm, a, b);
}

obj_t *builtin_lt(VM *vm, obj_t *args) {
    ARG_TYPECHECK(vm, args, "lt", OBJ_NUM);
    ARG_NUMCHECK(vm, args, "lt", 2);

    obj_t *a = car(args);
    obj_t *b = cadr(args);

    return num_lt(vm, a, b);
}

obj_t *builtin_lte(VM *vm, obj_t *args) {
    ARG_TYPECHECK(vm, args, "lte", OBJ_NUM);
    ARG_NUMCHECK(vm, args, "lte", 2);

    obj_t *a = car(args);
    obj_t *b = cadr(args);

    return num_lte(vm, a, b);
}

obj_t *builtin_is_null(VM *vm, obj_t *args) {
    ARG_NUMCHECK(vm, args, "null?", 1);
    return car(args) == the_empty_list ? true : false;
}

obj_t *builtin_is_boolean(VM *vm, obj_t *args) {
    ARG_NUMCHECK(vm, args, "boolean?", 1);
    return is_boolean(car(args)) ? true : false;
}

obj_t *builtin_is_symbol(VM *vm, obj_t *args) {
    ARG_NUMCHECK(vm, args, "symbol?", 1);
    return is_symbol(car(args)) ? true : false;
}

obj_t *builtin_is_num(VM *vm, obj_t *args) {
    ARG_NUMCHECK(vm, args, "number?", 1);
    obj_t *num = car(args);
    return is_num(num) ? true : false;
}

obj_t *builtin_is_char(VM *vm, obj_t *args) {
    ARG_NUMCHECK(vm, args, "char?", 1);
    return is_char(car(args)) ? true : false;
}

obj_t *builtin_is_string(VM *vm, obj_t *args) {
    ARG_NUMCHECK(vm, args, "string?", 1);
    return is_string(car(args)) ? true : false;
}

obj_t *builtin_is_pair(VM *vm, obj_t *args) {
    ARG_NUMCHECK(vm, args, "pair?", 1);
    return is_pair(car(args)) ? true : false;
}

obj_t *builtin_is_list(VM *vm, obj_t *args) {
    ARG_NUMCHECK(vm, args, "list?", 1);
    return is_list(car(args)) ? true : false;
}

obj_t *builtin_is_proc(VM *vm, obj_t *args) {
    ARG_NUMCHECK(vm, args, "proc?", 1);
    return is_builtin(car(args)) ? true : false;
}

obj_t *builtin_cons(VM *vm, obj_t *args) {
    ARG_NUMCHECK(vm, args, "cons", 2);
    obj_t *car_obj = car(args);
    obj_t *cdr_obj = cadr(args);
    return mk_cons(vm, car_obj, cdr_obj);
}

obj_t *builtin_car(VM *vm, obj_t *args) {
    ARG_NUMCHECK(vm, args, "car", 1);
    ARG_TYPECHECK(vm, args, "car", OBJ_PAIR);
    return caar(args);
}

obj_t *builtin_cdr(VM *vm, obj_t *args) {
    ARG_NUMCHECK(vm, args, "cdr", 1);
    ARG_TYPECHECK(vm, args, "cdr", OBJ_PAIR);
    return cdar(args);
}

obj_t *builtin_setcar(VM *vm, obj_t *args) {
    ARG_NUMCHECK(vm, args, "set-car!", 2);
    FIG_ASSERT(vm, is_pair(car(args)), "invalid argument passed to set-car!");
    obj_t *arg = car(args);
    arg->car = cadr(args);
    return NULL;
}

obj_t *builtin_setcdr(VM *vm, obj_t *args) {
    ARG_NUMCHECK(vm, args, "set-cdr!", 2);
    FIG_ASSERT(vm, is_pair(car(args)), "invalid argument passed to set-cdr!");
    obj_t *arg = car(args);
    arg->cdr = cadr(args);
    return NULL;
}

obj_t *builtin_list(VM *vm, obj_t *args) {
    push(vm, args);
    return args;
}

obj_t *builtin_char_to_int(VM *vm, obj_t *args) {
    ARG_NUMCHECK(vm, args, "char->int", 1);
    FIG_ASSERT(vm, is_char(car(args)), "invalid argument passed to char->int");
    obj_t *arg = car(args);
    return mk_num_from_long(vm, arg->character, 1l);
}

obj_t *builtin_int_to_char(VM *vm, obj_t *args) {
    ARG_NUMCHECK(vm, args, "int->char", 1);
    FIG_ASSERT(vm, is_num(car(args)) && car(args)->denom == 1, "invalid argument passed to int->char");
    obj_t *arg = car(args);
    return mk_char(vm, arg->numer);
}

obj_t *builtin_number_to_string(VM *vm, obj_t *args) {
    ARG_NUMCHECK(vm, args, "number->string", 1);
    FIG_ASSERT(vm, is_num(car(args)), "invalid argument passed to number->string");

    obj_t *arg = car(args);

    char *num = num_to_string(arg);
    obj_t *result = mk_string(vm, num);

    free(num);

    return result;
}

obj_t *builtin_string_to_number(VM *vm, obj_t *args) {
    ARG_NUMCHECK(vm, args, "string->number", 1);
    FIG_ASSERT(vm, is_string(car(args)),
               "invalid argument passed to string->number");
    obj_t *arg = car(args);
    return mk_num_from_str(vm, arg->str);
}

obj_t *builtin_symbol_to_string(VM *vm, obj_t *args) {
    ARG_NUMCHECK(vm, args, "symbol->string", 1);
    FIG_ASSERT(vm, is_symbol(car(args)),
               "invalid argument passed to symbol->string");
    obj_t *arg = car(args);
    return mk_string(vm, arg->sym);
}

obj_t *builtin_string_to_symbol(VM *vm, obj_t *args) {
    ARG_NUMCHECK(vm, args, "symbol->string", 1);
    FIG_ASSERT(vm, is_string(car(args)),
               "invalid argument passed to string->symbol");
    obj_t *arg = car(args);
    return mk_sym(vm, arg->sym);
}

obj_t *builtin_is_equal(VM *vm, obj_t *args) {
    ARG_NUMCHECK(vm, args, "eq?", 2);

    obj_t *a = car(args);
    obj_t *b = cadr(args);

    if (a->type != b->type) {
        return false;
    }

    switch (a->type) {
    case OBJ_NUM:
        return num_eq(vm, a, b);
    case OBJ_CHAR:
        return a->character == b->character ? true : false;
    default:
        return a == b ? true : false;
    }
}

obj_t *builtin_string_append(VM *vm, obj_t *args) {
    ARG_TYPECHECK(vm, args, "string-append", OBJ_STR);
    char buf[MAX_STRING_LENGTH];
    int i = 0;
    while (!is_the_empty_list(args)) {
        char *sym = car(args)->sym;
        while (*sym) {
            buf[i++] = *sym++;
        }
        args = cdr(args);
    }
    buf[i] = '\0';

    return mk_string(vm, buf);
}

void display(obj_t *object) {
    switch (object->type) {
        case OBJ_STR:
            printf("%s", object->str);
            break;
        default:
            print(object);
    }
}

obj_t *builtin_display(VM *vm, obj_t *args) {
    FIG_ASSERT(vm, !is_the_empty_list(args), "invalid syntax display");
    while (!is_the_empty_list(args)) {
        display(car(args));
        args = cdr(args);
        printf(" ");
    }
    printf("\n");
    return NULL;
}

obj_t *readfile(VM *vm, char *fname) {

    FILE *infile;
    infile = fopen(fname, "r");

    if (!infile)
        return mk_err(vm, "could not open %s", fname);

    reader *rdr = reader_new(infile);

    while (!feof(infile)) {
        int sp = vm->sp;
        obj_t *object = eval(vm, universe, read(vm, rdr));
        if (object && object->type == OBJ_ERR) {
            println(object);
        }
        popn(vm, vm->sp - sp);
    }

    reader_delete(rdr);

    return NULL;
}

obj_t *builtin_load(VM *vm, obj_t *args) {
    obj_t *f = car(args);
    char *filename = f->str;
    obj_t *res = readfile(vm, filename);
    return res;
}

void cleanup(VM *vm) {
    obj_t *object = vm->alloc_list;
    while (object) {
        obj_delete(object);
        object = object->next;
    }

    free(vm);
    table_delete(symbol_table);
}

obj_t *builtin_exit(VM *vm, obj_t *args) {
    cleanup(vm);
    exit(0);
    return NULL;
}
