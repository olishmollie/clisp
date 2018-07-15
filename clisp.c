#include <ctype.h>
#include <editline/readline.h>
#include <stdio.h>
#include <stdlib.h>

#define NUM 256
#define SYM 257
#define END 258
#define NONE -1

int tokenval, pos, eof = -1;

int nextchar(char *input) {
  int n = input[++pos];
  return n ? n : eof;
}

int curchar(char *input) { return input[pos]; }

void skipspaces(char *input) {
  while (isspace(curchar(input))) {
    nextchar(input);
  }
}

int lexdigit(char *input) {
  int num = 0;
  while (isdigit(curchar(input))) {
    num = num * 10 + (curchar(input) - '0');
    int n = nextchar(input);
  }
  return num;
}

int lexan(char *input) {
  pos = 0;
  skipspaces(input);
  int cur = curchar(input);
  if (cur < 0)
    return END;
  else if (isdigit(cur)) {
    tokenval = lexdigit(input);
    return NUM;
  } else {
    tokenval = NONE;
    return cur;
  }
}

int main(void) {
  printf("Welcome to clisp! Use ctrl+c to exit.\n");
  while (1) {
    char *input = readline("clisp> ");
    add_history(input);
    int tok = lexan(input);
    printf("tokenval = %d\n", tokenval);
    free(input);
  }

  return 0;
}