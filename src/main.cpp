#include <iostream>
#include <string>

#include "tokenizer.hpp"
#include "parser.hpp"
#include "ast_printer.hpp"
#include "evaluator.hpp"

int main(int argc, char* argv[]) {
    bool printAST = false;

    // Optional flag: --ast
    if (argc > 1 && std::string(argv[1]) == "--ast")
        printAST = true;

    Evaluator evaluator;   // persists ANS across expressions

    while (true) {
        std::string input;
        std::cout << "Enter expression: ";

        // Exit on EOF or empty line
        if (!std::getline(std::cin, input) || input.empty())
            break;


        try {
            // Tokenize
            Tokenizer tokenizer(input);

            // Parse
            Parser parser(std::move(tokenizer));
            std::unique_ptr<ASTNode> ast = parser.parse();

            // Optional AST visualization
            if (printAST) {
                std::cout << "\nAST Structure:\n";
                std::cout << ASTPrinter::print(ast.get()) << "\n";
            }

            // Evaluate (updates ANS)
            double result = evaluator.eval(ast.get());

            std::cout << "Result: " << result << "\n";
        }
        catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << "\n";
        }
    }

    std::cout << "\n Session ended. Hope it was fun! \n";

    return 0;
}
