// lexer.cpp

#include "lexer.h"
#include <cctype>
#include <iostream>

Lexer::Lexer(const std::string source) : source(source), current(0), line(1)
{
    keywords["if"] = TokenType::IF;
    keywords["else if"] = TokenType::ELSEIF;
    keywords["else"] = TokenType::ELSE;
    keywords["false"] = TokenType::FALSE;
    keywords["for"] = TokenType::FOR;
    keywords["nil"] = TokenType::NIL;
    keywords["or"] = TokenType::OR;
    keywords["print"] = TokenType::PRINT;
    keywords["input"] = TokenType::INPUT;
    keywords["var"] = TokenType::VAR;
    keywords["loop"] = TokenType::LOOP;
    keywords["to"] = TokenType::TO;
    keywords["step"] = TokenType::STEP;
    keywords["break"] = TokenType::BREAK;
    keywords["continue"] = TokenType::CONTINUE;
    keywords["down"] = TokenType::DOWN;
    keywords["compeq"] = TokenType::COMPEQ;
    keywords["compneq"] = TokenType::COMPNEQ;
    keywords["compge"] = TokenType::COMPGE;
    keywords["comple"] = TokenType::COMPLE;
    keywords["compg"] = TokenType::COMPG;
    keywords["compl"] = TokenType::COMPL;
    keywords["and"] = TokenType::AND;
    keywords["not"] = TokenType::NOT;
    keywords["true"] = TokenType::TRUE;
    // keywords["fun"] = TokenType::FUN;
    // keywords["return"] = TokenType::RETURN;
    // keywords["super"] = TokenType::SUPER;
    // keywords["this"] = TokenType::THIS;
    // keywords["while"] = TokenType::WHILE;
    // keywords["class"] = TokenType::CLASS;
}

std::vector<Token> Lexer::lex()
{
    std::vector<Token> tokens;
    while (!isAtEnd())
    {
        skipWhiteSpace();
        if (isAtEnd())
            break;

        char ch = currentChar();

        if (ch == '-' && current + 1 < source.size() && std::isdigit(source[current + 1]))
        {
            // Handle negative number
            Token token = number();
            tokens.push_back(token);
            continue;
        }

        if (std::isalpha(ch))
        {
            tokens.push_back(identifier());
        }
        else if (std::isdigit(ch))
        {
            tokens.push_back(number());
        }
        else if (ch == '"')
        {
            tokens.push_back(string());
        }
        else
        {
            TokenType type = identifyToken(ch);
            tokens.push_back(Token(type, std::string(1, ch), "", line));
            advance();
        }
    }

    tokens.push_back(Token(TokenType::EOF_TOKEN));
    return tokens;
}

char Lexer::currentChar()
{
    return source[current];
}

void Lexer::advance()
{
    if (!isAtEnd())
    {
        current++;
    }
}

TokenType Lexer::identifyToken(char ch)
{
    switch (ch)
    {
    case '(':
        return TokenType::LEFT_PAREN;
    case ')':
        return TokenType::RIGHT_PAREN;
    case '{':
        return TokenType::LEFT_BRACE;
    case '}':
        return TokenType::RIGHT_BRACE;
    case ',':
        return TokenType::COMMA;
    case '.':
        return TokenType::DOT;
    case '-':
        return TokenType::MINUS;
    case '+':
        return TokenType::PLUS;
    case ';':
        return TokenType::SEMICOLON;
    case '/':
        return TokenType::SLASH;
    case '*':
        return TokenType::STAR;
    case '!':
        return TokenType::BANG;
    case '=':
        return TokenType::EQUAL;
    case '>':
        return TokenType::GREATER;
    case '<':
        return TokenType::LESS;
    default:
        return TokenType::EOF_TOKEN;
    }
}

void Lexer::skipWhiteSpace()
{
    while (!isAtEnd())
    {
        char c = currentChar();
        
        switch (c) {
            case ' ':
            case '\r':
            case '\t':
                advance();
                break;
            case '\n':
                line++;
                advance();
                break;
            case '/':
                if (current + 1 < source.size()) {
                    if (source[current + 1] == '/') {
                        // Single-line comment: skip until end of line
                        advance();  // Skip first '/'
                        advance();  // Skip second '/'
                        while (!isAtEnd() && currentChar() != '\n') {
                            advance();
                        }
                    } else if (source[current + 1] == '*') {
                        // Multi-line comment: skip until */
                        advance();  // Skip '/'
                        advance();  // Skip '*'
                        while (!isAtEnd()) {
                            if (currentChar() == '*' && current + 1 < source.size() && source[current + 1] == '/') {
                                advance();  // Skip '*'
                                advance();  // Skip '/'
                                break;
                            }
                            
                            if (currentChar() == '\n') {
                                line++;
                            }
                            
                            advance();
                        }
                    } else {
                        return;  // This is a division operator, not a comment
                    }
                } else {
                    return;  // Just a single slash at the end
                }
                break;
            default:
                return;
        }
    }
}

bool Lexer::isAtEnd()
{
    return current >= source.size();
}

char Lexer::peek()
{
    if (isAtEnd())
        return '\0';
    return source[current];
}

bool Lexer::match(char expected)
{
    if (isAtEnd())
        return false;
    if (source[current] != expected)
        return false;
    current++;
    return true;
}

void Lexer::addToken(TokenType type)
{
    std::string lexeme = source.substr(current, 1);
    tokens.push_back(Token(type, lexeme, "", line));
    advance();
}

Token Lexer::string()
{
    std::string lexeme;
    advance(); // Skip opening quote
    while (!isAtEnd() && currentChar() != '"')
    {
        lexeme += currentChar();
        advance();
    }
    advance(); // Skip closing quote

    Token token;
    token.type = TokenType::STRING;
    token.lexeme = lexeme;
    return token;
}

Token Lexer::number()
{
    std::string lexeme;

    // Handle negative sign
    if (currentChar() == '-')
    {
        lexeme += '-';
        advance();
    }

    // Collect digits before decimal point
    while (!isAtEnd() && std::isdigit(currentChar()))
    {
        lexeme += currentChar();
        advance();
    }

    // Handle decimal point
    if (!isAtEnd() && currentChar() == '.')
    {
        lexeme += '.';
        advance();

        while (!isAtEnd() && std::isdigit(currentChar()))
        {
            lexeme += currentChar();
            advance();
        }
    }

    Token token;
    token.type = TokenType::NUMBER;
    token.lexeme = lexeme;
    return token;
}

Token Lexer::identifier()
{
    std::string lexeme;
    while (!isAtEnd() && (std::isalnum(currentChar()) || currentChar() == '_'))
    {
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
        if (current + 1 < source.size() && std::isdigit(source[current + 1]))
        {
            // This is a negative number
            tokens.push_back(Token(TokenType::MINUS, "-", "", line));
            advance(); // Move past the minus sign
            tokens.push_back(number());
        }
        else
        {
            addToken(TokenType::MINUS);
        }
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
