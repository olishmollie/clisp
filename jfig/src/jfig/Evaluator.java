package jfig;

class Evaluator {
    FigObject evaluate(FigObject object, Environment environment) {
        if (object.isNil()) {
            throw new Error("invalid syntax.");
        }

        if (object.isNumber()) {
            return object;
        } else if (object.isSymbol()) {
            return evaluateSymbol((Symbol) object, environment);
        } else if (object.isPair()) {
            return evaluatePair((Pair) object, environment);
        }

        return null;
    }

    private FigObject evaluateSymbol(Symbol object, Environment environment) {
        return environment.get(object.getValue());
    }

    private FigObject evaluatePair(Pair object, Environment environment) {
        if (object.car == Symbol.quote) {
            return evaluateQuote(object);
        } else if (object.car == Symbol.define) {
            return evaluateDefine(object, environment);
        } else if (object.car == Symbol.lambda) {
            return evaluateLambda(object);
        }

        FigObject procedure = evaluate(object.car, environment);
        if (!procedure.isCallable()) {
            throw new Error("invalid procedure.");
        }
        Pair arguments = (Pair) object.cdr;

        FigObject result = ((FigCallable) procedure).call(arguments, environment);

        return result;
    }

    private FigObject evaluateQuote(Pair object) {
        return object.cadr();
    }

    private FigObject evaluateDefine(Pair object, Environment environment) {
        FigObject name = object.cadr();

        if (name.isPair()) {
            FigObject target = ((Pair) name).car;
            if (!target.isSymbol()) {
                throw new Error("invalid definition target");
            }

            FigObject parameters = ((Pair) name).cdr;
            FigObject body = object.caddr();

            Pair lambdaObject =
                    new Pair(Symbol.lambda, new Pair(parameters, new Pair(body, Pair.nil)));
            FigObject lambda = evaluateLambda(lambdaObject);

            environment.define(((Symbol) target).getValue(), lambda);

            return null;
        }

        if (!(name.isSymbol())) {
            throw new Error("invalid definition target");
        }

        FigObject value = evaluate(object.caddr(), environment);

        environment.define(((Symbol) name).getValue(), value);

        return null;
    }

    private FigObject evaluateLambda(Pair object) {
        if (object.cdr.isNil()) {
            throw new Error("invalid lambda definition.");
        }

        FigObject parameters = object.cadr();
        if (!parameters.isPair()) {
            throw new Error("invalid lambda parameter list.");
        }

        FigObject body = object.caddr();
        if (body.isNil()) {
            throw new Error("invalid lambda body.");
        }

        return new Procedure(null, (Pair) parameters, body);
    }

}
