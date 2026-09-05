Tokenizer
	enum class TokenType - contains all possible types of tokens
	struct Token - essentially will store the tokens
	class Tokenizer - evaluates the string
		> here the nextToken evaluates valid tokens and returns a Token element

			Naturally skips whitespaces, but not within numbers, like 5 6 will be read as two independent tokens
			Numbers such as .5 or 5. will have a 0 appended respectively, if it contains more than one dot there will be an error saying Invalid Number
			also allows for exponent numbers (e or E), here we don't allow dots after e, it can't end in e, also allow for a optinal sign after e. The tokenizer consumes characters until one violates these rules, then returns a `Number` token or throws a descriptive error.


AST (Abstract syntax tree)
	ASTNode — base class for all syntax tree nodes.
	   - Declared with a virtual destructor to allow safe deletion through base pointers.
	   - Enables polymorphic storage of different node types in std::unique_ptr<ASTNode>.

	## Derived node types
		- NumberNode — represents numeric literals (obtained through `std::stod`, converting strings to doubles)
		- IdentifierNode — represents variables or function names
		- UnaryNode — represents unary operators (`+x`, `-x`)  
		  → Applies only when the parser expects the *start* of an expression or subexpression
		  →  Divided into prefix(-/+) and postfix (such as the factorial)
		- BinaryNode — represents binary operators (`+`, `-`, `*`, `/`, `^`)  
		  → Applies only after a left‑hand side has already been parsed
		- FunctionCallNode — represents function calls (`sin(x)`, `log(3,10)`)  
		  → Each argument is stored as an AST subtree in a vector of `unique_ptr<ASTNode>`

Parser  
Builds the grammar of attribution for the AST
	
	- **Lowest precedence:** binary `+` and `-` handled by `parseExpression`.  
	  These operators are evaluated last.
	
	- **Medium precedence:** `*` and `/` handled by `parseTerm`.  
	  These bind tighter than addition and subtraction.
	
	- **High precedence:** `^` (exponentiation) handled by `parseFactor`.  
	  This operator is right‑associative.
	
	- **Very high precedence:** prefix unary `+x` and `-x` handled by `parseUnary`.
	  Unary operators apply before exponentiation
	
	- **Highest operator precedence:** postfix unary `x!` → handled in `parseFactor` _after_ exponentiation → binds tighter than prefix unary
	
	- **Atomic units:** numbers, identifiers, parentheses, function calls handled by `parsePrimary`.  
	
	
	In this expression grammar, only a small set of tokens can legally begin an expression. This follows directly from the rules:
	
	```
	unary → ('+' | '-') unary | primary  | postfix unary (!)
	primary → number | identifier | '(' expression ')'
	```
	
	### **Valid starters**
	
	- **number**
	- **identifier**
	- ** prefix unary** `+` **or** `-`
	- `(` - (parenthesized expression)
	    
	
	These are the only tokens accepted by `parseUnary()` and `parsePrimary()` as the beginning of an expression.
	
	### **Why nothing else is valid**
	
	Tokens such as `^`, `*`, `/`, `)`, `,` cannot start an expression because they only appear **after** a primary in higher‑level grammar rules (e.g., exponentiation, multiplication, function calls, closing parentheses). If any of these appear at the start, the parser eventually reaches `parsePrimary()`, which rejects them as unexpected.
	
	### **Why parsing continues after `a + b` without restarting**
	
	The parser never calls `parseExpression()` again after parsing `b`. Instead, control returns upward through the call stack:
	
	parsePrimary → parseFactorial → parseUnary → parseFactor → parseTerm → parseExpression
	
	`parseExpression()` is still inside its while-loop:
	
	    while (current.type == Plus || current.type == Minus)
	
	So after `parsePrimary()` returns the node for `b`, the parser continues from the same loop, checks the next token, and either:
	
	- continues parsing another term, or
	- exits the loop and returns the final AST.
	
	Calling `next()` only advances the token stream; it does not restart the parser or re-enter the grammar. The structure of the grammar ensures that parsing continues naturally from the correct place.

AST_printer
Builds the AST tree in readable language so that it can be read and precedence order analysed
	AST printing is a developer-only feature. It should be controlled by a runtime flag:
	
	    bool enablePrettyPrint = false;
	
	After parsing, if the flag is true, the AST is printed. This keeps the printer optional and separate from the parser and evaluator.
	
	dynamic_cast is used to safely check the real derived type of an ASTNode at runtime. It returns nullptr if the cast fails, making it ideal for AST visitors and printers. It is safer than static_cast because it verifies the actual object type.

	### ASTPrinter recursion mirrors parser recursion

	The AST printer uses the same recursive pattern as the parser. The parser builds the tree by recursively constructing nodes; the printer walks the tree by recursively visiting nodes. Each node handles itself and then delegates to its children. This shared structure is why both systems feel similar and why the printer correctly prints the entire expression tree.


Evaluator
Walks the tree and evaluates each node
	IdentifierNode necessiatates a unordered_map composed of the constant values to consider
	Unary essentially stores only a sign (+ or -) and a child node. The evaluator recursively computes the child value, then applies the sign. Multiple unary operators (---+5) become nested UnaryNodes. Recursion naturally accumulates signs: each level multiplies the child by +1 or -1. The child is never compromised; it is always the next node in the chain.

	Added safeguards against overflow in each node evaluation
- **SIN(x)** — Sine
    
- **COS(x)** — Cosine
    
- **TAN(x)** — Tangent
    
- **ASIN(x)** — Arc sine (domain:−1,1)

- **ACOS(x)** — Arc cosine (domain:−1,1)
- **ATAN(x)** — Arc tangent
    
- **EXP(x)** — Exponential
    
- **SQRT(x)** — Square root (domain: x ≥ 0)
    
- **LOG(x)** — Natural logarithm (domain: x > 0)
    
- **LOG(x, base)** — Logarithm with base (domain: x > 0, base > 0, base ≠ 1)
    
- **MAX(a, b, ...)** — Maximum of arguments
    
- **MIN(a, b, ...)** — Minimum of arguments
    
- **ABS(x)**

Also allows to access the last answer as ANS
	Represents the **last evaluated result** within the current session. When encountered, the evaluator retrieves the stored value from its internal variable map (`variables["ANS"]`). If no previous computation exists, `ANS` defaults to `0`. This node ensures continuity between expressions — for example, `ANS * 3` multiplies the last result by three. `ANS` is session‑persistent but resets when the program restarts.


Main
	Handles the interactive session and orchestrates the calculator’s core components. Creates a single `Evaluator` instance that persists throughout the program’s lifetime, allowing `ANS` to retain its value between expressions. Each iteration reads an input line, tokenizes, parses, evaluates, and prints the result. The program only terminates when the user sends an empty line or EOF (`Ctrl+D` on Linux/macOS, `Ctrl+Z` on Windows). This design transforms the calculator into a REPL (Read–Eval–Print Loop), enabling continuous evaluation without restarting the executable.
