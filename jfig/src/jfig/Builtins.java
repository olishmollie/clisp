package jfig;


class Builtins {
    static private final Evaluator evaluator = new Evaluator();

    static final Procedure add = new Procedure() {
        @Override
        public FigObject call(Pair arguments, Environment environment) {
            Number result = new Number(0.0);

            Pair cur = (Pair) arguments;
            while (!cur.isNil()) {
                FigObject x = evaluator.evaluate(cur.car, environment);
                if (!x.isNumber()) {
                    throw new Error("arguments to add must be numbers.");
                }
                result = new Number(result.getValue() + ((Number) x).getValue());
                cur = (Pair) cur.cdr;
            }

            return result;
        }
    };

    static final Procedure subtract = new Procedure() {
        @Override
        public FigObject call(Pair arguments, Environment environment) {
            if (arguments.isNil()) {
                throw new Error("cannot call minus with no arguments.");
            }

            if (!arguments.car.isNumber()) {
                throw new Error("arguments to minus must be numbers.");
            }

            Number result = (Number) arguments.car;

            // unary minus
            if (arguments.cdr.isNil()) {
                return new Number(-1 * result.getValue());
            }

            Pair cur = (Pair) arguments.cdr;
            while (!cur.isNil()) {
                FigObject x = evaluator.evaluate(cur.car, environment);
                if (!x.isNumber()) {
                    throw new Error("arguments to minus be numbers.");
                }
                result = new Number(result.getValue() - ((Number) x).getValue());
                cur = (Pair) cur.cdr;
            }

            return result;
        }
    };

    static final Procedure multiply = new Procedure() {
        @Override
        public FigObject call(Pair arguments, Environment environment) {
            if (arguments.isNil()) {
                throw new Error("cannot call multiply with no arguments.");
            }

            if (!arguments.car.isNumber()) {
                throw new Error("arguments to multiply must be numbers.");
            }

            Number result = (Number) arguments.car;

            Pair cur = (Pair) arguments.cdr;
            while (!cur.isNil()) {
                FigObject x = evaluator.evaluate(cur.car, environment);
                if (!x.isNumber()) {
                    throw new Error("arguments to multiply be numbers.");
                }
                result = new Number(result.getValue() * ((Number) x).getValue());
                cur = (Pair) cur.cdr;
            }

            return result;
        }
    };

    static final Procedure divide = new Procedure() {
        @Override
        public FigObject call(Pair arguments, Environment environment) {
            if (arguments.isNil()) {
                throw new Error("cannot call divide with no arguments.");
            }

            if (!arguments.car.isNumber()) {
                throw new Error("arguments to divide must be numbers.");
            }

            Number result = (Number) arguments.car;

            Pair cur = (Pair) arguments.cdr;
            while (!cur.isNil()) {
                FigObject x = evaluator.evaluate(cur.car, environment);
                if (!x.isNumber()) {
                    throw new Error("arguments to multiply be numbers.");
                }
                if (((Number) x).getValue() == 0) {
                    throw new Error("division by zero.");
                }
                result = new Number(result.getValue() / ((Number) x).getValue());
                cur = (Pair) cur.cdr;
            }

            return result;
        }
    };

    static final Procedure exit = new Procedure() {
        @Override
        public FigObject call(Pair arguments, Environment environment) {
            int exitValue = 0;
            if (!arguments.isNil()) {
                FigObject arg = arguments.car;
                if (!arg.isNumber()) {
                    throw new Error("invalid argument to exit.");
                }
                exitValue = (int) ((Number) arg).getValue();
            }

            System.exit(exitValue);

            return null;
        }
    };

    static final Procedure isNumber = new Procedure() {
        @Override
        public FigObject call(Pair arguments, Environment environment) {
            if (arguments.isNil() || !arguments.cdr.isNil()) {
                throw new Error("incorrect number of arguments to isNumber");
            }

            FigObject object = evaluator.evaluate(arguments.car, environment);

            return object.isNumber() ? Bool.t : Bool.f;
        }
    };

    static final Procedure cons = new Procedure() {
        @Override
        public FigObject call(Pair arguments, Environment environment) {
            Pair result = new Pair();

            if (arguments.isNil() || arguments.cadr().isNil() || !arguments.cddr().isNil()) {
                throw new Error("incorrect number of arguments to cons.");
            }

            result.car = evaluator.evaluate(arguments.car, environment);
            result.cdr = evaluator.evaluate(arguments.cadr(), environment);

            return result;
        }
    };

    static final Procedure car = new Procedure() {
        @Override
        public FigObject call(Pair arguments, Environment environment) {
            if (arguments.isNil() || !arguments.cdr.isNil()) {
                throw new Error("incorrect number of arguments to car.");
            }

            FigObject pair = evaluator.evaluate(arguments.car, environment);

            if (!arguments.car.isPair()) {
                throw new Error("argument to car must be a pair.");
            }

            return ((Pair) pair).car;
        }
    };

    static final Procedure cdr = new Procedure() {
        @Override
        public FigObject call(Pair arguments, Environment environment) {
            if (arguments.isNil() || !arguments.cdr.isNil()) {
                throw new Error("incorrect number of arguments to cdr.");
            }

            FigObject pair = evaluator.evaluate(arguments.car, environment);

            if (!arguments.car.isPair()) {
                throw new Error("argument to cdr must be a pair.");
            }

            return ((Pair) pair).cdr;
        }
    };
}
