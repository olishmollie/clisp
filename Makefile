clisp: clisp.c src/object.c
	cc clisp.c src/object.c -ledit -o bin/clisp