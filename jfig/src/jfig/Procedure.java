package jfig;

class Procedure implements FigObject, FigCallable {
    private String name;
    private final Pair parameters;
    private final FigObject body;

    // Used for builtins
    Procedure() {
        this.name = null;
        this.parameters = null;
        this.body = null;
    }

    Procedure(String name, Pair parameters, FigObject body) {
        this.name = name;
        this.parameters = parameters;
        this.body = body;
    }

    void setName(String name) {
        this.name = name;
    }

    @Override
    public FigObject call(Pair arguments, Environment enclosing) {
        Environment environment = new Environment(enclosing);
        Evaluator evaluator = new Evaluator();

        Pair currentArgument = arguments;
        Pair currentParameter = parameters;
        while (!currentArgument.isNil() && !currentParameter.isNil()) {
            FigObject parameter = currentParameter.car;
            if (!parameter.isSymbol()) {
                throw new Error("invalid parameter");
            }
            FigObject argument = evaluator.evaluate(currentArgument.car, environment);
            environment.define(((Symbol) parameter).getValue(), argument);
            currentArgument = (Pair) currentArgument.cdr;
            currentParameter = (Pair) currentParameter.cdr;
        }

        if (!currentArgument.isNil() || !currentParameter.isNil()) {
            throw new Error("incorrect number of arguments.");
        }

        FigObject result = evaluator.evaluate(body, environment);

        return result;
    }

    @Override
    public String toString() {
        if (name == null) {
            return "<#procedure>";
        }
        return "<#procedure '" + name + "'>";
    }

}
