#include "tokenizer.hpp"
#include "parser.hpp"
#include "evaluator.hpp"
#include <cassert>
#include <cmath>
#include <iostream>
#include <stdexcept>

bool approx(double a, double b, double eps = 1e-9) {
    return std::abs(a - b) < eps;
}

double eval(const std::string& expr) {
    Tokenizer tz(expr);
    Parser parser(std::move(tz));
    auto ast = parser.parse();

    Evaluator evaluator;              // use evaluator class
    return evaluator.eval(ast.get()); // updates ANS internally
}

void test(const std::string& expr, double expected) {
    double result = eval(expr);
    if (!approx(result, expected)) {
        std::cerr << "FAILED: " << expr
                  << " expected " << expected
                  << " got " << result << "\n";
        std::exit(1);
    }
}

void test_error(const std::string& expr) {
    try {
        double r = eval(expr);
        std::cerr << "FAILED (expected error): " << expr
                  << " got " << r << "\n";
        std::exit(1);
    } catch (const std::exception&) {
        // Expected
    }
}

int main() {

    std::cout << "Running full test suite...\n";

    // ------------------------------------------------------------
    // BASIC ARITHMETIC
    // ------------------------------------------------------------
    test("2+3*4", 14);
    test("(2+3)*4", 20);
    test("10/2", 5);
    test("2^3", 8);
    test("2^3^2", 512);

    // ------------------------------------------------------------
    // UNARY OPERATORS
    // ------------------------------------------------------------
    test("-5", -5);
    test("--5", 5);
    test("+-5", -5);
    test("-(-(-5))", -5);
    test("-(2+3)", -5);

    // ------------------------------------------------------------
    // PARENTHESES DEPTH
    // ------------------------------------------------------------
    test("(((((3)))))", 3);
    test("((2+(3*4)))", 14);
    test("((((2))))+((((3))))", 5);

    // ------------------------------------------------------------
    // FUNCTIONS (existing + new)
    // ------------------------------------------------------------
    test("sqrt(9)", 3);
    test("sin(0)", 0);
    test("cos(0)", 1);
    test("tan(0)", 0);
    test("asin(0)", 0);
    test("acos(1)", 0);
    test("atan(1)", std::atan(1));
    test("exp(1)", std::exp(1));
    test("log(8,2)", 3);
    test("max(3,5,10)", 10);
    test("min(3,5,10)", 3);
    test("log(8)", std::log(8));

    // ------------------------------------------------------------
    // NESTED FUNCTIONS
    // ------------------------------------------------------------
    test("sqrt(max(9,16))", 4);
    test("sin(cos(0))", std::sin(std::cos(0)));
    test("log(max(8,2),2)", 3);
    test("tan(sin(0))", 0);
    test("exp(sqrt(4))", std::exp(2));

    // ------------------------------------------------------------
    // CONSTANTS
    // ------------------------------------------------------------
    test("pi", M_PI);
    test("e", M_E);
    test("sin(pi/2)", 1);

    // ------------------------------------------------------------
    // WHITESPACE HANDLING
    // ------------------------------------------------------------
    test("   2 +   3 *   4   ", 14);
    test("sqrt ( 9 )", 3);

    // ------------------------------------------------------------
    // ANS MEMORY TESTS
    // ------------------------------------------------------------
    {
        Evaluator ev;
        auto ast1 = Parser(Tokenizer("2+3")).parseExpression();
        double r1 = ev.eval(ast1.get());
        if (!approx(ev.variables["ANS"], 5)) {
            std::cerr << "FAILED: ANS after 2+3 should be 5\n";
            std::exit(1);
        }

        auto ast2 = Parser(Tokenizer("ANS * 2")).parseExpression();
        double r2 = ev.eval(ast2.get());
        if (!approx(r2, 10)) {
            std::cerr << "FAILED: ANS * 2 should be 10\n";
            std::exit(1);
        }
    }

    // ANS inside functions
    {
        Evaluator ev;
        ev.eval(Parser(Tokenizer("10")).parseExpression().get()); // ANS = 10
        double r = ev.eval(Parser(Tokenizer("sqrt(ANS)")).parseExpression().get());
        if (!approx(r, std::sqrt(10))) {
            std::cerr << "FAILED: sqrt(ANS) incorrect\n";
            std::exit(1);
        }
    }

    // ANS chained
    {
        Evaluator ev;
        ev.eval(Parser(Tokenizer("5")).parseExpression().get()); // ANS = 5
        ev.eval(Parser(Tokenizer("ANS + 5")).parseExpression().get()); // ANS = 10
        double r = ev.eval(Parser(Tokenizer("ANS * ANS")).parseExpression().get());
        if (!approx(r, 100)) {
            std::cerr << "FAILED: ANS * ANS incorrect\n";
            std::exit(1);
        }
    }

    // ------------------------------------------------------------
    // LONG EXPRESSIONS (stress)
    // ------------------------------------------------------------
    test("1+2+3+4+5+6+7+8+9+10", 55);
    test("2*2*2*2*2*2*2*2", 256);
    test("((((1+2)*3)-4)/5)^2", std::pow(((1+2)*3 - 4)/5, 2));

    // ------------------------------------------------------------
    // ERROR CASES (syntax errors)
    // ------------------------------------------------------------
    test_error("((2+3)");
    test_error("sqrt()");
    test_error("max()");
    test_error("unknownFunc(3)");
    test_error("2*/3");
    test_error("sin(");
    test_error(")2+3(");
    test_error("2+*3");
    test_error("2**3");
    test_error("2//3");
    test_error("2^^3");
    test_error("2+(");
    test_error("min(3,)");
    test_error("min(,3)");

    // ------------------------------------------------------------
    // DOMAIN ERRORS (runtime errors)
    // ------------------------------------------------------------
    test_error("sqrt(-1)");
    test_error("log(-1,2)");
    test_error("log(8,0)");
    test_error("log(8,1)");
    test_error("asin(2)");
    test_error("acos(-2)");

    // ------------------------------------------------------------
    // DIVISION BY ZERO
    // ------------------------------------------------------------
    test_error("1/0");
    test_error("10/(5-5)");

    // ------------------------------------------------------------
    // EXTREME NESTING (parser stress)
    // ------------------------------------------------------------
    test("((((((((((2))))))))))", 2);

    std::cout << "All tests passed.\n";
}

