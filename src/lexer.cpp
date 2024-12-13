#include "lexer.h"
#include <cctype>
#include<iostream>

Lexer::Lexer(const std::string source): source(source){}

std::vector<Token> Lexer::lex(){
    
    std::vector<Token> tokens;
    std::cout<< std::endl << "Source code: " << source << std::endl<<std::endl;

    while(!isAtEnd()){
        skipWhiteSpace();
        char ch = currentChar();
        Token token;

        if(std::isalpha(ch)){
            token = identifier();
        }
        else if(std::isdigit(ch)){
            token = number();
        }  
        else if(ch == '"'){
            token = string();
        }
        else{
            token.type = identifyToken(ch);
            token.lexeme = std::string(1, ch);
            advance();
        }
        tokens.push_back(token);
    }

    Token eofToken;
    eofToken.type = TokenType::EOF_TOKEN;
    tokens.push_back(eofToken);
    return tokens;
}

char Lexer::currentChar(){
    return source[current];
}

void Lexer::advance(){
    if (!isAtEnd()) { 
        current++;
    }
}

TokenType Lexer::identifyToken(char ch){
     switch (ch) {
        case '(': return TokenType::LEFT_PAREN;
        case ')': return TokenType::RIGHT_PAREN;
        case '{': return TokenType::LEFT_BRACE;
        case '}': return TokenType::RIGHT_BRACE;
        case ',': return TokenType::COMMA;
        case '.': return TokenType::DOT;
        case '-': return TokenType::MINUS;
        case '+': return TokenType::PLUS;
        case ';': return TokenType::SEMICOLON;
        case '/': return TokenType::SLASH;
        case '*': return TokenType::STAR;
        case '!': return TokenType::BANG;
        case '=': return TokenType::EQUAL;
        case '>': return TokenType::GREATER;
        case '<': return TokenType::LESS;
        default: return TokenType::EOF_TOKEN;
    }
}
void Lexer::skipWhiteSpace(){
    while(!isAtEnd() && (currentChar() == ' ' || currentChar() == '\t' || currentChar() == '\n')){
        advance();
    }
}
bool Lexer::isAtEnd() { 
    return current >= source.size();
}


Token Lexer::string(){
    std::string lexeme;
    advance();
    while(!isAtEnd() && currentChar() != '"'){
        lexeme += currentChar();
        advance();
    }
    advance();
    Token token;
    token.type = TokenType::STRING;
    token.lexeme = lexeme;
    return token;
}

Token Lexer::number(){
    std::string lexeme;
    while(std::isalnum(currentChar())){
        lexeme += currentChar();
        advance();
    }
    Token token;
    token.type = TokenType::NUMBER;
    token.lexeme = lexeme;
    return token;
}

Token Lexer::identifier(){
    std::string lexeme;
    while(std::isalnum(currentChar()) || currentChar() == '_'){
        lexeme += currentChar();
        advance();
    }
    Token token;
    token.type = TokenType::IDENTIFIER;
    token.lexeme = lexeme;
    return token;
}