#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "ast.h"
#include <vector>
#include <memory>
#include <stdexcept>

class Parser
{
public:
    Parser(const std::vector<Token> &tokens) : tokens(tokens), current(0) {}

    std::vector<std::unique_ptr<Stmt>> parse()
    {
        std::vector<std::unique_ptr<Stmt>> statements;
        while (!isAtEnd())
        {
            try
            {
                statements.push_back(declaration());
            }
            catch (const std::runtime_error &e)
            {
                synchronize();
            }
        }
        return statements;
    }

private:
    const std::vector<Token> &tokens;
    size_t current;

    void synchronize()
    {
        advance();

        while (!isAtEnd())
        {
            if (previous().type == TokenType::SEMICOLON)
                return;

            switch (peek().type)
            {
            case TokenType::CLASS:
            case TokenType::FUN:
            case TokenType::VAR:
            case TokenType::FOR:
            case TokenType::IF:
            case TokenType::WHILE:
            case TokenType::PRINT:
            case TokenType::RETURN:
                return;
            }

            advance();
        }
    }

    Token peek() const
    {
        return tokens[current];
    }

    std::unique_ptr<Stmt> declaration()
    {
        if (match({TokenType::VAR}))
        {
            return varDeclaration();
        }
        return statement();
    }

    std::unique_ptr<Stmt> varDeclaration()
    {
        Token name = consume(TokenType::IDENTIFIER, "Expect variable name.");

        std::unique_ptr<Expr> initializer = nullptr;
        if (match({TokenType::EQUAL}))
        {
            initializer = expression();
        }

        consume(TokenType::SEMICOLON, "Expect ';' after variable declaration.");
        return std::make_unique<VarStmt>(name, std::move(initializer));
    }

    std::unique_ptr<Stmt> statement()
    {
        if (match({TokenType::PRINT}))
        {
            return printStmt();
        }
        throw std::runtime_error("Unexpected token.");
    }

    std::unique_ptr<Stmt> printStmt()
    {
        auto value = expression();
        consume(TokenType::SEMICOLON, "Expect ';' after value.");
        return std::make_unique<PrintStmt>(std::move(value));
    }

    std::unique_ptr<Expr> expression()
    {
        return term();
    }

    std::unique_ptr<Expr> term()
    {
        auto expr = factor();

        while (match({TokenType::PLUS, TokenType::MINUS}))
        {
            Token op = previous();
            auto right = factor();
            expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
        }

        return expr;
    }

    std::unique_ptr<Expr> factor()
    {
        if (match({TokenType::NUMBER}))
        {
            return std::make_unique<NumberExpr>(std::stod(previous().lexeme));
        }

        if (match({TokenType::STRING}))
        {
            return std::make_unique<StringExpr>(previous().lexeme);
        }

        if (match({TokenType::IDENTIFIER}))
        {
            return std::make_unique<VariableExpr>(previous());
        }

        if (match({TokenType::LEFT_PAREN}))
        {
            auto expr = expression();
            consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
            return expr;
        }

        throw std::runtime_error("Unexpected token.");
    }

    bool match(const std::vector<TokenType> &types)
    {
        for (const auto &type : types)
        {
            if (check(type))
            {
                advance();
                return true;
            }
        }
        return false;
    }

    bool check(TokenType type) const
    {
        if (isAtEnd())
            return false;
        return tokens[current].type == type;
    }

    Token advance()
    {
        if (!isAtEnd())
            current++;
        return previous();
    }

    bool isAtEnd() const
    {
        return current >= tokens.size();
    }

    Token previous() const
    {
        return tokens[current - 1];
    }

    Token consume(TokenType type, const std::string &message)
    {
        if (check(type))
            return advance();
        throw std::runtime_error(message);
    }
};

#endif