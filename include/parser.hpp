#pragma once

#include "tokenizer.hpp"
#include "ast.hpp"


class Parser {
public:
    Parser(Tokenizer tokenizer)
        : tokenizer(std::move(tokenizer)) {
        next(); // load first token
    }

    // Parses an expression according to grammar rules and returns its AST subtree
    // Callable function
    std::unique_ptr<ASTNode> parse(); // top level parser   
    std::unique_ptr<ASTNode> parseExpression(); // '+' or '-' as operators


private:
    Tokenizer tokenizer;
    Token current;

    // Advances to the next token from the tokenizer
    void next() {
        current = tokenizer.nextToken();
    }

    // If the current token matches 'type', consumes it and returns true
    bool accept(TokenType type) {
        if (current.type == type) {
            next();
            return true;
        }
        return false;
    }

    // Checks current type, just to improve readability
    bool isType(TokenType t) { return current.type == t; }

    // Requires the current token to match 'type'. If not, throws a syntax error.
    void expect(TokenType type) {
        if (current.type != type) {

            // Special case: we reached end of input before finding the expected token
            if (current.type == TokenType::End) {
                throw std::runtime_error(
                    "Expected '" + tokenTypeToString(type) + 
                    "' but it never appeared in the input"
                );
            }

            // Normal case: wrong token encountered
            throw std::runtime_error(
                "Expected '" + tokenTypeToString(type) +
                "' but found '" + tokenTypeToString(current.type) + "'"
            );
        }

        next();
    }

    // Converts internal TokenType values into strings
    std::string tokenTypeToString(TokenType t) {
        switch (t) {
            case TokenType::Number: return "number";
            case TokenType::Plus: return "+";
            case TokenType::Minus: return "-";
            case TokenType::Star: return "*";
            case TokenType::Slash: return "/";
            case TokenType::Hat: return "^";
            case TokenType::Mod: return "%";
            case TokenType::Factorial: return "!";
            case TokenType::LParen: return "(";
            case TokenType::RParen: return ")";
            case TokenType::Comma: return ",";
            case TokenType::Identifier: return "variable or function name";
            case TokenType::End: return "end of expression";
            default: return "unknown token";
        }
    }


    // Functions applied recursively
    std::unique_ptr<ASTNode> parseTerm(); // attributes '*', '/' or '%'
    std::unique_ptr<ASTNode> parseFactor(); // attributes '^'
    std::unique_ptr<ASTNode> parseUnary(); // attributes eventual signals to expressions
    std::unique_ptr<ASTNode> parsePostfix(); // attributes '!'
    std::unique_ptr<ASTNode> parsePrimary(); // sees if its a number / identifier / parentheses
    std::unique_ptr<ASTNode> parseFunctionCall(const std::string& name);
};
