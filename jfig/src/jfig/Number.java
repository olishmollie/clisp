package jfig;

class Number implements FigObject {
    private final double value;

    Number(double value) {
        this.value = value;
    }

    double getValue() {
        return this.value;
    }

    @Override
    public String toString() {
        String result = Double.toString(value);
        if (result.endsWith(".0")) {
            result = result.substring(0, result.length() - 2);
        }

        return result;
    }
}
