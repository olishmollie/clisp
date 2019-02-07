package jfig;

interface FigObject {

    default boolean isNil() {
        return this == Pair.nil;
    }

    default boolean isTrue() {
        return this == Bool.t;
    }

    default boolean isFalse() {
        return this == Bool.f;
    }

    default boolean isBool() {
        return this instanceof Bool;
    }

    default boolean isCharacter() {
        return this instanceof FigCharacter;
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

    default boolean isProcedure() {
        return this instanceof Procedure;
    }
}
