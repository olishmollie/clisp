package jfig;

class Evaluator {
    FigObject evaluate(FigObject object, Environment environment) {
        if (object.isNil()) {
            throw new Error("invalid syntax.");
        }

        if (object.isNumber() || object.isBool() || object.isCharacter()) {
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
        } else if (object.car == Symbol.begin) {
            return evaluateBegin(object, environment);
        } else if (object.car == Symbol.lambda) {
            return evaluateLambda(object, environment);
        } else if (object.car == Symbol.iff) {
            return evaluateIf(object, environment);
        }

        FigObject procedure = evaluate(object.car, environment);
        if (!procedure.isProcedure()) {
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
            FigObject body = object.cddr();

            Pair lambdaObject = new Pair(Symbol.lambda, new Pair(parameters, body));
            FigObject lambda = evaluateLambda(lambdaObject, environment);

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

    private FigObject evaluateBegin(Pair object, Environment environment) {
        FigObject actions = object.cdr;

        if (!actions.isPair()) {
            throw new Error("invalid syntax begin.");
        }

        while (!((Pair) actions).cdr.isNil()) {
            if (!actions.isPair()) {
                throw new Error("invalid syntax begin.");
            }
            evaluate(((Pair) actions).car, environment);
            actions = ((Pair) actions).cdr;
        }

        return evaluate(((Pair) actions).car, environment);
    }

    private FigObject evaluateLambda(Pair object, Environment environment) {
        if (object.cdr.isNil()) {
            throw new Error("invalid lambda definition.");
        }

        FigObject parameters = object.cadr();
        if (!parameters.isPair()) {
            throw new Error("invalid lambda parameter list.");
        }

        FigObject body = object.cddr();
        if (body.isNil()) {
            throw new Error("invalid lambda body.");
        }

        // wrap body in begin block
        body = new Pair(Symbol.begin, body);

        return new Procedure(null, (Pair) parameters, body, environment);
    }

    private FigObject evaluateIf(Pair object, Environment environment) {
        if (object.cdr.isNil() || !object.cdr.isPair()) {
            throw new Error("bad syntax if.");
        }

        FigObject predicate = evaluate(object.cadr(), environment);
        if (!predicate.isFalse()) {
            return evaluate(object.caddr(), environment);
        } else {
            return evaluate(object.cadddr(), environment);
        }
    }

}
