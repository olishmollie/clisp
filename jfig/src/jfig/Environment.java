package jfig;

import java.util.HashMap;
import java.util.Map;

class Environment {
    Environment enclosing;
    private final Map<String, FigObject> map;

    Environment() {
        this.enclosing = null;
        this.map = new HashMap<>();
    }

    Environment(Environment enclosing) {
        this.enclosing = enclosing;
        this.map = new HashMap<>();
    }

    void define(String key, FigObject value) {
        // add name to procedure
        if (value instanceof Procedure) {
            ((Procedure) value).setName(key);
        }
        this.map.put(key, value);
    }

    FigObject get(String key) {
        if (this.map.get(key) != null) {
            return this.map.get(key);
        }

        if (this.enclosing != null) {
            return this.enclosing.get(key);
        }

        throw new Error("unbound variable " + "'" + key + "'.");
    }

}
