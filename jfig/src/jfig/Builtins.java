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

            FigObject result = evaluator.evaluate(arguments.car, environment);

            if (!result.isNumber()) {
                throw new Error("arguments to minus must be numbers.");
            }

            // unary minus
            if (arguments.cdr.isNil()) {
                return new Number(-1 * ((Number) result).getValue());
            }

            Pair cur = (Pair) arguments.cdr;
            while (!cur.isNil()) {
                FigObject x = evaluator.evaluate(cur.car, environment);

                if (!x.isNumber()) {
                    throw new Error("arguments to minus be numbers.");
                }

                result = new Number(((Number) result).getValue() - ((Number) x).getValue());
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

            FigObject result = evaluator.evaluate(arguments.car, environment);

            if (!result.isNumber()) {
                throw new Error("arguments to multiply must be numbers.");
            }

            Pair cur = (Pair) arguments.cdr;
            while (!cur.isNil()) {
                FigObject x = evaluator.evaluate(cur.car, environment);

                if (!x.isNumber()) {
                    throw new Error("arguments to multiply be numbers.");
                }

                result = new Number(((Number) result).getValue() * ((Number) x).getValue());
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

            FigObject result = evaluator.evaluate(arguments.car, environment);

            if (!result.isNumber()) {
                throw new Error("arguments to divide must be numbers.");
            }

            Pair cur = (Pair) arguments.cdr;
            while (!cur.isNil()) {
                FigObject x = evaluator.evaluate(cur.car, environment);

                if (!x.isNumber()) {
                    throw new Error("arguments to divide be numbers.");
                }

                if (((Number) x).getValue() == 0) {
                    throw new Error("division by zero.");
                }

                result = new Number(((Number) result).getValue() / ((Number) x).getValue());
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

    static final Procedure isBool = new Procedure() {
        @Override
        public FigObject call(Pair arguments, Environment environment) {
            if (arguments.isNil() || !arguments.cdr.isNil()) {
                throw new Error("incorrect number of arguments to isSymbol");
            }

            FigObject object = evaluator.evaluate(arguments.car, environment);

            return object.isBool() ? Bool.t : Bool.f;
        }
    };

    static final Procedure isSymbol = new Procedure() {
        @Override
        public FigObject call(Pair arguments, Environment environment) {
            if (arguments.isNil() || !arguments.cdr.isNil()) {
                throw new Error("incorrect number of arguments to isSymbol");
            }

            FigObject object = evaluator.evaluate(arguments.car, environment);

            return object.isSymbol() ? Bool.t : Bool.f;
        }
    };

    static final Procedure eq = new Procedure() {
        @Override
        public FigObject call(Pair arguments, Environment environment) {
            if (arguments.isNil() || arguments.cadr().isNil() || !arguments.cddr().isNil()) {
                throw new Error("incorrect number of arguments to eq.");
            }

            FigObject a = evaluator.evaluate(arguments.car, environment);
            FigObject b = evaluator.evaluate(arguments.cadr(), environment);

            if (a.isNumber() && b.isNumber()) {
                return ((Number) a).getValue() == ((Number) b).getValue() ? Bool.t : Bool.f;
            } else if (a.isSymbol() && b.isSymbol()) {
                return ((Symbol) a).getValue().equals(((Symbol) b).getValue()) ? Bool.t : Bool.f;
            }

            return a == b ? Bool.t : Bool.f;
        }
    };

    static final Procedure gt = new Procedure() {
        @Override
        public FigObject call(Pair arguments, Environment environment) {
            if (arguments.isNil() || arguments.cadr().isNil() || !arguments.cddr().isNil()) {
                throw new Error("incorrect number of arguments for >.");
            }

            FigObject a = evaluator.evaluate(arguments.car, environment);
            FigObject b = evaluator.evaluate(arguments.cadr(), environment);

            if (a.isNumber() && b.isNumber()) {
                return ((Number) a).getValue() > ((Number) b).getValue() ? Bool.t : Bool.f;
            }

            throw new Error("incorrect arguments passed to >.");
        }
    };

    static final Procedure gte = new Procedure() {
        @Override
        public FigObject call(Pair arguments, Environment environment) {
            if (arguments.isNil() || arguments.cadr().isNil() || !arguments.cddr().isNil()) {
                throw new Error("incorrect number of arguments for >.");
            }

            FigObject a = evaluator.evaluate(arguments.car, environment);
            FigObject b = evaluator.evaluate(arguments.cadr(), environment);

            if (a.isNumber() && b.isNumber()) {
                return ((Number) a).getValue() >= ((Number) b).getValue() ? Bool.t : Bool.f;
            }

            throw new Error("incorrect arguments passed to >.");
        }
    };

    static final Procedure lt = new Procedure() {
        @Override
        public FigObject call(Pair arguments, Environment environment) {
            if (arguments.isNil() || arguments.cadr().isNil() || !arguments.cddr().isNil()) {
                throw new Error("incorrect number of arguments for <.");
            }

            FigObject a = evaluator.evaluate(arguments.car, environment);
            FigObject b = evaluator.evaluate(arguments.cadr(), environment);

            if (a.isNumber() && b.isNumber()) {
                return ((Number) a).getValue() < ((Number) b).getValue() ? Bool.t : Bool.f;
            }

            throw new Error("incorrect arguments passed to >.");
        }
    };

    static final Procedure lte = new Procedure() {
        @Override
        public FigObject call(Pair arguments, Environment environment) {
            if (arguments.isNil() || arguments.cadr().isNil() || !arguments.cddr().isNil()) {
                throw new Error("incorrect number of arguments for <.");
            }

            FigObject a = evaluator.evaluate(arguments.car, environment);
            FigObject b = evaluator.evaluate(arguments.cadr(), environment);

            if (a.isNumber() && b.isNumber()) {
                return ((Number) a).getValue() <= ((Number) b).getValue() ? Bool.t : Bool.f;
            }

            throw new Error("incorrect arguments passed to >.");
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

    static final Procedure print = new Procedure() {
        @Override
        public FigObject call(Pair arguments, Environment environment) {
            if (arguments.isNil() || !arguments.cdr.isNil()) {
                throw new Error("incorrect number of arguments to print.");
            }

            System.out.print(evaluator.evaluate(arguments.car, environment));
            return null;
        }
    };

    static final Procedure println = new Procedure() {
        @Override
        public FigObject call(Pair arguments, Environment environment) {
            print.call(arguments, environment);
            System.out.println();
            return null;
        }
    };
}
