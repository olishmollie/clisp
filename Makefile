clisp: clisp.c src/token.c src/table.c src/ast.c src/object.c
	cc clisp.c src/token.c src/table.c src/ast.c src/object.c -ledit -o bin/clisp