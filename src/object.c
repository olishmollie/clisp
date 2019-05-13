#include <math.h>
#include <stdarg.h>

#include "numbers.h"
#include "object.h"

obj_t *obj_new(VM *vm, object_type type) {
    obj_t *object = malloc(sizeof(obj_t));

    object->type = type;
    object->marked = 0;

    if (vm->obj_count >= vm->gc_threshold) {
        gc(vm);
        vm->gc_threshold = vm->obj_count * 2;
    }

    object->next = vm->alloc_list;
    vm->alloc_list = object;

    vm->obj_count++;

    return object;
}

obj_t *mk_cons(VM *vm, obj_t *car, obj_t *cdr) {
    push(vm, car);
    push(vm, cdr);

    obj_t *object = obj_new(vm, OBJ_PAIR);

    object->car = car;
    object->cdr = cdr;

    pop(vm);
    pop(vm);

    push(vm, object);

    return object;
}

obj_t *mk_vec(VM *vm, obj_t **objects, int size) {
    obj_t *vec = obj_new(vm, OBJ_VEC);
    vec->size = size;
    vec->objects = objects;
    return vec;
}

obj_t *mk_num_from_str(VM *vm, char *str, int is_decimal, int is_fractional) {
    obj_t *num = obj_new(vm, OBJ_NUM);

    if (is_decimal) {
        strtok(str, ".");
        char *f = strtok(NULL, ".");

        long denom = (long) pow(10.0, (double) strlen(f));

        num->numer = (long) (strtod(str, NULL) * denom) + strtol(f, NULL, 10);
        num->denom = denom;
    }
    else if (is_fractional) {
        char *numer = strtok(str, "/");
        char *denom = strtok(NULL, "/");

        num->numer = strtol(numer, NULL, 10);
        num->denom = strtol(denom, NULL, 10);
    }
    else {
        num->numer = strtol(str, NULL, 10);
        num->denom = 1l;
    }

    num = reduce(vm, num);
    push(vm, num);

    return num;
}

obj_t *mk_num_from_long(VM *vm, long numer, long denom) {
    if (denom == 0) {
        return mk_err(vm, "division by zero");
    }

    obj_t *num = obj_new(vm, OBJ_NUM);

    num->numer = numer;
    num->denom = denom;

    num = reduce(vm, num);

    push(vm, num);
    return num;
}

char *num_to_string(obj_t *num) {
    char *buf = malloc(sizeof(char) * MAX_STRING_LENGTH);
    if (num->denom == 1) {
        snprintf(buf, MAX_STRING_LENGTH - 1, "%li", num->numer);
    } else {
        snprintf(buf, MAX_STRING_LENGTH - 1, "%li/%li", num->numer, num->denom);
    }
    return buf;
}

obj_t *mk_sym(VM *vm, char *name) {
    obj_t *object;

    if ((object = table_get(symbol_table, name))) {
        push(vm, object);
        return object;
    }

    object = obj_new(vm, OBJ_SYM);
    object->sym = malloc(sizeof(char) * (strlen(name) + 1));
    strcpy(object->sym, name);

    table_put(symbol_table, object->sym, object);

    push(vm, object);
    return object;
}

obj_t *mk_string(VM *vm, char *str) {
    obj_t *object = obj_new(vm, OBJ_STR);
    object->str = malloc(sizeof(char) * (strlen(str) + 1));
    strcpy(object->str, str);
    push(vm, object);
    return object;
}

obj_t *mk_builtin(VM *vm, char *name, builtin proc) {
    obj_t *object = obj_new(vm, OBJ_BUILTIN);

    object->name = malloc(sizeof(char) * (strlen(name) + 1));
    strcpy(object->name, name);

    object->proc = proc;

    push(vm, object);
    return object;
}

obj_t *mk_fun(VM *vm, obj_t *env, obj_t *params, obj_t *body) {
    obj_t *object = obj_new(vm, OBJ_FUN);

    object->env = env;
    object->params = params;
    object->body = body;
    object->variadic = is_list(object->params) ? 0 : 1;

    push(vm, object);
    return object;
}

