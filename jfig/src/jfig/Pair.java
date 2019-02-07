package jfig;

class Pair implements FigObject {

    // the empty list
    static class Nil extends Pair {
        @Override
        public String toString() {
            return "()";
        }
    }

    static final Nil nil = new Nil();

    private static String listString(Pair list) {
        StringBuilder builder = new StringBuilder();

        builder.append(list.car.toString());

        if (list.cdr.isNil()) {
            return builder.toString();
        } else if (list.cdr.isPair()) {
            builder.append(" ");
            builder.append(Pair.listString((Pair) list.cdr));
        } else {
            builder.append(" . ");
            builder.append(list.cdr.toString());
        }

        return builder.toString();

    }

    FigObject car;
    FigObject cdr;

    Pair(FigObject car, FigObject cdr) {
        this.car = car;
        this.cdr = cdr;
    }

    Pair() {
        this.car = null;
        this.cdr = null;
    }

    Pair getValue() {
        return this;
    }

    void setCar(FigObject car) {
        this.car = car;
    }

    void setCdr(FigObject cdr) {
        this.cdr = cdr;
    }

    FigObject cadr() {
        return ((Pair) this.cdr).car;
    }

    FigObject cddr() {
        return ((Pair) this.cdr).cdr;
    }

    FigObject cdddr() {
        return ((Pair) this.cddr()).cdr;
    }

    FigObject caddr() {
        return ((Pair) this.cddr()).car;
    }

    FigObject cadddr() {
        return ((Pair) this.cdddr()).car;
    }

    @Override
    public String toString() {
        return "(" + Pair.listString(this) + ")";
    }
}

