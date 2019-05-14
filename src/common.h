#ifndef COMMON_H
#define COMMON_H

#define VERSION "0.3"

#include "exception.h"
#include "object.h"
#include "table.h"
#include "vm.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

typedef struct VM VM;

VM *vm;

obj_t *universe;
table_t *symbol_table;

obj_t *the_empty_list;
obj_t *true;
obj_t *false;

obj_t *quote_sym;
obj_t *quasiquote_sym;
obj_t *unquote_sym;
obj_t *unquote_splicing_sym;

obj_t *define_sym;
obj_t *set_sym;
obj_t *if_sym;
obj_t *lambda_sym;
obj_t *begin_sym;

/* exception handling */
jmp_buf exc_env;
obj_t *exc;


#endif