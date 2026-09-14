#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <memory>
#include <filesystem>

#include "tokenizer.hpp"
#include "parser.hpp"
#include "ast_printer.hpp"
#include "evaluator.hpp"
#include "graph_module.hpp"

void handleGraphAdd(GraphModule& graph, const std::string& input) {
    std::string rest = input.substr(std::string("graphadd ").size());
    std::stringstream ss(rest);
    std::string token;
    std::vector<std::string> tokens;

    while (ss >> token)
        tokens.push_back(token);

    if (tokens.empty()) {
        std::cout << "Usage: graphadd <expr> [xmin=] [xmax=] [res=]\n";
        return;
    }

    std::string expr;
    size_t paramStart = tokens.size();
    for (size_t i = 0; i < tokens.size(); ++i) {
        const std::string& t = tokens[i];
        if (t.rfind("xmin=", 0) == 0 || t.rfind("xmax=", 0) == 0 || t.rfind("res=", 0) == 0) {
            paramStart = i;
            break;
        }
        if (!expr.empty()) expr += " ";
        expr += t;
    }

    double xmin = -10;
    double xmax = 10;
    int resolution = 1000;

    for (size_t i = paramStart; i < tokens.size(); ++i) {
        const std::string& t = tokens[i];
        if (t.rfind("xmin=", 0) == 0)
            xmin = std::stod(t.substr(5));
        else if (t.rfind("xmax=", 0) == 0)
            xmax = std::stod(t.substr(5));
        else if (t.rfind("res=", 0) == 0)
            resolution = std::stoi(t.substr(4));
        else
            std::cout << "Unknown parameter: " << t << "\n";
    }

    graph.plot(expr, xmin, xmax, resolution);
    std::cout << "Added curve: " << expr << "\n";
}

int main(int argc, char* argv[]) {
    bool printAST = false;
    if (argc > 1 && std::string(argv[1]) == "--ast")
        printAST = true;

    Evaluator evaluator;
    GraphModule graph(800, 600);

    while (true) {
        std::string input;
        std::cout << "Enter expression: ";
        if (!std::getline(std::cin, input) || input.empty())
            break;

        if (input.rfind("graphadd ", 0) == 0) {
            try {
                handleGraphAdd(graph, input);
            } catch (const std::exception& e) {
                std::cerr << "Error: " << e.what() << "\n";
            }
            continue;
        }

        if (input == "graphshow") {
            if (graph.empty()) {
                std::cout << "No curves to display. Use graphadd first.\n";
                continue;
            }
            try {
                graph.render();
            } catch (const std::exception& e) {
                std::cerr << "Error: " << e.what() << "\n";
            }
            graph.clear();
            continue;
        }

        try {
            Tokenizer tokenizer(input);
            Parser parser(std::move(tokenizer));
            std::unique_ptr<ASTNode> ast = parser.parse();

            if (printAST) {
                std::cout << "\nAST Structure:\n";
                std::cout << ASTPrinter::print(ast.get()) << "\n";
            }

            double result = evaluator.eval(ast.get());
            std::cout << "Result: " << result << "\n";
        }
        catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << "\n";
        }
    }

    std::cout << "\nSession ended. Hope it was fun!\n";
    return 0;
}
