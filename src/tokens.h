// tokens.h

#ifndef TOKEN_H
#define TOKEN_H

#include <string>   

enum class TokenType{
    LEFT_PAREN, RIGHT_PAREN, LEFT_BRACE, RIGHT_BRACE,
    COMMA, DOT, MINUS, PLUS, SEMICOLON, SLASH, STAR,
    INPUT, BANG, BANG_EQUAL, EQUAL, EQUAL_EQUAL,
    GREATER, GREATER_EQUAL, LESS, LESS_EQUAL,
    IDENTIFIER, STRING, NUMBER, AND, CLASS, ELSE,
    FALSE, FUN, FOR, IF, NIL, OR, PRINT, RETURN,
    SUPER, THIS, TRUE, VAR, WHILE, EOF_TOKEN
};

struct Token {
    TokenType type;
    std::string lexeme;
    std::string literal;  
    int line;

    Token(TokenType type = TokenType::EOF_TOKEN, 
          std::string lexeme = "", 
          std::string literal = "",
          int line = 1) : type(type), lexeme(lexeme), literal(literal), line(line) {}
};

std::string tokenTypeToString(TokenType type);

#endif // TOKEN_H