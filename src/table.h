#ifndef _TABLE_H
#define _TABLE_H

#define MAXCHAR 2048

char lexemes[MAXCHAR];

void table_init();
int table_insert(char *symbol);
int table_lookup(char *symbol);

#endif