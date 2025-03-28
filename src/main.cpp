// main.cpp
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cstdlib>
#include "lexer.h"
#include "tokens.h"
#include "parser.h"
#include "interpreter.h"

class AxScript {
public:

    static void Guide() {
        std::cout << "AxScript v1.0.0" << std::endl;
        std::cout << "Usage: axscript [filename]" << std::endl;
    }

    static void runFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file " << filename << std::endl;
            std::exit(65);
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        run(buffer.str());
        file.close();
    }

    static void runPrompt() {
        while (true) {
            std::cout << ">> ";
            std::string line;
            if (!std::getline(std::cin, line)) {
                break;
            }

            if (line.empty() || line == "exit") {
                std::cout << "\nExiting!" << std::endl;
                break;
            }

            run(line);
        }
    }

    static void run(const std::string& source) {
        try {
            Lexer lexer(source);
            std::vector<Token> tokens = lexer.lex();

            Parser parser(tokens);
            std::vector<std::unique_ptr<Stmt>> statements = parser.parse();

            Interpreter interpreter;
            interpreter.interpret(statements);
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc == 2) {
        AxScript::runFile(argv[1]);
    } else {
        AxScript::Guide();
        AxScript::runPrompt();
    }
    return 0;
}