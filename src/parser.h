#pragma once

#include "lexer.h"
#include "ast.h"
#include <vector>
#include <memory>
#include <stdexcept>

class Parser {
public:
    Parser(const std::vector<Token>& tokens) : tokens(tokens), current(0) {}

    std::unique_ptr<Expr> parse() {
        return statement();
    }

private:
    const std::vector<Token>& tokens;
    size_t current;

    std::unique_ptr<Expr> statement() {
        if (match({TokenType::IDENTIFIER}) && previous().lexeme == "print") {
            return printStmt();
        }
        throw std::runtime_error("Unexpected token.");
    }

    std::unique_ptr<Expr> printStmt() {
        auto value = expression();
        return std::make_unique<PrintStmt>(std::move(value));
    }

    std::unique_ptr<Expr> expression() {
        return term();
    }

    std::unique_ptr<Expr> term() {
        auto expr = factor();

        while (match({TokenType::PLUS, TokenType::MINUS})) {
            Token op = previous();
            auto right = factor();
            expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
        }

        return expr;
    }

    std::unique_ptr<Expr> factor() {
        if (match({TokenType::NUMBER})) {
            return std::make_unique<NumberExpr>(std::stod(previous().lexeme));
        }

        if (match({TokenType::STRING})) {
            return std::make_unique<StringExpr>(previous().lexeme);
        }

        if (match({TokenType::LEFT_PAREN})) {
            auto expr = expression();
            consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
            return expr;
        }

        throw std::runtime_error("Unexpected token.");
    }

    bool match(const std::vector<TokenType>& types) {
        for (const auto& type : types) {
            if (check(type)) {
                advance();
                return true;
            }
        }
        return false;
    }

    bool check(TokenType type) const {
        if (isAtEnd()) return false;
        return tokens[current].type == type;
    }

    Token advance() {
        if (!isAtEnd()) current++;
        return previous();
    }

    bool isAtEnd() const {
        return current >= tokens.size();
    }

    Token previous() const {
        return tokens[current - 1];
    }

    Token consume(TokenType type, const std::string& message) {
        if (check(type)) return advance();
        throw std::runtime_error(message);
    }
};