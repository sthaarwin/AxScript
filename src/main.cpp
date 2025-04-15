// main.cpp
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <readline/readline.h>
#include <readline/history.h>
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
        using_history();
        
        std::string line;
        while (true) {
            char* lineRaw = readline(">> ");
            if (lineRaw == nullptr) {
                std::cout << std::endl;
                break;
            }
            
            line = lineRaw;
            
            if (!line.empty()) {
                add_history(lineRaw);
            }
            
            free(lineRaw);
            
            if (line == "exit") {
                std::cout << "Exiting!";
                break;
            }
            
            run(line);
        }
        
        clear_history();
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