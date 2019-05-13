#include "common.h"
#include "numbers.h"

static long gcd(long a, long b) {
    if (a == 0)
        return b;
    return gcd(b % a, a);
}

obj_t *reduce(VM *vm, obj_t *rational) {
    int divisor = gcd(rational->numer, rational->denom);
    if (divisor == 1) {
        return rational;
    }
    rational->numer /= divisor;
    rational->denom /= divisor;

    if (rational->denom < 0) {
        rational->numer *= -1;
        rational->denom *= -1;
    }

    return rational;
}

obj_t *num_add(VM *vm, obj_t *a, obj_t *b) {
    if (a->denom == b->denom) {
        // TODO: check for overflow
        return mk_num_from_long(vm, a->numer + b->numer, a->denom);
    }
    long denom = a->denom * b->denom;
    long numer = a->numer * b->denom + b->numer * a->denom;
    return mk_num_from_long(vm, numer, denom);
}

obj_t *num_sub(VM *vm, obj_t *a, obj_t *b) {
    if (a->denom == b->denom) {
        return mk_num_from_long(vm, a->numer - b->numer, a->denom);
    }
    long denom = a->denom * b->denom;
    long numer = a->numer * b->denom - b->numer * a->denom;
    return mk_num_from_long(vm, numer, denom);
}

obj_t *num_mul(VM *vm, obj_t *a, obj_t *b) {
    long denom = a->denom * b->denom;
    long numer = a->numer * b->numer;
    return mk_num_from_long(vm, numer, denom);
}

obj_t *num_div(VM *vm, obj_t *a, obj_t *b) {
    long denom = a->denom * b->numer;
    long numer = a->numer * b->denom;
    return mk_num_from_long(vm, numer, denom);
}

obj_t *num_mod(VM *vm, obj_t *a, obj_t *b) {
    long divisor = a->numer;
    long modulus = b->numer;
    return mk_num_from_long(vm, divisor % modulus, 1l);
}


obj_t *num_gt(VM *vm, obj_t *a, obj_t *b) {
    long denom = a->denom * b->denom;
    return (a->numer * denom > b->numer * denom) ? true : false;
}

obj_t *num_gte(VM *vm, obj_t *a, obj_t *b) {
    long denom = a->denom * b->denom;
    return (a->numer * denom >= b->numer * denom) ? true : false;
}

obj_t *num_lt(VM *vm, obj_t *a, obj_t *b) {
    long denom = a->denom * b->denom;
    return (a->numer * denom < b->numer * denom) ? true : false;
}

obj_t *num_lte(VM *vm, obj_t *a, obj_t *b) {
    long denom = a->denom * b->denom;
    return (a->numer * denom <= b->numer * denom) ? true : false;
}

obj_t *num_eq(VM *vm, obj_t *a, obj_t *b) {
    long denom = a->denom * b->denom;
    return a->numer * denom == b->numer * denom ? true : false;
}