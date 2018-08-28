#include "builtins.h"

obj_t *builtin_plus(VM *vm, obj_t *args) {
    ARG_TYPECHECK(args, "plus", OBJ_NUM);
    long res = 0;
    while (!is_the_empty_list(args)) {
        res += car(args)->num;
        args = cdr(args);
    }
    return mk_num_from_long(vm, res);
}

obj_t *builtin_minus(VM *vm, obj_t *args) {
    ARG_TYPECHECK(args, "minus", OBJ_NUM);
    if (args == the_empty_list)
        return mk_err("minus passed no arguments");

    /* unary minus */
    if (is_the_empty_list(cdr(args)))
        return mk_num_from_long(vm, -1 * car(args)->num);

    long res = car(args)->num;
    args = cdr(args);
    while (!is_the_empty_list(args)) {
        res -= car(args)->num;
        args = cdr(args);
    }

    return mk_num_from_long(vm, res);
}

obj_t *builtin_times(VM *vm, obj_t *args) {
    ARG_TYPECHECK(args, "times", OBJ_NUM);
    if (args == the_empty_list)
        return mk_err("times passed no arguments");
    long res = car(args)->num;
    args = cdr(args);
    while (!is_the_empty_list(args)) {
        res *= car(args)->num;
        args = cdr(args);
    }

    return mk_num_from_long(vm, res);
}

obj_t *builtin_divide(VM *vm, obj_t *args) {
    ARG_TYPECHECK(args, "divide", OBJ_NUM);
    if (args == the_empty_list)
        return mk_err("divide passed no arguments");
    long res = car(args)->num;
    args = cdr(args);
    while (!is_the_empty_list(args)) {
        if (car(args)->num == 0)
            return mk_err("division by zero");
        res /= car(args)->num;
        args = cdr(args);
    }

    return mk_num_from_long(vm, res);
}

obj_t *builtin_remainder(VM *vm, obj_t *args) {
    ARG_TYPECHECK(args, "remainder", OBJ_NUM);
    if (args == the_empty_list)
        return mk_err("remainder passed no arguments");
    long res = car(args)->num;
    args = cdr(args);
    while (!is_the_empty_list(args)) {
        if (car(args)->num == 0)
            return mk_err("division by zero");
        res %= car(args)->num;
        args = cdr(args);
    }

    return mk_num_from_long(vm, res);
}

obj_t *builtin_gt(VM *vm, obj_t *args) {
    ARG_TYPECHECK(args, "gt", OBJ_NUM);
    ARG_NUMCHECK(args, "gt", 2);

    obj_t *x = car(args);
    obj_t *y = cadr(args);

    return x->num > y->num ? true : false;
}

obj_t *builtin_gte(VM *vm, obj_t *args) {
    ARG_TYPECHECK(args, "gte", OBJ_NUM);
    ARG_NUMCHECK(args, "gte", 2);

    obj_t *x = car(args);
    obj_t *y = cadr(args);

    return x->num >= y->num ? true : false;
}

obj_t *builtin_lt(VM *vm, obj_t *args) {
    ARG_TYPECHECK(args, "lt", OBJ_NUM);
    ARG_NUMCHECK(args, "lt", 2);

    obj_t *x = car(args);
    obj_t *y = cadr(args);

    return x->num < y->num ? true : false;
}

obj_t *builtin_lte(VM *vm, obj_t *args) {
    ARG_TYPECHECK(args, "lte", OBJ_NUM);
    ARG_NUMCHECK(args, "lte", 2);

    obj_t *x = car(args);
    obj_t *y = cadr(args);

    return x->num <= y->num ? true : false;
}

obj_t *builtin_is_null(VM *vm, obj_t *args) {
    ARG_NUMCHECK(args, "null?", 1);
    return car(args) == the_empty_list ? true : false;
}

obj_t *builtin_is_boolean(VM *vm, obj_t *args) {
    ARG_NUMCHECK(args, "boolean?", 1);
    return is_boolean(car(args)) ? true : false;
}

obj_t *builtin_is_symbol(VM *vm, obj_t *args) {
    ARG_NUMCHECK(args, "symbol?", 1);
    return is_symbol(car(args)) ? true : false;
}

obj_t *builtin_is_num(VM *vm, obj_t *args) {
    ARG_NUMCHECK(args, "number?", 1);
    return is_num(car(args)) ? true : false;
}

obj_t *builtin_is_char(VM *vm, obj_t *args) {
    ARG_NUMCHECK(args, "char?", 1);
    return is_char(car(args)) ? true : false;
}

obj_t *builtin_is_string(VM *vm, obj_t *args) {
    ARG_NUMCHECK(args, "string?", 1);
    return is_string(car(args)) ? true : false;
}

