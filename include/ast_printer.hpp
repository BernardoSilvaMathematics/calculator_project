#pragma once
#include "ast.hpp"
#include <string>
#include <sstream>

class ASTPrinter {
/*
 * Builds a readable tree showing how operations are grouped
 * Developer-only feature; not intended for end users
 */
public:
    static std::string print(const ASTNode* node);

private:
    static void printNode(const ASTNode* node,
                          std::string& out,
                          const std::string& indent,
                          bool isLast);
};

inline std::string ASTPrinter::print(const ASTNode* node) {
    std::string out;
    printNode(node, out, "", true);
    return out;
}

inline void ASTPrinter::printNode(const ASTNode* node,
                                  std::string& out,
                                  const std::string& indent,
                                  bool isLast){
    // prefix ├── or └──
    out += indent;
    out += isLast ? "└── " : "├── ";

    // identify node type
    if (auto num = dynamic_cast<const NumberNode*>(node)) {
        out += "Number(" + std::to_string(num->value) + ")\n";
        return;
    }

    if (auto id = dynamic_cast<const IdentifierNode*>(node)) {
        out += "Identifier(" + id->name + ")\n";
        return;
    }

    if (auto un = dynamic_cast<const UnaryNode*>(node)) {
        out += std::string("Unary(") + un->op + ")\n";

        // child
        printNode(un->child.get(),
                  out,
                  indent + (isLast ? "    " : "│   "),
                  true);
        return;
    }

    if (auto un = dynamic_cast<const FactorialNode*>(node)) {
        out += "Factorial(!)\n";

        // child
        printNode(un->child.get(),
                out,
                indent + (isLast ? "    " : "│   "),
                true);
        return;
    }


    if (auto bin = dynamic_cast<const BinaryNode*>(node)) {
        out += std::string("Binary(") + bin->op + ")\n";

        // left child
        printNode(bin->left.get(),
                  out,
                  indent + (isLast ? "    " : "│   "),
                  false);

        // right child
        printNode(bin->right.get(),
                  out,
                  indent + (isLast ? "    " : "│   "),
                  true);
        return;
    }

    if (auto fn = dynamic_cast<const FunctionCallNode*>(node)) {
        out += "FunctionCall(" + fn->name + ")\n";

        for (size_t i = 0; i < fn->args.size(); ++i) {
            bool lastArg = (i == fn->args.size() - 1);
            printNode(fn->args[i].get(),
                      out,
                      indent + (isLast ? "    " : "│   "),
                      lastArg);
        }
        return;
    }

    // fallback
    out += "UnknownNode\n";
}


