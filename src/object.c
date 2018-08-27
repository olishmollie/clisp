#include "object.h"

obj_t *obj_new(VM *vm, object_type type) {
    obj_t *object = malloc(sizeof(obj_t));
    object->type = type;
    object->marked = 0;

    if (vm->obj_count >= vm->gc_threshold) {
        printf("running garbage collector...\n");
    }
    object->next = vm->alloc_list;
    vm->alloc_list = object;

    return object;
}

obj_t *mk_cons(VM *vm, obj_t *car, obj_t *cdr) {
    obj_t *object = obj_new(vm, OBJ_PAIR);
    object->car = car;
    object->cdr = cdr;

    push(vm, object);
    return object;
}

obj_t *mk_num_from_str(VM *vm, char *numstr) {
    obj_t *object = obj_new(vm, OBJ_NUM);
    object->num = strtol(numstr, NULL, 10);
    push(vm, object);
    return object;
}

obj_t *mk_num_from_long(VM *vm, long num) {
    obj_t *object = obj_new(vm, OBJ_NUM);
    object->num = num;
    push(vm, object);
    return object;
}

char *num_to_string(obj_t *object) {
    char *buf = malloc(sizeof(char) * MAXSTRLEN);
    snprintf(buf, MAXSTRLEN - 1, "%li", object->num);
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

obj_t *mk_fun(VM *vm, env_t *env, obj_t *params, obj_t *body) {
    obj_t *object = obj_new(vm, OBJ_FUN);
    object->env = env;
    object->params = params;
    object->body = body;
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

obj_t *mk_err(char *fmt, ...) {
    obj_t *object = obj_new(vm, OBJ_ERR);

    va_list args;
    va_start(args, fmt);
    object->err = malloc(sizeof(char) * 512);
    vsnprintf(object->err, 511, fmt, args);
    va_end(args);

    return object;
}

int is_the_empty_list(obj_t *object) { return object == the_empty_list; }
int is_false(obj_t *object) { return object == false; }
int is_true(obj_t *object) { return !is_false(object); }

int is_pair(obj_t *object) { return object->type == OBJ_PAIR; }

int is_num(obj_t *object) { return object->type == OBJ_NUM; }

int is_symbol(obj_t *object) { return object->type == OBJ_SYM; }
int is_boolean(obj_t *object) { return object == true || object == false; }
int is_char(obj_t *object) { return object->type == OBJ_CHAR; }
int is_string(obj_t *object) { return object->type == OBJ_STR; }
int is_builtin(obj_t *object) { return object->type == OBJ_BUILTIN; }
int is_fun(obj_t *object) { return object->type == OBJ_FUN; }
int is_error(obj_t *object) { return object->type == OBJ_ERR; }

char *type_name(object_type type) {
    switch (type) {
    case OBJ_NUM:
        return "number";
    case OBJ_SYM:
        return "symbol";
    case OBJ_STR:
        return "string";
    case OBJ_PAIR:
        return "cons";
    case OBJ_BOOL:
        return "boolean";
    case OBJ_CHAR:
        return "character";
    case OBJ_FUN:
        return "function";
    case OBJ_NIL:
        return "nil";
    case OBJ_ERR:
        return "error";
    default:
        return "unknown";
    }
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
            printf("%li", object->num);
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
