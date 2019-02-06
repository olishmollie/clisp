package jfig;

class Bool implements FigObject {
    static final Bool t = new Bool(true);
    static final Bool f = new Bool(false);

    private final boolean value;

    Bool(boolean value) {
        this.value = value;
    }

    boolean getValue() {
        return this.value;
    }

    @Override
    public String toString() {
        return this.value ? "#t" : "#f";
    }
}
