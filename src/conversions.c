#include "conversions.h"
#include "gmp.h"

/* int to double */
obj *itod(obj *i) {
    mpf_t dbl;
    mpf_init(dbl);
    mpf_set_z(dbl, i->num->integ);
    obj_delete(i);
    return obj_dbl(dbl);
}

/* int to rat */
obj *itor(obj *i) {
    mpq_t rat;
    mpq_init(rat);
    mpq_set_z(rat, i->num->integ);
    obj_delete(i);
    return obj_rat(rat);
}

/* rat to int */
obj *rtoi(obj *r) {
    mpz_t integ;
    mpz_init(integ);
    mpz_set_q(integ, r->num->rat);
    obj_delete(r);
    return obj_int(integ);
}

/* rat to double */
obj *rtod(obj *r) {
    mpf_t dbl;
    mpf_init(dbl);
    mpf_set_q(dbl, r->num->rat);
    obj_delete(r);
    return obj_dbl(dbl);
}

/* double to int */
obj *dtoi(obj *d) {
    mpz_t integ;
    mpz_init(integ);
    mpz_set_f(integ, d->num->dbl);
    obj_delete(d);
    return obj_int(integ);
}

/* double to rat */
obj *dtor(obj *d) {
    mpq_t rat;
    mpq_init(rat);
    mpq_set_f(rat, d->num->dbl);
    obj_delete(d);
    return obj_rat(rat);
}

obj *conv_int(obj *i, num_type type) {
    switch (type) {
    case NUM_INT:
        return i;
    case NUM_RAT:
        return itor(i);
    case NUM_DBL:
        return itod(i);
    default:
        obj_delete(i);
        return obj_err("unable to convert int to unknown num type");
    }
}

obj *conv_rat(obj *r, num_type type) {
    switch (type) {
    case NUM_RAT:
        return r;
    case NUM_INT:
        return rtoi(r);
    case NUM_DBL:
        return rtod(r);
    default:
        obj_delete(r);
        return obj_err("unable to convert rat to unknown num type");
    }
}

obj *conv_dbl(obj *d, num_type type) {
    switch (type) {
    case NUM_DBL:
        return d;
    case NUM_INT:
        return dtoi(d);
    case NUM_RAT:
        return dtor(d);
    default:
        obj_delete(d);
        return obj_err("unable to convert double to unknown num type");
    }
}

/* int op int -> int
 * int op rat -> rat
 * int op float -> float
 */

void conv(obj **x, obj **y) {
    if ((*x)->num->type == (*y)->num->type)
        return;
    if ((*x)->num->type == NUM_INT) {
        *x = conv_int(*x, (*y)->num->type);
    } else if ((*x)->num->type == NUM_RAT) {
        if ((*y)->num->type == NUM_INT) {
            *y = conv_int(*y, NUM_RAT);
        } else if ((*y)->num->type == NUM_DBL) {
            *x = conv_rat(*x, NUM_DBL);
        }
    } else {
        if ((*y)->num->type == NUM_INT) {
            *y = conv_int(*y, NUM_DBL);
        } else {
            *y = conv_rat(*y, NUM_DBL);
        }
    }
}
