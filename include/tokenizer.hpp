#pragma once
#include <string>
#include <vector>
#include <stdexcept>

enum class TokenType {
    Number,
    Identifier,
    Plus,
    Minus,
    Star,
    Slash,
    Hat,
    Mod,
    Factorial,
    LParen,
    RParen,
    Comma,
    End
};

struct Token {
    TokenType type;
    std::string value; // empty for operators
    size_t line;
    size_t column;
};

class Tokenizer {
    public:
        Tokenizer(const std::string& input);
        Token nextToken();
    private:
        std::string input;
        size_t pos{0};
        size_t line{1};
        size_t column{1};
};