obj_t *builtin_is_pair(VM *vm, obj_t *args) {
    ARG_NUMCHECK(args, "pair?", 1);
    return is_pair(car(args)) ? true : false;
}

obj_t *builtin_is_proc(VM *vm, obj_t *args) {
    ARG_NUMCHECK(args, "proc?", 1);
    return is_builtin(car(args)) ? true : false;
}

obj_t *builtin_cons(VM *vm, obj_t *args) {
    ARG_NUMCHECK(args, "cons", 2);
    obj_t *car_obj = car(args);
    obj_t *cdr_obj = cadr(args);
    return mk_cons(vm, car_obj, cdr_obj);
}

obj_t *builtin_car(VM *vm, obj_t *args) {
    ARG_NUMCHECK(args, "car", 1);
    ARG_TYPECHECK(args, "car", OBJ_PAIR);
    return caar(args);
}

obj_t *builtin_cdr(VM *vm, obj_t *args) {
    ARG_NUMCHECK(args, "cdr", 1);
    ARG_TYPECHECK(args, "cdr", OBJ_PAIR);
    return cdar(args);
}

obj_t *builtin_setcar(VM *vm, obj_t *args) {
    ARG_NUMCHECK(args, "set-car!", 2);
    FIG_ASSERT(is_pair(car(args)), "invalid argument passed to set-car!");
    obj_t *arg = car(args);
    arg->car = cadr(args);
    return NULL;
}

obj_t *builtin_setcdr(VM *vm, obj_t *args) {
    ARG_NUMCHECK(args, "set-cdr!", 2);
    FIG_ASSERT(is_pair(car(args)), "invalid argument passed to set-cdr!");
    obj_t *arg = car(args);
    arg->cdr = cadr(args);
    return NULL;
}

obj_t *builtin_list(VM *vm, obj_t *args) { return args; }

obj_t *builtin_char_to_int(VM *vm, obj_t *args) {
    ARG_NUMCHECK(args, "char->int", 1);
    FIG_ASSERT(is_char(car(args)), "invalid argument passed to char->int");
    obj_t *arg = car(args);
    return mk_num_from_long(vm, arg->character);
}

obj_t *builtin_int_to_char(VM *vm, obj_t *args) {
    ARG_NUMCHECK(args, "int->char", 1);
    FIG_ASSERT(is_num(car(args)), "invalid argument passed to int->char");
    obj_t *arg = car(args);
    FIG_ASSERT(arg->num >= 0 && arg->num <= 9,
               "invalid argument passed to int->char");
    return mk_char(vm, arg->num);
}

obj_t *builtin_number_to_string(VM *vm, obj_t *args) {
    ARG_NUMCHECK(args, "number->string", 1);
    FIG_ASSERT(is_num(car(args)), "invalid argument passed to number->string");
    obj_t *arg = car(args);
    return mk_string(vm, num_to_string(arg));
}

obj_t *builtin_string_to_number(VM *vm, obj_t *args) {
    ARG_NUMCHECK(args, "string->number", 1);
    FIG_ASSERT(is_string(car(args)),
               "invalid argument passed to string->number");
    obj_t *arg = car(args);
    return mk_num_from_str(vm, arg->str);
}

obj_t *builtin_symbol_to_string(VM *vm, obj_t *args) {
    ARG_NUMCHECK(args, "symbol->string", 1);
    FIG_ASSERT(is_symbol(car(args)),
               "invalid argument passed to symbol->string");
    obj_t *arg = car(args);
    return mk_string(vm, arg->sym);
}

obj_t *builtin_string_to_symbol(VM *vm, obj_t *args) {
    ARG_NUMCHECK(args, "symbol->string", 1);
    FIG_ASSERT(is_string(car(args)),
               "invalid argument passed to string->symbol");
    obj_t *arg = car(args);
    return mk_sym(vm, arg->sym);
}

obj_t *builtin_is_equal(VM *vm, obj_t *args) {
    ARG_NUMCHECK(args, "eq?", 2);

    obj_t *x = car(args);
    obj_t *y = cadr(args);

    if (x->type != y->type)
        return false;

    switch (x->type) {
    case OBJ_NUM:
        return x->num == y->num ? true : false;
    case OBJ_CHAR:
        return x->character == y->character ? true : false;
    default:
        return x == y ? true : false;
    }
}

obj_t *readfile(VM *vm, char *fname) {

    FILE *infile;
    infile = fopen(fname, "r");

    if (!infile)
        return mk_err("could not open %s", fname);

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
    free(universe);
    table_delete(symbol_table);
}

obj_t *builtin_exit(VM *vm, obj_t *args) {
    cleanup(vm);
    exit(0);
    return NULL;
}