obj_t *mk_char(VM *vm, char c) {
    obj_t *object = obj_new(vm, OBJ_CHAR);
    object->character = c;
    push(vm, object);
    return object;
}

obj_t *mk_bool(VM *vm, int value) {
    obj_t *object = obj_new(vm, OBJ_BOOL);
    object->boolean = value;
    push(vm, object);
    return object;
}

obj_t *mk_nil(VM *vm) {
    obj_t *object = obj_new(vm, OBJ_NIL);
    push(vm, object);
    return object;
}

obj_t *mk_err(VM *vm, char *fmt, ...) {
    obj_t *object = obj_new(vm, OBJ_ERR);

    va_list args;
    va_start(args, fmt);
    object->err = malloc(sizeof(char) * 512);
    vsnprintf(object->err, 511, fmt, args);
    va_end(args);

    push(vm, object);
    return object;
}

obj_t *mk_env(VM *vm) {
    obj_t *frame = mk_cons(vm, the_empty_list, the_empty_list);
    obj_t *env = mk_cons(vm, frame, the_empty_list);
    return env;
}

void add_binding_to_frame(VM *vm, obj_t *frame, obj_t *symbol, obj_t *object) {
    set_car(frame, mk_cons(vm, symbol, car(frame)));
    set_cdr(frame, mk_cons(vm, object, cdr(frame)));
}

obj_t *env_define(VM *vm, obj_t *env, obj_t *symbol, obj_t *value) {
    obj_t *frame;
    obj_t *symbols;
    obj_t *values;

    frame = car(env);
    symbols = car(frame);
    values = cdr(frame);

    /* for variadic definitions */
    if (!is_list(symbols)) {
        add_binding_to_frame(vm, frame, symbol, value);
        return NULL;
    }

    while (!is_the_empty_list(symbols)) {
        if (car(symbols) == symbol) {
            set_car(values, value);
            return NULL;
        }
        symbols = cdr(symbols);
        values = cdr(values);
    }

    add_binding_to_frame(vm, frame, symbol, value);

    return NULL;
}

obj_t *env_set(VM *vm, obj_t *env, obj_t *symbol, obj_t *value) {
    obj_t *frame;
    obj_t *symbols;
    obj_t *values;

    while (!is_the_empty_list(env)) {
        frame = car(env);
        symbols = car(frame);
        values = cdr(frame);

        while (!is_the_empty_list(symbols)) {
            if (car(symbols) == symbol) {
                set_car(values, value);
                return NULL;
            }
            symbols = cdr(symbols);
            values = cdr(values);
        }
        env = cdr(env);
    }

    return mk_err(vm, "unbound symbol '%s'", symbol->sym);
}

obj_t *env_lookup(VM *vm, obj_t *env, obj_t *symbol) {
    obj_t *frame;
    obj_t *symbols;
    obj_t *objects;

    while (!is_the_empty_list(env)) {
        frame = car(env);
        symbols = car(frame);
        objects = cdr(frame);

        while (is_pair(symbols)) {
            if (symbol == car(symbols)) {
                return car(objects);
            }

            symbols = cdr(symbols);
            objects = cdr(objects);
        }

        /* for variadic functions */
        if (symbols == symbol) {
            return objects;
        }

        env = cdr(env);
    }

    return mk_err(vm, "unbound symbol '%s'", symbol->sym);
}

obj_t *mk_frame(VM *vm, obj_t *symbols, obj_t *values) {
    return mk_cons(vm, symbols, values);
}

obj_t *env_extend(VM *vm, obj_t *env, obj_t *symbols, obj_t *values) {
    obj_t *frame = mk_frame(vm, symbols, values);
    return mk_cons(vm, frame, env);
}

int is_the_empty_list(obj_t *object) { return object == the_empty_list; }
int is_false(obj_t *object) { return object == false; }
int is_true(obj_t *object) { return !is_false(object); }

int is_pair(obj_t *object) { return object->type == OBJ_PAIR; }

int is_list(obj_t *object) {
    while (is_pair(object)) {
        object = cdr(object);
    }
    return is_the_empty_list(object);
}

