clisp: clisp.c token.c table.c ast.c object.c
	cc clisp.c token.c table.c ast.c object.c -ledit -o clisp