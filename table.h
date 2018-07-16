#ifndef _TABLE_H
#define _TABLE_H

#define MAXCHAR 2048

char lexemes[MAXCHAR];

void init();
int insert(char *symbol);
int lookup(char *symbol);

#endif