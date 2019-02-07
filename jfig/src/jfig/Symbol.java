package jfig;

class Symbol implements FigObject {
    static final Symbol quote = new Symbol("quote");
    static final Symbol define = new Symbol("define");
    static final Symbol begin = new Symbol("begin");
    static final Symbol lambda = new Symbol("lambda");
    static final Symbol iff = new Symbol("if");

    private final String value;

    Symbol(String value) {
        this.value = value;
    }

    String getValue() {
        return this.value;
    }

    @Override
    public String toString() {
        return this.value;
    }
}
