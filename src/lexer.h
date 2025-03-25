// lexer.h

#ifndef LEXER_H
#define LEXER_H


#include "tokens.h"
#include <string>
#include <vector>
#include <unordered_map>

class Lexer
{

public:
    Lexer(const std::string source);

    std::vector<Token> lex();

private:
    std::string source;

    size_t current;

    int line;

    std::unordered_map<std::string, TokenType> keywords = {
        {"break", TokenType::BREAK},
        {"continue", TokenType::CONTINUE},
        {"down", TokenType::DOWN},
        {"to", TokenType::TO},
        {"step", TokenType::STEP},
        {"compeq", TokenType::COMPEQ},
        {"compneq", TokenType::COMPNEQ},
        {"compge", TokenType::COMPGE},
        {"comple", TokenType::COMPLE},
        {"compg", TokenType::COMPG},
        {"compl", TokenType::COMPL},
        {"and", TokenType::AND},
        {"or", TokenType::OR},
        {"not", TokenType::NOT}
    };

    std::vector<Token> tokens;

    char currentChar();

    void advance();

    TokenType identifyToken(char ch);

    void skipWhiteSpace();

    bool isAtEnd();

    char peek();

    bool match(char expected);

    void addToken(TokenType type);

    Token string();

    Token number();

    Token identifier();

    void scanToken();

    void error(int line, const std::string &message);
};

#endif // LEXER_H