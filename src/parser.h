// parser.h
#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "ast.h"
#include <vector>
#include <memory>
#include <stdexcept>

class Parser {
public:
    Parser(const std::vector<Token>& tokens) : tokens(tokens), current(0) {}

    std::vector<std::unique_ptr<Stmt>> parse() {
        std::vector<std::unique_ptr<Stmt>> statements;
        while (!isAtEnd()) {
            try {
                auto stmt = declaration();
                if (stmt) {
                    statements.push_back(std::move(stmt));
                }
            } catch (const std::runtime_error& e) {
                synchronize();
            }
        }
        return statements;
    }

private:
    const std::vector<Token>& tokens;
    size_t current;

    std::unique_ptr<Stmt> declaration() {
        if (match({TokenType::VAR})) {
            return varDeclaration();
        }
        return statement();
    }

    std::unique_ptr<Stmt> statement() {
        if (match({TokenType::PRINT})) {
            return printStatement();
        }
        if (match({TokenType::IF})) {
            return ifStatement();
        }
        if (match({TokenType::ELSEIF})) {
            return elseIfStatement();
        }
        if (match({TokenType::INPUT})) {
            return InputStatement();
        }
        throw std::runtime_error("Expect statement.");
    }

    std::unique_ptr<Stmt> ifStatement() {
        consume(TokenType::LEFT_PAREN, "Expect '(' after 'if'.");
        auto condition = expression();
        consume(TokenType::RIGHT_PAREN, "Expect ')' after condition.");

        auto thenBranch = statement();
        std::unique_ptr<Stmt> elseBranch = nullptr;

        if (match({TokenType::ELSE})) {
            elseBranch = statement();
        }

        return std::make_unique<IfStmt>(std::move(condition), std::move(thenBranch), std::move(elseBranch));
    }

    std::unique_ptr<Stmt> elseIfStatement() {
        consume(TokenType::LEFT_PAREN, "Expect '(' after 'elseif'.");
        auto condition = expression();
        consume(TokenType::RIGHT_PAREN, "Expect ')' after condition.");

        auto thenBranch = statement();
        std::unique_ptr<Stmt> elseBranch = nullptr;

        if (match({TokenType::ELSE})) {
            elseBranch = statement();
        }

        return std::make_unique<ElseIfStmt>(std::move(condition), std::move(thenBranch), std::move(elseBranch));
    }

    std::unique_ptr<Stmt> printStatement() {
        auto value = expression();
        consume(TokenType::SEMICOLON, "Expect ';' after value.");
        return std::make_unique<PrintStmt>(std::move(value));
    }

    std::unique_ptr<Stmt> InputStatement() {
        Token variableName = consume(TokenType::IDENTIFIER, "Expect variable name.");
        consume(TokenType::SEMICOLON, "Expect ';' after variable name.");
        return std::make_unique<InputStmt>(variableName);
    }

    std::unique_ptr<Stmt> varDeclaration() {
        Token name = consume(TokenType::IDENTIFIER, "Expect variable name.");
        
        std::unique_ptr<Expr> initializer = nullptr;
        if (match({TokenType::EQUAL})) {
            initializer = expression();
        }
        
        consume(TokenType::SEMICOLON, "Expect ';' after variable declaration.");
        return std::make_unique<VarStmt>(name, std::move(initializer));
    }

    std::unique_ptr<Expr> expression() {
        return equality();
    }

    std::unique_ptr<Expr> equality() {
        auto expr = comparison();

        while (match({TokenType::BANG_EQUAL, TokenType::EQUAL_EQUAL})) {
            Token op = previous();
            auto right = comparison();
            expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
        }

        return expr;
    }

    std::unique_ptr<Expr> comparison() {
        auto expr = term();

        while (match({TokenType::GREATER, TokenType::GREATER_EQUAL,
                     TokenType::LESS, TokenType::LESS_EQUAL})) {
            Token op = previous();
            auto right = term();
            expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
        }

        return expr;
    }

    std::unique_ptr<Expr> term() {
        auto expr = factor();

        while (match({TokenType::MINUS, TokenType::PLUS})) {
            Token op = previous();
            auto right = factor();
            expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
        }

        return expr;
    }

    std::unique_ptr<Expr> factor() {
        auto expr = primary();

        while (match({TokenType::SLASH, TokenType::STAR})) {
            Token op = previous();
            auto right = primary();
            expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
        }

        return expr;
    }

    std::unique_ptr<Expr> primary() {
        if (match({TokenType::NUMBER})) {
            return std::make_unique<NumberExpr>(std::stod(previous().lexeme));
        }
        if (match({TokenType::STRING})) {
            return std::make_unique<StringExpr>(previous().lexeme);
        }
        if (match({TokenType::IDENTIFIER})) {
            return std::make_unique<VariableExpr>(previous());
        }
        if (match({TokenType::LEFT_PAREN})) {
            auto expr = expression();
            consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
            return expr;
        }
        throw std::runtime_error("Expect expression.");
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

    Token consume(TokenType type, const std::string& message) {
        if (check(type)) return advance();
        throw std::runtime_error(message);
    }

    bool check(TokenType type) const {
        if (isAtEnd()) return false;
        return peek().type == type;
    }

    Token advance() {
        if (!isAtEnd()) current++;
        return previous();
    }

    bool isAtEnd() const {
        return peek().type == TokenType::EOF_TOKEN;
    }

    Token peek() const {
        return tokens[current];
    }

    Token previous() const {
        return tokens[current - 1];
    }

    void synchronize() {
        advance();

        while (!isAtEnd()) {
            if (previous().type == TokenType::SEMICOLON) return;

            switch (peek().type) {
                case TokenType::CLASS:
                case TokenType::FUN:
                case TokenType::VAR:
                case TokenType::FOR:
                case TokenType::IF:
                case TokenType::WHILE:
                case TokenType::PRINT:
                case TokenType::RETURN:
                    return;
                default:
                    advance();
            }
        }
    }
};

#endif