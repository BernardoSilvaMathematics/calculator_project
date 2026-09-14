# C++ Scientific Calculator

A modular calculator featuring a **tokenizer**, **recursive-descent parser**, **AST**, **evaluator**, and an interactive **graphing module**.
Supports arithmetic, functions, unary operators, exponentiation, modulo, factorial, scientific notation, a session-persistent `ANS` variable, and multi-curve function plotting with pan/zoom.

## Current Features

- Recursive-descent parser with correct operator precedence and associativity
- Abstract Syntax Tree with `Number`, `Identifier`, `Unary`, `Factorial`, `Binary`, and `FunctionCall` nodes
- Evaluator with overflow safeguards (finite-result checks on every operator, function, and factorial) and robust error handling
- Scientific notation support (`1e10`, `1.5e-3`, `5e+3`) with descriptive tokenizer errors on malformed input
- Interactive graphing window (SFML) with adaptive curve sampling, multi-curve support, pan/zoom, and PNG export

### Supported Operators

```
+  -  *  /  ^  %  !
```

Standard precedence applies (`^` right-associative, `!` postfix and binds tighter than unary `-`), with full support for parentheses and nested unary chains (`--5`, `+-5`, etc.).

### Supported Functions

```
SIN, COS, TAN, ASIN, ACOS, ATAN
EXP, SQRT
LOG(x) and LOG(x, base)
MAX, MIN, ABS
```

- Persistent `ANS` variable for session continuity — references the last evaluated result and can be used in any subsequent expression
- Clear, descriptive errors from the tokenizer, parser, and evaluator (e.g. domain errors, division/modulo by zero, malformed numbers with line/column info)


## Graphing

Type an expression using `x` as the independent variable, and plot it in an interactive window.

```
Enter expression: graphadd sin(x)
Added curve: sin(x)
Enter expression: graphadd cos(x) xmin=-20 xmax=20 res=2000
Added curve: cos(x)
Enter expression: graphshow
```

- `graphadd <expr> [xmin=] [xmax=] [res=]` — adds a curve to the pending plot. `xmin`/`xmax` default to `-10`/`10`, `res` (sample resolution) defaults to `1000`. Multiple curves can be added before showing the graph, each with its own domain and resolution.
- `graphshow` — opens the graph window and clears the pending curve list once the window is closed.

Sampling uses an adaptive step size that increases density where the curve is steep or curved, and coarsens where it's flat, so shapes stay smooth without wasting points on straight sections.

**In-window controls:**

| Action | Control |
|---|---|
| Zoom | Mouse scroll wheel, or the `+`/`-` buttons |
| Pan | Middle-click and drag |
| Reset view | `R` key |
| Save as PNG | `Save` button (saved to `graphs/`) |
| Clear curves | `Clear` button |
| Close window | `Esc` or the window close button |

A malformed expression passed to `graphadd`/`graphshow` prints an error and returns to the prompt rather than affecting the rest of the session.

## Documentation

This repository includes detailed Markdown documentation:

- **Architecture** — high-level design and data flow
- **Development Log** — implementation notes, tokenizer rules, parser grammar, AST structure, evaluator behavior
- **Project Overview** — goals of the project

These files contain the full technical explanation of the system.

## Dependencies

- A C++17-compatible compiler
- [SFML](https://www.sfml-dev.org/) (`libsfml-dev` on Debian/Ubuntu, or the equivalent for your platform) for the graphing window
- A TTF font at `fonts/DejaVuSans.ttf` (relative to the working directory, or `../fonts/DejaVuSans.ttf`) for axis labels and the legend. If missing, the graph still renders, just without text labels.

## How to Build & Run

```bash
mkdir build && cd build
cmake ..
make
./calculator
```

Run with `--ast` to print the parsed AST structure alongside each result:

```bash
./calculator --ast
```

## Running Tests

The project includes a test suite covering arithmetic, precedence, unary/factorial interactions, functions, domain errors, division/modulo by zero, and `ANS` memory persistence.



## Known Limitations / Roadmap

- Real-number arithmetic only, no complex numbers or big integers
- No variable assignment beyond the built-in `ANS` (planned: user-defined variables)
- Graphing currently supports single-variable functions of `x` only (planned: parametric/polar curves)
- No GUI beyond the graph window, the calculator itself is still command-line driven (planned: custom GUI)

