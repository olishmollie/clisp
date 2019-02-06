package jfig;

interface FigCallable extends FigObject {
    FigObject call(Pair arguments, Environment environment);
}
