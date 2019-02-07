package jfig;

class FigReader {
    private final String source;

    private int start = 0;
    private int current = 0;

    FigReader(String source) {
        this.source = source;
    }

    FigObject read() {
        if (this.source.length() == 0) {
            return null;
        }

        skipSpaces();

        char c = advance();

        if (c == ')') {
            throw new Error("unexpected ')'");
        } else if (c == '.') {
            throw new Error("unexpected '.'");
        } else if (c == '#') {
            return constant();
        } else if (c == '\'') {
            return quote();
        } else if (c == '(') {
            reset(); // eat the '('
            return list();
        } else if (isDigit(c)) {
            return number();
        }

        return symbol();
    }

    private FigObject constant() {
        if (peek() == '\\') {
            return character();
        }

        if (peek() == 't') {
            consume();
            return Bool.t;
        } else if (peek() == 'f') {
            consume();
            return Bool.f;
        }

        throw new Error("invalid constant.");
    }

    private FigObject character() {
        advance(); // eat the \

        switch (peek()) {
            case 'n':
                throw new Error("invalid character.");
            case 't':
                throw new Error("invalid character.");
            default:
                char c = advance();
                if (isDelimeter(peek())) {
                    reset();
                    return new FigCharacter(c);
                }
                throw new Error("invalid character " + Character.toString(c) + ".");
        }
    }

    private FigObject quote() {
        reset();
        Pair quote = new Pair();
        quote.car = Symbol.quote;
        quote.cdr = new Pair(read(), Pair.nil);
        return quote;
    }

    private FigObject list() {
        // Empty list
        if (peek() == ')') {
            consume();
            return Pair.nil;
        }

        if (isAtEnd()) {
            throw new Error("expected ')', got eof.");
        }

        FigObject car = read();

        skipSpaces();

        // Improper list
        if (peek() == '.') {
            consume(); // eat the '.'
            FigObject cdr = read();

            expect(')');

            return new Pair(car, cdr);
        }

        FigObject cdr = list();

        return new Pair(car, cdr);
    }

    private FigObject symbol() {
        // negative numbers
        if (previous() == '-' && isDigit(peek())) {
            return number();
        }

        while (isSymbolChar(peek())) {
            advance();
        }

        String lexeme = reset();
        if (lexeme.equals("quote")) {
            return Symbol.quote;
        } else if (lexeme.equals("define")) {
            return Symbol.define;
        } else if (lexeme.equals("begin")) {
            return Symbol.begin;
        } else if (lexeme.equals("lambda")) {
            return Symbol.lambda;
        } else if (lexeme.equals("if")) {
            return Symbol.iff;
        }

        return new Symbol(lexeme);
    }

    private FigObject number() {
        while (isDigit(peek())) {
            advance();
        }

        if (peek() == '.') {
            advance();
            while (isDigit(peek())) {
                advance();
            }
        }

        return new Number(Double.parseDouble(reset()));
    }

    private char advance() {
        return source.charAt(current++);
    }

    private void expect(char c) {
        skipSpaces();

        if (c == peek()) {
            advance();
            return;
        }

        String actual = peek() == '\0' ? "eof" : Character.toString(peek());
        throw new Error("expected " + c + ", got " + actual + ".");
    }

    private void skipSpaces() {
        while (isSpace(peek())) {
            consume();
        }
    }

    private void consume() {
        advance();
        start = current;
    }

    // TODO: better name
    private String reset() {
        String result = source.substring(start, current);
        start = current;
        return result;
    }

    private char peek() {
        if (isAtEnd()) {
            return '\0';
        }

        return source.charAt(current);
    }

    private char previous() {
        return source.charAt(current - 1);
    }

    boolean isAtEnd() {
        return current >= source.length();
    }

    private boolean isDelimeter(char c) {
        switch (c) {
            case ' ':
            case '\n':
            case '\t':
            case '\0':
                return true;
            default:
                return false;
        }
    }

    private boolean isSymbolChar(char c) {
        String validSymbols = "+*-/!@$%^&-_=<>?";
        for (int i = 0; i < validSymbols.length(); i++) {
            if (c == validSymbols.charAt(i)) {
                return true;
            }
        }
        return isDigit(c) || isAlphaNumeric(c);
    }

    private boolean isDigit(char c) {
        return c >= '0' && c <= '9';
    }

    private boolean isAlphaNumeric(char c) {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || isDigit(c);
    }

    private boolean isSpace(char c) {
        return c == ' ' || c == '\t' || c == '\n';
    }

}
