#include "tokenizer.hpp"
#include <cctype>


Tokenizer::Tokenizer(const std::string& input):input(input), pos(0) {};

Token Tokenizer::nextToken() {
        /*
        * Lexes the next token from the input, keeping track of line and column.
        *
        * Skips whitespace
        * Recognizes and validates numeric literals
        * Recognizes identifiers (uppercase)
        * Recognizes operators and parentheses
        */

        size_t size = input.size();

        auto advance = [&]() {
            char c = input[pos++];
            if (c == '\n') {
                line++;
                column = 1;
            } else {
                column++;
            }
            return c;
        };

        // Skip whitespace
        while (pos < size && std::isspace(input[pos])) {
            advance();
        }
        // Records the starting line and column
        size_t start_line = line;
        size_t start_col = column;

        if (pos >= size)
            return {TokenType::End, "", start_line, start_col};

        char ch = input[pos];

        // Number
        if (std::isdigit(ch) || ch == '.') {
            std::string id;
            bool one_dot = false;
            bool seen_e = false;
            bool fail = false;

            // Build number token
            while (pos < size) {
                char c = input[pos];

                // digits
                if (std::isdigit(c)) {
                    id += advance();
                }
                // decimal point
                else if (c == '.') {
                    if (one_dot) {
                        fail = true;
                        id += advance();
                        break;
                    }
                    if (seen_e) {
                        fail = true;
                        id += advance();
                        break;
                    }
                    one_dot = true;
                    id += advance();
                }
                // exponent marker
                else if (c == 'e' || c == 'E') {
                    if (seen_e) {
                        fail = true;
                        id += advance();
                        break;
                    }
                    seen_e = true;
                    id += std::toupper(static_cast<unsigned char>(advance()));

                    // optional sign after exponent
                    if (pos < size && (input[pos] == '+' || input[pos] == '-')) {
                        id += advance();
                    }

                    // must have at least one digit after e or sign
                    if (pos >= size || !std::isdigit(input[pos])) {
                        fail = true;
                        break;
                    }
                }
                else {
                    break; // end of number
                }
            }

            // After building id and detecting fail
            std::string error_msg{id};

            if (one_dot && seen_e && id.rfind(".E", 0) == 0) fail = true;

            if (fail) {
                if (seen_e && id.back() == '.') {
                    error_msg += " invalid number: cannot have '.' after exponent";
                }
                else if (id.back() == '.') {
                    error_msg += " invalid number: multiple dots or trailing '.'";
                }
                else if (seen_e) {
                    error_msg += " malformed exponent";
                }
                else {
                    error_msg += " invalid number format";
                }

                throw std::domain_error(
                    "Invalid number at line " +
                    std::to_string(start_line) +
                    ", column " +
                    std::to_string(start_col) +
                    ": " + error_msg
                );
            }


            // normalize leading/trailing dot
            if (id.front() == '.') id.insert(id.begin(), '0');
            if (id.back() == '.') id.push_back('0');

            return Token{TokenType::Number, id, start_line, start_col};
        }


        // Identifier
        if (std::isalpha(ch)) {
            std::string id{};
            while (pos < size && std::isalpha(input[pos])) {
                id += std::toupper(static_cast<unsigned char>(advance()));
            }
            return Token{TokenType::Identifier, id, start_line, start_col};
        }

        // Operators
        char c = advance();
        switch (c) {
            case '+': return {TokenType::Plus, "", start_line, start_col};
            case '-': return {TokenType::Minus, "", start_line, start_col};
            case '*': return {TokenType::Star, "", start_line, start_col};
            case '/': return {TokenType::Slash, "", start_line, start_col};
            case '^': return {TokenType::Hat, "", start_line, start_col};
            case '%': return {TokenType::Mod, "", start_line, start_col};
            case '!': return {TokenType::Factorial, "", start_line, start_col};
            case '(': return {TokenType::LParen, "", start_line, start_col};
            case ')': return {TokenType::RParen, "", start_line, start_col};
            case ',': return {TokenType::Comma, "", start_line, start_col};
        }

        throw std::domain_error("Unknown character at line " +
                                std::to_string(start_line) +
                                ", column " +
                                std::to_string(start_col));
    }

            
        
