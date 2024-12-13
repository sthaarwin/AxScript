#include<iostream>
#include<fstream>
#include<string>
#include<sstream>
#include<cstdlib>
#include"lexer.h"
#include"tokens.h"


class AxScript{
    public:

        static void runFile(std::string filename){
            std::ifstream file(filename);
            if(!file.is_open()){
                std::cerr << "Error: Could not open file " << filename << std::endl;
                std::exit(65);
            }
            std::stringstream buffer;
            buffer << file.rdbuf();
            run(buffer.str());
            file.close();
        }
        
        static void runPrompt(){
            std::string line;
            while(true){
             std::cout << ">> ";
             if(!std::getline(std::cin, line) || line.empty()){
                 std::cout << std::endl;
                 break;
             }
            }
        }

        static void run(std::string source){
          Lexer lexer(source);
            std::vector<Token> tokens = lexer.lex();
            for(Token token : tokens){
                std::cout <<" Token : "<< tokenTypeToString(token.type) << "\n Lexeme : " << token.lexeme << std::endl;
            }
        }
};

int main(int argc, char* argv[]){
    if(argc < 2){
        std::cerr << "Usage : " << argv[0] << " <filename>" << std::endl;
        return 1;
    }
    else if(argc ==  2){
        AxScript::runFile(argv[1]);
    }
    else{
        AxScript::runPrompt();
    }
}