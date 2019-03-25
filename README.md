# WarpXParser

This code evaluates mathematical expressions given in the form of
string.  It supports basic operators, `+`, `-`, `*`, `/`, and `**`,
and basic math functions, `sqrt`, `exp`, `log`, `log10`, `sin`, `cos`,
`tan`, `asin`, `acos`, `atan`, `sinh`, `cosh`, `tanh`, and `abs`.  The
minimum and maximum of two numbers can be computed with `min` and
`max`, respectively.  It also supports Heaviside step function,
`heaviside(x1,x2) = 0, x2, 1`, for `x1 < 0`, `x1 = 0` and `x1 > 0`,
respectively.

The source files (`.h` `.H`, `.c`, and `.cpp`) are in `src/`
directory.  The public interface is in `WarpXParser.H`.

Here is an example of basic usage.

```cpp
WarpXParser parser("exp(-(x**2+y**2)/3.0)*sin(omega*t)");
parser.setConstant("omega", 0.3); // omega cannot be changed again.
double x = 3.1, y = 3.2, t = 0.1;
parser.registerVariable("x",x);
parser.registerVariable("y",y);
parser.registerVariable("t",t);
double result = parser.eval();
t = 0.2;  // Variables can be changed.
result = parser.eval();
```

There is an alternative way of registering variables.

```cpp
WarpXParser parser("exp(-(x**2+y**2)/3.0)*sin(omega*t)");
parser.registerVariables({"x","y","t"});
parser.setConstant("omega", 0.3); // omega cannot be changed again.
double result = parser.eval(3.1,3.2,0.1);
result = parser.eval(3.1,3.2,0.2);
```

Here, all variables are registered with `registerVariables` and values
are passed to the `eval` function in exactly the same order.  Note
that these two ways of `register` and `eval` cannot be mixed.

OpenMP is supported. Below is an example of using OpenMP.

```cpp
WarpXParser parser("exp(-(x**2+y**2)/3.0)*sin(omega*t)");
parser.setConstant("omega", 0.3); // omega cannot be changed again.

std::vector<double> result(100);

#pragma omp parallel
{
    // Define thread private variables.
    // Note the lifetime of these variables must be beyon eval().
    double x = 3.1, y = 3.2, t;

    // Every thread must register.
    parser.registerVariable("x",x);
    parser.registerVariable("y",y);
    parser.registerVariable("t",t);
    
    const int n = result.size();
#pragma omp for
    for (int i = 0; i < n; ++i) {
        t = 0.1 * i;
        result[i] = parser.eval();
    }
}
```

Alternatively, we can do

```cpp
WarpXParser parser("exp(-(x**2+y**2)/3.0)*sin(omega*t)");
parser.registerVariables({"x","y","t"});
parser.setConstant("omega", 0.3); // omega cannot be changed again.

std::vector<double> result(100);
double x = 3.1, y = 3.2;

#pragma omp parallel for
for (int i = 0; i < 100; ++i) {
    t = 0.1 * i;
    result[i] = parser.eval(x,y,t);
}
```