int is_vector(obj_t *object) {
    return object->type == OBJ_VEC;
}

int is_num(obj_t *object) { return object->type == OBJ_NUM; }
int is_integer(obj_t *object) { return is_num(object) && object->denom == 1; }
int is_symbol(obj_t *object) { return object->type == OBJ_SYM; }
int is_boolean(obj_t *object) { return object == true || object == false; }
int is_char(obj_t *object) { return object->type == OBJ_CHAR; }
int is_string(obj_t *object) { return object->type == OBJ_STR; }
int is_builtin(obj_t *object) { return object->type == OBJ_BUILTIN; }
int is_fun(obj_t *object) { return object->type == OBJ_FUN; }
int is_error(obj_t *object) { return object->type == OBJ_ERR; }

static char *type_names[10] = {"number", "symbol", "string",
                               "pair", "bool", "char", "builtin",
                               "function", "nil", "error"};

char *type_name(object_type type) {
    if (type < 0 || type > 9) {
        return "unknown";
    }
    return type_names[type];
}

int length(obj_t *object) {
    int l = 0;
    while (!is_the_empty_list(object)) {
        l++;
        object = cdr(object);
    }
    return l;
}

/* list fns ---------------------------------------------------------------- */

obj_t *car(obj_t *pair) { return pair->car; }
obj_t *cdr(obj_t *pair) { return pair->cdr; }
void set_car(obj_t *pair, obj_t *value) { pair->car = value; }
void set_cdr(obj_t *pair, obj_t *value) { pair->cdr = value; }

/* printing ---------------------------------------------------------------- */

void print(obj_t *object);

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

void print_cons(obj_t *object) {
    putchar('(');
    obj_t *p = object;
    while (1) {
        print(car(p));
        obj_t *cdr_obj = cdr(p);
        if (cdr_obj->type != OBJ_PAIR) {
            if (cdr_obj->type != OBJ_NIL) {
                printf(" . ");
                print(cdr_obj);
            }
            putchar(')');
            break;
        }
        putchar(' ');
        p = cdr(p);
    }
}

void print(obj_t *object) {
    if (object) {
        switch (object->type) {
        case OBJ_NUM:
            if (object->denom == 1) {
                printf("%li", object->numer);
            } else {
                double d = (double) object->numer / object->denom;
                double l = round(d * 100000);
                if (fabs(d * 100000 - l) > 0.1) {
                    printf("%li/%li", object->numer, object->denom);
                } else {
                    printf("%g", d);
                }
            }
            break;
        case OBJ_SYM:
            printf("%s", object->sym);
            break;
        case OBJ_STR:
            print_rawstr(object->str);
            break;
        case OBJ_PAIR:
            print_cons(object);
            break;
        case OBJ_VEC:
            printf("#(");
            for (int i = 0; i < object->size; i++) {
                print(object->objects[i]);
                if (i != object->size - 1) {
                    printf(" ");
                }
            }
            printf(")");
            break;
        case OBJ_BOOL:
            printf("%s", object->boolean ? "#t" : "#f");
            break;
        case OBJ_CHAR:
            if (object->character == '\n')
                printf("#\\newline");
            else if (object->character == '\t')
                printf("#\\tab");
            else if (object->character == ' ')
                printf("#\\space");
            else {
                printf("#\\%c", object->character);
            }
            break;
        case OBJ_BUILTIN:
            printf("#<procedure '%s'>", object->name);
            break;
        case OBJ_FUN:
            printf("#<procedure>");
            break;
        case OBJ_ERR:
            printf("error: %s", object->err);
            break;
        case OBJ_NIL:
            printf("()");
            break;
        default:
            printf("Cannot print unknown obj_t type\n");
        }
    }
}

void println(obj_t *object) {
    print(object);
    putchar('\n');
}

void obj_delete(obj_t *object) {
    if (object) {
        if (is_symbol(object))
            free(object->sym);
        else if (is_string(object))
            free(object->sym);
        else if (is_builtin(object))
            free(object->name);
        else if (is_error(object))
            free(object->err);

        free(object);
        vm->obj_count--;
    }
}
