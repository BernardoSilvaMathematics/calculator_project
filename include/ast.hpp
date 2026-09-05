#pragma once

#include <memory>
#include <string>
#include <vector>

/*
* Base class for all AST nodes.
* Allows storing any node type via std::unique_ptr<ASTNode>.
*/
struct ASTNode {
    virtual ~ASTNode() = default;
};


// Represents a numeric literal, e.g. "3.14" or "42"
struct NumberNode : ASTNode {
    double value;
    NumberNode(double v) : value(v) {}
};


// Represents an identifier: variables like ans or function names like sin
struct IdentifierNode : ASTNode {
    std::string name;
    IdentifierNode(std::string n) : name(std::move(n)) {}
};


// Represents unary operators: +x, -x
struct UnaryNode : ASTNode {
    char op;  // '+' or '-'
    std::unique_ptr<ASTNode> child;
    UnaryNode(char o, std::unique_ptr<ASTNode> c)
        : op(o), child(std::move(c)) {}
};

// Represents Factorial node: (child)!
struct FactorialNode : ASTNode {
    std::unique_ptr<ASTNode> child;
    FactorialNode(std::unique_ptr<ASTNode> c)
        : child(std::move(c)) {}
};


// Represents binary operators: +, -, *, /, ^, %
struct BinaryNode : ASTNode {
    char op;
    std::unique_ptr<ASTNode> left;
    std::unique_ptr<ASTNode> right;
    BinaryNode(char o, std::unique_ptr<ASTNode> l, std::unique_ptr<ASTNode> r)
        : op(o), left(std::move(l)), right(std::move(r)) {}
};


// Represents a function call: sin(x), log(3, 10), max(a, b, c)
struct FunctionCallNode : ASTNode {
    std::string name;
    std::vector<std::unique_ptr<ASTNode>> args;
    FunctionCallNode(std::string n, std::vector<std::unique_ptr<ASTNode>> a)
        : name(std::move(n)), args(std::move(a)) {}
};
