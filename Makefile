clisp: clisp.c src/token.c src/table.c src/ast.c src/object.c src/list.c
	cc clisp.c src/token.c src/table.c src/ast.c src/object.c src/list.c -ledit -o bin/clisp