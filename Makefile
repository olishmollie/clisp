clisp: clisp.c token.c table.c ast.c
	cc clisp.c token.c table.c ast.c -ledit -o clisp