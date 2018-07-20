clisp: clisp.c src/table.c src/object.c
	cc clisp.c src/table.c src/object.c -ledit -o bin/clisp