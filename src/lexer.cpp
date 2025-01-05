//lexer.cpp

#include "lexer.h"
#include <cctype>
#include <iostream>

Lexer::Lexer(const std::string source): source(source), current(0), line(1) {
    keywords["and"] = TokenType::AND;
    keywords["class"] = TokenType::CLASS;
    keywords["else"] = TokenType::ELSE;
    keywords["false"] = TokenType::FALSE;
    keywords["for"] = TokenType::FOR;
    keywords["fun"] = TokenType::FUN;
    keywords["if"] = TokenType::IF;
    keywords["elif"] = TokenType::ELIF;
    keywords["else"] = TokenType::ELSE;
    keywords["nil"] = TokenType::NIL;
    keywords["or"] = TokenType::OR;
    keywords["print"] = TokenType::PRINT;
    keywords["input"] = TokenType::INPUT;
    keywords["return"] = TokenType::RETURN;
    keywords["super"] = TokenType::SUPER;
    keywords["this"] = TokenType::THIS;
    keywords["true"] = TokenType::TRUE;
    keywords["var"] = TokenType::VAR;
    keywords["while"] = TokenType::WHILE;
}

std::vector<Token> Lexer::lex() {
    std::vector<Token> tokens;
    while (!isAtEnd()) {
        skipWhiteSpace();
        char ch = currentChar();
        Token token;

        if (std::isalpha(ch)) {
            token = identifier();
        }
        else if (std::isdigit(ch)) {
            token = number();
        }  
        else if (ch == '"') {
            token = string();
        }
        else {
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

char Lexer::currentChar() {
    return source[current];
}

void Lexer::advance() {
    if (!isAtEnd()) { 
        current++;
    }
}

TokenType Lexer::identifyToken(char ch) {
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

void Lexer::skipWhiteSpace() {
    while (!isAtEnd() && (currentChar() == ' ' || currentChar() == '\t' || currentChar() == '\n')) {
        if (currentChar() == '\n') {
            line++;
        }
        advance();
    }
}

bool Lexer::isAtEnd() {
    return current >= source.size();
}

char Lexer::peek() {
    if (isAtEnd()) return '\0';
    return source[current];
}

bool Lexer::match(char expected) {
    if (isAtEnd()) return false;
    if (source[current] != expected) return false;
    current++;
    return true;
}

void Lexer::addToken(TokenType type)
{
    std::string lexeme = source.substr(current, 1);
    tokens.push_back(Token(type, lexeme, "", line));
    advance();
}

Token Lexer::string() {
    std::string lexeme;
    advance();  // Skip opening quote
    while (!isAtEnd() && currentChar() != '"') {
        lexeme += currentChar();
        advance();
    }
    advance();  // Skip closing quote
    
    Token token;
    token.type = TokenType::STRING;
    token.lexeme = lexeme;
    return token;
}

Token Lexer::number() {
    std::string lexeme;
    while (!isAtEnd() && std::isdigit(currentChar())) {
        lexeme += currentChar();
        advance();
    }
    
    // Optional: Handle decimal numbers
    if (!isAtEnd() && currentChar() == '.' && std::isdigit(peek())) {
        lexeme += currentChar();
        advance();
        while (!isAtEnd() && std::isdigit(currentChar())) {
            lexeme += currentChar();
            advance();
        }
    }
    
    Token token;
    token.type = TokenType::NUMBER;
    token.lexeme = lexeme;
    return token;
}

Token Lexer::identifier() {
    std::string lexeme;
    while (!isAtEnd() && (std::isalnum(currentChar()) || currentChar() == '_')) {
        lexeme += currentChar();
        advance();
    }
    
    auto it = keywords.find(lexeme);
    TokenType type = (it != keywords.end()) ? it->second : TokenType::IDENTIFIER;

    return Token(type, lexeme, "", line);
}

void Lexer::scanToken()
{
    char ch = currentChar();
    switch (ch)
    {
    case '(':
        addToken(TokenType::LEFT_PAREN);
        break;
    case ')':
        addToken(TokenType::RIGHT_PAREN);
        break;
    case '{':
        addToken(TokenType::LEFT_BRACE);
        break;
    case '}':
        addToken(TokenType::RIGHT_BRACE);
        break;
    case ',':
        addToken(TokenType::COMMA);
        break;
    case '.':
        addToken(TokenType::DOT);
        break;
    case '-':
        addToken(TokenType::MINUS);
        break;
    case '+':
        addToken(TokenType::PLUS);
        break;
    case ';':
        addToken(TokenType::SEMICOLON);
        break;
    case '*':
        addToken(TokenType::STAR);
        break;
    case '!':
        addToken(match('=') ? TokenType::BANG_EQUAL : TokenType::BANG);
        break;
    case '=':
        addToken(match('=') ? TokenType::EQUAL_EQUAL : TokenType::EQUAL);
        break;
    case '<':
        addToken(match('=') ? TokenType::LESS_EQUAL : TokenType::LESS);
        break;
    case '>':
        addToken(match('=') ? TokenType::GREATER_EQUAL : TokenType::GREATER);
        break;
    case '/':
        if (match('/'))
        {
            while (currentChar() != '\n' && !isAtEnd())
                advance();
        }
        else
        {
            addToken(TokenType::SLASH);
        }
        break;
    case ' ':
    case '\r':
    case '\t':
        break;
    case '\n':
        line++;
        break;
    case '"':
        string();
        break;
    default:
        if (std::isdigit(ch))
        {
            number();
        }
        else if (std::isalpha(ch))
        {
            identifier();
        }
        else
        {
            error(line, "Unexpected character.");
        }
        break;
    }
}

void Lexer::error(int line, const std::string &message)
{
    std::cerr << "[line " << line << "] Error: " << message << std::endl;
}
