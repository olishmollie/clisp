package jfig;

class FigCharacter implements FigObject {
    static final FigCharacter newline = new FigCharacter('\n');
    static final FigCharacter tab = new FigCharacter('\t');

    private final char value;

    FigCharacter(char value) {
        this.value = value;
    }

    char getValue() {
        return value;
    }

    @Override
    public String toString() {
        switch (value) {
            case '\n':
                return "#\\newline";
            case '\t':
                return "#\\tab";
            default:
                return "#\\" + Character.toString(value);
        }
    }
}
