#include "object.h"

obj *obj_new(obj_t type) {
    obj *object = malloc(sizeof(obj));
    object->type = type;
    object->mark = 0;
    return object;
}

obj *cons(obj *car, obj *cdr) {
    obj *object = obj_new(OBJ_PAIR);
    object->car = car;
    object->cdr = cdr;
    return object;
}

obj *mk_num_from_str(char *numstr) {
    obj *object = obj_new(OBJ_NUM);
    object->num = strtol(numstr, NULL, 10);
    return object;
}

obj *mk_num_from_long(long num) {
    obj *object = obj_new(OBJ_NUM);
    object->num = num;
    return object;
}

char *num_to_string(obj *object) {
    char *buf = malloc(sizeof(char) * MAXSTRLEN);
    snprintf(buf, MAXSTRLEN - 1, "%li", object->num);
    return buf;
}

obj *mk_sym(char *name) {
    obj *object;
    obj *el;

    el = symbol_table;
    while (!is_the_empty_list(el)) {
        if (strcmp(car(el)->sym, name) == 0) {
            return car(el);
        }
        el = cdr(el);
    }

    object = obj_new(OBJ_SYM);
    object->sym = malloc(sizeof(char) * (strlen(name) + 1));
    strcpy(object->sym, name);
    symbol_table = cons(object, symbol_table);
    return object;
}

obj *mk_string(char *str) {
    obj *object = obj_new(OBJ_STR);
    object->str = malloc(sizeof(char) * (strlen(str) + 1));
    strcpy(object->str, str);
    return object;
}

obj *mk_builtin(char *name, builtin proc) {
    obj *object = obj_new(OBJ_BUILTIN);
    object->name = malloc(sizeof(char) * (strlen(name) + 1));
    strcpy(object->name, name);
    object->proc = proc;
    return object;
}

obj *mk_fun(obj *env, obj *params, obj *body) {
    obj *object = obj_new(OBJ_FUN);
    object->env = env;
    object->params = params;
    object->body = body;
    return object;
}

obj *mk_char(char c) {
    obj *object = obj_new(OBJ_CHAR);
    object->character = c;
    return object;
}

obj *mk_bool(int value) {
    obj *object = obj_new(OBJ_BOOL);
    object->boolean = value;
    return object;
}

obj *mk_nil(void) {
    obj *object = obj_new(OBJ_NIL);
    return object;
}

obj *mk_err(char *fmt, ...) {
    obj *object = obj_new(OBJ_ERR);

    va_list args;
    va_start(args, fmt);
    object->err = malloc(sizeof(char) * 512);
    vsnprintf(object->err, 511, fmt, args);
    va_end(args);

    return object;
}

int is_the_empty_list(obj *object) { return object == the_empty_list; }
int is_false(obj *object) { return object == false; }
int is_true(obj *object) { return !is_false(object); }

int is_pair(obj *object) { return object->type == OBJ_PAIR; }

int is_num(obj *object) { return object->type == OBJ_NUM; }

int is_symbol(obj *object) { return object->type == OBJ_SYM; }
int is_boolean(obj *object) { return object == true || object == false; }
int is_char(obj *object) { return object->type == OBJ_CHAR; }
int is_string(obj *object) { return object->type == OBJ_STR; }
int is_builtin(obj *object) { return object->type == OBJ_BUILTIN; }
int is_fun(obj *object) { return object->type == OBJ_FUN; }
int is_error(obj *object) { return object->type == OBJ_ERR; }

char *type_name(obj_t type) {
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

int length(obj *object) {
    int l = 0;
    while (!is_the_empty_list(object)) {
        l++;
        object = cdr(object);
    }
    return l;
}

/* list fns ---------------------------------------------------------------- */

obj *car(obj *pair) { return pair->car; }
obj *cdr(obj *pair) { return pair->cdr; }
void set_car(obj *pair, obj *value) { pair->car = value; }
void set_cdr(obj *pair, obj *value) { pair->cdr = value; }

/* environments ------------------------------------------------------------ */

// initial env is ( '()  )
obj *mk_frame(obj *vars, obj *vals) { return cons(vars, vals); }

obj *frame_vars(obj *frame) { return car(frame); }

obj *frame_vals(obj *frame) { return cdr(frame); }

void add_binding_to_frame(obj *var, obj *val, obj *frame) {
    set_car(frame, cons(var, car(frame)));
    set_cdr(frame, cons(val, cdr(frame)));
}

obj *env_extend(obj *env, obj *vars, obj *vals) {
    return cons(mk_frame(vars, vals), env);
}

obj *env_lookup(obj *env, obj *var) {
    obj *frame, *vars, *vals;
    while (!is_the_empty_list(env)) {
        frame = car(env);
        vars = frame_vars(frame);
        vals = frame_vals(frame);
        while (!is_the_empty_list(vars)) {
            if (car(vars) == var)
                return car(vals);
            vars = cdr(vars);
            vals = cdr(vals);
        }
        env = cdr(env);
    }
    return mk_err("unbound variable '%s'", var->sym);
}

obj *env_set(obj *env, obj *var, obj *val) {
    obj *frame, *vars, *vals;
    while (!is_the_empty_list(env)) {
        frame = car(env);
        vars = frame_vars(frame);
        vals = frame_vals(frame);
        while (!is_the_empty_list(vars)) {
            if (car(vars) == var) {
                set_car(vals, val);
                return NULL;
            }
            vars = cdr(vars);
            vals = cdr(vals);
        }
        env = cdr(env);
    }
    return mk_err("unbound variable '%s'", var->sym);
}

obj *env_define(obj *env, obj *var, obj *val) {
    obj *frame, *vars, *vals;

    frame = car(env);
    vars = frame_vars(frame);
    vals = frame_vals(frame);

    while (!is_the_empty_list(vars)) {
        if (car(vars) == var) {
            set_car(vals, val);
            return NULL;
        }
        vars = cdr(vars);
        vals = cdr(vals);
    }
    add_binding_to_frame(var, val, frame);
    return NULL;
}

obj *env_new(void) {
    obj *env;
    env = env_extend(the_empty_list, the_empty_list, the_empty_list);
    return env;
}

/* printing ---------------------------------------------------------------- */

void print(obj *object);

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

void print_cons(obj *object) {
    putchar('(');
    obj *p = object;
    while (1) {
        print(car(p));
        obj *cdr_obj = cdr(p);
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

void print(obj *object) {
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
            printf("Cannot print unknown obj type\n");
        }
    }
}

void println(obj *object) {
    print(object);
    putchar('\n');
}
