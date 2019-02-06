package jfig;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.nio.charset.Charset;
import java.nio.file.Files;
import java.nio.file.Paths;

class Fig {
    static private Evaluator evaluator = new Evaluator();
    static private Environment globals = new Environment();

    public static void main(String[] args) throws IOException {
        initGlobals();

        if (args.length > 1) {
            System.out.println("Usage: fig [argfile]");
            System.exit(1);
        }

        if (args.length == 1) {
            runFile(args[0]);
        } else {
            runPrompt();
        }
    }

    private static void initGlobals() {
        globals.define("+", Builtins.add);
        globals.define("-", Builtins.subtract);
        globals.define("*", Builtins.multiply);
        globals.define("/", Builtins.divide);

        globals.define("exit", Builtins.exit);
        globals.define("isNumber?", Builtins.isNumber);
        globals.define("cons", Builtins.cons);
        globals.define("car", Builtins.car);
        globals.define("cdr", Builtins.cdr);
    }

    private static void runFile(String filename) throws IOException {
        byte[] encoded = Files.readAllBytes(Paths.get(filename));
        String data = new String(encoded, Charset.defaultCharset());
        run(data);
    }

    private static void runPrompt() throws IOException {
        InputStreamReader input = new InputStreamReader(System.in);
        BufferedReader reader = new BufferedReader(input);

        for (;;) {
            System.out.print("> ");
            run(reader.readLine());
        }
    }

    private static void run(String data) {
        FigReader reader = new FigReader(data);
        FigObject object = reader.read();
        if (object != null) {
            System.out.println(evaluator.evaluate(object, globals));
        }
    }
}
