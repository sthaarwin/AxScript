#ifndef LEXER_H
#define LEXER_H

#include "tokens.h"
#include <string>
#include <vector>

class Lexer{
    public:
        Lexer(const std::string source);
        std::vector<Token> lex();

    private:
        std::string source;
        std::vector<Token> tokens;

        size_t current = 0;
        char currentChar();
        void advance();
        bool isAtEnd();
        TokenType identifyToken(char ch);
        void skipWhiteSpace();
        Token string();
        Token number();
        Token identifier();    
};

#endif // LEXER_H