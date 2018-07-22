clisp: clisp.c src/object.c src/env.c src/builtins.c
	cc clisp.c src/object.c src/env.c src/builtins.c -ledit -o bin/clisp