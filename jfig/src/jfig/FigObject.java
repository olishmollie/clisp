package jfig;

interface FigObject {

    default boolean isNil() {
        return this == Pair.nil;
    }

    default boolean isNumber() {
        return this instanceof Number;
    }

    default boolean isSymbol() {
        return this instanceof Symbol;
    }

    default boolean isPair() {
        return this instanceof Pair;
    }

    default boolean isCallable() {
        return this instanceof FigCallable;
    }
}
