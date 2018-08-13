# SO Question

I am implementing a scheme in c, for learning and fun, and I'm stuck on how to properly manage lexical scope.

When a lambda object is created, it inherits its environment from the current context.

```c
obj *eval_lambda(env *e, obj *args) {

    // check param list for non-symbols
    obj *params = obj_popcar(&args);

    // lambda inherits env in which it is created
    obj *res = mk_lambda(params, args);
    res->fun->e = e;

    return res;
```

When the lambda is invoked, an empty environment is created that holds a reference to the lambda's environment.

```c
obj *invoke(env *e, obj *lambda, obj *args) {

    // check number of arguments and params match
    NARGCHECK(args, f->fun->name ? f->fun->name : "lambda",
              f->fun->params->nargs);


     // create new env for binding and execution,
     // extending lambda's existing environment

    env *local_env = env_new();
    local_env->parent = f->fun->e;

    // bind args to params
    while (f->fun->params->nargs > 0) {
        obj *param = obj_popcar(&f->fun->params);
        obj *arg = obj_popcar(&args);
        env_insert(local_env, param, arg);
        obj_delete(param);
        obj_delete(arg);
    }
    obj_delete(args);

    // evaluate each expression in lambda body but one
    while (f->fun->body->nargs > 1) {
        obj *expr = obj_popcar(&f->fun->body);
        obj *res = eval(local_env, expr);
        obj_delete(res);
    }

    // last expression evaluated is return value
    obj *expr = obj_popcar(&f->fun->body);
    obj *res = eval(local_env, expr);

    // causes segfault in certain situations
    env_delete(local_env);

    return res;
}
```

The problem happens with nested lambdas, for instance this redefinition of car and cons taken from SICP:

```scheme
(define (cons x y)
    (lambda (m) (m x y)))

(define (car z)
    (z (lambda (p q) p)))

(car (cons 1 2)) ;; causes segfault
```

I'm not entirely sure why this is happening, but I think it has to do with car and cons sharing a reference to the same environment. Not freeing the local environment avoids the issue, but causes a memory leak.

Does anyone have any insight in how to approach this? I'd be happy to post more code if necessary. Thanks.
