#include "ast.hpp"
#include <cmath>
#include <unordered_map>
#include <stdexcept>
#include <functional>
#include <algorithm>


using Func = std::function<double(const std::vector<double>&)>;

// Constants allowed in the calculator
const std::unordered_map<std::string, double> identifiersConst = {
    {"PI", 3.14159265358979323846},
    {"E", 2.71828182845904523536},
    {"TAU", 6.28318530717958647692},
    {"PHI", 1.61803398874989484820}
};


/*
 * Map of supported functions
 * Each entry contains its own evaluation logic, so the evaluator
 * does not need special handling for individual functions.
 */

const std::unordered_map<std::string, Func> functions = {
    {"SIN", [](auto& a){
        if (a.size() != 1)
            throw std::runtime_error("sin() takes exactly 1 argument");
        return std::sin(a[0]);
    }},
    {"COS", [](auto& a){
        if (a.size() != 1)
            throw std::runtime_error("cos() takes exactly 1 argument");
        return std::cos(a[0]);
    }},
    {"TAN", [](auto& a){
        if (a.size() != 1)
            throw std::runtime_error("tan() takes exactly 1 argument");
        return std::tan(a[0]);
    }},
    {"ASIN", [](auto& a){
        if (a.size() != 1)
            throw std::runtime_error("asin() takes exactly 1 argument");
        double x = a[0];
        if (x < -1.0 || x > 1.0)
            throw std::runtime_error("asin() domain error: input must be in [-1, 1]");
        return std::asin(x);
    }},
    {"ACOS", [](auto& a){
        if (a.size() != 1)
            throw std::runtime_error("acos() takes exactly 1 argument");
        double x = a[0];
        if (x < -1.0 || x > 1.0)
            throw std::runtime_error("acos() domain error: input must be in [-1, 1]");
        return std::acos(x);
    }},
    {"ATAN", [](auto& a){
        if (a.size() != 1)
            throw std::runtime_error("atan() takes exactly 1 argument");
        return std::atan(a[0]);
    }},
    {"EXP", [](auto& a){
        if (a.size() != 1)
            throw std::runtime_error("exp() takes exactly 1 argument");
        return std::exp(a[0]);
    }},
    {"SQRT", [](auto& a){
        if (a.size() != 1)
            throw std::runtime_error("sqrt() takes exactly 1 argument");
        if (a[0] < 0)
            throw std::runtime_error("sqrt() domain error: negative input");
        return std::sqrt(a[0]);
    }},
    {"LOG", [](auto& a){
        if (a.size() == 1) {
            double x = a[0];
            if (x <= 0)
                throw std::runtime_error("log(x) domain error: x must be > 0");
            return std::log(x);
        }
        if (a.size() == 2) {
            double x = a[0];
            double base = a[1];
            if (x <= 0)
                throw std::runtime_error("log(x, base) domain error: x must be > 0");
            if (base <= 0 || base == 1)
                throw std::runtime_error("log(x, base) invalid base: must be > 0 and ≠ 1");
            return std::log(x) / std::log(base);
        }
        throw std::runtime_error("log() takes 1 or 2 arguments");
    }},
    {"MAX", [](auto& a){
        if (a.empty())
            throw std::runtime_error("max() requires at least one argument");
        return *std::max_element(a.begin(), a.end());
    }},
    {"MIN", [](auto& a){
        if (a.empty())
            throw std::runtime_error("min() requires at least one argument");
        return *std::min_element(a.begin(), a.end());
    }},
    {"ABS", [](auto& a){
        if (a.size() != 1)
            throw std::runtime_error("ABS() takes exactly 1 argument");
        return std::abs(a[0]);
    }}
};



class Evaluator {
public:
    std::unordered_map<std::string, double> variables;

    Evaluator() {
        variables["ANS"] = 0.0;   // initialize memory
    }

    double eval(const ASTNode* node) {
        double result = node_eval(node);
        variables["ANS"] = result;
        return result;
    }

private:

    double node_eval(const ASTNode* node) {
        /*
        * Recursively evaluates the AST. Each node type encodes its own semantics.
        * Binary operators are evaluated left to right except '^', which is right-associative
        */

        // Number literal
        if (auto num = dynamic_cast<const NumberNode*>(node))
            return num->value;

        // Identifier (constant or variable)
        if (auto id = dynamic_cast<const IdentifierNode*>(node)) {

            if (auto cit = identifiersConst.find(id->name); cit != identifiersConst.end())
                return cit->second;

            if (auto vit = variables.find(id->name); vit != variables.end())
                return vit->second;

            throw std::runtime_error("Unknown identifier: " + id->name);
        }

        // Unary
        if (auto un = dynamic_cast<const UnaryNode*>(node)) {
            double child = node_eval(un->child.get());
            double sign = (un->op == '+' ? 1.0 : -1.0);
            return sign * child;
        }

        // Factorial
        if (auto un = dynamic_cast<const FactorialNode*>(node)) {
            double child = node_eval(un->child.get());
            if (child < 0) throw std::runtime_error("Factorial(!) domain error: negative input");
            double result;
            if (std::floor(child) == child && child <= 20) {
                unsigned long long n = static_cast<unsigned long long>(child);
                unsigned long long r = 1;
                for (unsigned long long i = 2; i <= n; ++i) r *= i;
                result = static_cast<double>(r);
            } else {
                result = std::tgamma(child + 1.0);
            }
            if (!std::isfinite(result))
                throw std::runtime_error("Factorial(!) overflow: result too large");
            return result;
        }

        // Binary
        if (auto bin = dynamic_cast<const BinaryNode*>(node)) {
            double left  = node_eval(bin->left.get());
            double right = node_eval(bin->right.get());

            auto safe = [&](double value, char op) {
                if (!std::isfinite(value))
                    throw std::runtime_error("Numeric overflow in operator '" + std::string(1, op) + "'");
                return value;
            };

            switch (bin->op) {
                case '+': return safe(left + right, '+');
                case '-': return safe(left - right, '-');
                case '*': return safe(left * right, '*');
                case '/':
                    if (right == 0)
                        throw std::runtime_error("Invalid operation - Division by zero");
                    return safe(left / right, '/');
                case '^': { 
                    return safe(std::pow(left, right), '^');
                }
                case '%':{
                    if(right == 0)
                        throw std::runtime_error("Invalid operation - Moduli by 0");
                    return safe(std::fmod(left,right), '%');
                }
                default:
                    throw std::runtime_error("Unknown operator: " + std::string(1, bin->op));
            }
        }

        // Function call
        if (auto fn = dynamic_cast<const FunctionCallNode*>(node)) {

            std::vector<double> args;
            args.reserve(fn->args.size());
            for (auto& arg : fn->args)
                args.push_back(node_eval(arg.get()));

            auto it = functions.find(fn->name);
            if (it == functions.end())
                throw std::runtime_error("Unknown function: " + fn->name);

            double result = it->second(args);

            if (!std::isfinite(result))
                throw std::runtime_error("Function '" + fn->name + "' produced a non‑finite result");

            return result;
        }

        throw std::runtime_error("Unknown AST node type");
    }
};
