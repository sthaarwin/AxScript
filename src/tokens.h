// tokens.h

#ifndef TOKEN_H
#define TOKEN_H

#include <string>   

enum class TokenType {
    // Single character tokens
    LEFT_PAREN, RIGHT_PAREN, LEFT_BRACE, RIGHT_BRACE,
    LEFT_BRACKET, RIGHT_BRACKET, // Added for array support
    COMMA, DOT, MINUS, PLUS, SEMICOLON, SLASH, STAR, PERCENT, // Added PERCENT
    
    // One or two character tokens
    INPUT, BANG, BANG_EQUAL, EQUAL, EQUAL_EQUAL,
    GREATER, GREATER_EQUAL, LESS, LESS_EQUAL,
    
    // Literals
    IDENTIFIER, STRING, NUMBER, 
    
    // Keywords
    AND, OR, NOT, CLASS, FALSE, FUN, FOR, IF, ELSE, ELSEIF, 
    NIL, PRINT, RETURN_KW, SUPER, THIS, TRUE, VAR, WHILE, 
    
    // Special keywords for AxScript
    LOOP, TO, STEP, BREAK, CONTINUE, DOWN,
    COMPEQ, COMPNEQ, COMPGE, COMPLE, COMPG, COMPL,
    
    // End of file marker
    EOF_TOKEN
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