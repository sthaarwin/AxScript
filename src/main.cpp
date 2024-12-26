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

class AxScript
{
public:
    static void runFile(std::string filename)
    {
        std::ifstream file(filename);
        if (!file.is_open())
        {
            std::cerr << "Error: Could not open file " << filename << std::endl;
            std::exit(65);
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        run(buffer.str());
        file.close();
    }

    static void runPrompt()
    {
        while (true)
        {
            std::cout << ">> ";
            std::string line;
            if (!std::getline(std::cin, line))
            {
                break;
            }

            if (line.empty() || line == "exit")
            {
                std::cout << "\nExiting!" << std::endl;
                break;
            }

            run(line);
        }
    }

    static void run(std::string source)
    {
        Lexer lexer(source);
        std::vector<Token> tokens = lexer.lex();

        Parser parser(tokens);
        std::vector<std::unique_ptr<Stmt>> statements = parser.parse();

        Interpreter interpreter;
        // Execute each statement
        for (auto &stmt : statements)
        {
            interpreter.interpret(stmt);
        }
    }
    
};

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage : " << argv[0] << " <filename>" << std::endl;
        return 1;
    }
    else if (argc == 2)
    {
        AxScript::runFile(argv[1]);
    }
    else
    {
        AxScript::runPrompt();
    }
    return 0;
}