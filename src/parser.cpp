#include "parser.hpp"

// ('+' | '-')
std::unique_ptr<ASTNode> Parser::parseExpression() {
    auto node = parseTerm();

    // binary + and - between terms
    while (isType(TokenType::Plus) ||
           isType(TokenType::Minus) ) {

        char op = isType(TokenType::Plus) ? '+' : '-'; 
        next();

        auto right = parseTerm(); // goes to the next step of attribution
        node = std::make_unique<BinaryNode>(op, std::move(node), std::move(right));
    }

    return node;
}

// ('*' | '/' | '%') 
std::unique_ptr<ASTNode> Parser::parseTerm() {
    auto node = parseFactor();

    while (isType(TokenType::Star) ||
           isType(TokenType::Slash) ||
           isType(TokenType::Mod)) {

        char op;
        if (isType(TokenType::Star))       op = '*';
        else if (isType(TokenType::Slash)) op = '/';
        else                               op = '%';
        next();

        auto right = parseFactor();
        node = std::make_unique<BinaryNode>(op, std::move(node), std::move(right));
    }

    return node;
}

 // exponentiation: ^ is binary and right-associative
std::unique_ptr<ASTNode> Parser::parseFactor() {
    auto node = parseUnary();

    if (isType(TokenType::Hat)) {
        char op = '^';
        next();

        // recursion makes it right associative
        auto right = parseFactor();
        node = std::make_unique<BinaryNode>(op, std::move(node), std::move(right));
    }
    return node;
}


// ('+' | '-')
// only at the start of a subexpression
std::unique_ptr<ASTNode> Parser::parseUnary() {
    if (isType(TokenType::Plus) ||
        isType(TokenType::Minus) ) {

        char op = isType(TokenType::Plus) ? '+' : '-'; 
        next();

        auto child = parseUnary();
         // unary chains: +---+3 becomes nested UnaryNodes
        return std::make_unique<UnaryNode>(op, std::move(child));
    }

    return parsePostfix();
}

// ('!')
std::unique_ptr<ASTNode> Parser::parsePostfix() {
    auto node = parsePrimary();
    while (isType(TokenType::Factorial)) {
        next();
        node = std::make_unique<FactorialNode>(std::move(node));
    }
    return node;
}

// number | identifier | function-call | '(' expression ')'
std::unique_ptr<ASTNode> Parser::parsePrimary() {
    // number
    if (isType(TokenType::Number)) {
        double value = std::stod(current.value); //stod converts strings to double
        next();
        return std::make_unique<NumberNode>(value);
    }

    // identifier or function call
    if (isType(TokenType::Identifier)) {
        std::string name = current.value;
        next();

        // function call: name '(' ...
        if (accept(TokenType::LParen)) {
            return parseFunctionCall(name);
        }

        // plain identifier
        return std::make_unique<IdentifierNode>(name);
    }

    // '(' expression ')'
    if (accept(TokenType::LParen)) {
        auto node = parseExpression();
        expect(TokenType::RParen);
        return node;
    }

    // handle unexpected
    std::string error_msg{};
    if(isType(TokenType::Comma)) error_msg = " - commas are only allowed in function calls";
    if(isType(TokenType::RParen)) error_msg = " - ) can only be used to close expressions";


    throw std::runtime_error("Unexpected token - " + tokenTypeToString(current.type)
                             + " - at line "+ std::to_string(current.line) + " and column " + std::to_string(current.column)
                             + error_msg);
}

// identifier '(' arguments ')'
// arguments in expression ( ',' )
std::unique_ptr<ASTNode> Parser::parseFunctionCall(const std::string& name) {
    std::vector<std::unique_ptr<ASTNode>> args;

    // handle empty argument list: func()
    if (!accept(TokenType::RParen)) {
        // at least one argument
        do {
            args.push_back(parseExpression());
        } while (accept(TokenType::Comma));

        expect(TokenType::RParen);
    }

    return std::make_unique<FunctionCallNode>(name, std::move(args));
}

std::unique_ptr<ASTNode> Parser::parse() {
    auto ast = parseExpression();

    if (!isType(TokenType::End)) {
        std::string error_msg = " - extra input after complete expression";
        throw std::runtime_error(
            "Unexpected token - " + tokenTypeToString(current.type)
            + " - at line " + std::to_string(current.line)
            + " and column " + std::to_string(current.column)
            + error_msg
        );
    }

    return ast;
}


