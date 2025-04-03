// parser.h
#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "ast.h"
#include <vector>
#include <memory>
#include <stdexcept>
#include <iostream>

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
                auto stmt = declaration();
                if (stmt)
                {
                    statements.push_back(std::move(stmt));
                }
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

    std::unique_ptr<Stmt> declaration()
    {
        if (match({TokenType::VAR}))
        {
            return varDeclaration();
        }
        return statement();
    }

    std::unique_ptr<Stmt> statement()
    {
        if (match({TokenType::PRINT}))
        {
            return printStatement();
        }
        if (match({TokenType::INPUT}))
        {
            return InputStatement();
        }
        if (match({TokenType::LOOP}))
        {
            return loopStatement();
        }

        if (match({TokenType::BREAK}))
        {
            consume(TokenType::SEMICOLON, "Expect ';' after 'break'.");
            return std::make_unique<BreakStmt>();
        }
        if (match({TokenType::CONTINUE}))
        {
            consume(TokenType::SEMICOLON, "Expect ';' after 'continue'.");
            return std::make_unique<ContinueStmt>();
        }

        if (match({TokenType::LEFT_BRACE}))
        {
            std::vector<std::unique_ptr<Stmt>> statements;
            while (!check(TokenType::RIGHT_BRACE) && !isAtEnd())
            {
                statements.push_back(declaration());
            }
            consume(TokenType::RIGHT_BRACE, "Expect '}' after block.");
            return std::make_unique<BlockStmt>(std::move(statements));
        }

        if (match({TokenType::COMPEQ})) {
            consume(TokenType::LEFT_PAREN, "Expect '(' after 'compeq'.");
            auto leftExpr = expression();
            consume(TokenType::COMMA, "Expect ',' after left operand.");
            auto rightExpr = expression();
            consume(TokenType::RIGHT_PAREN, "Expect ')' after right operand.");
            
            // First check for AND
            if (check(TokenType::AND)) {
                auto andStmt = handleAND(std::move(leftExpr), std::move(rightExpr), TokenType::COMPEQ);
                return andStmt;
            }
            
            // Then check for OR
            if (check(TokenType::OR)) {
                auto orStmt = handleOR(std::move(leftExpr), std::move(rightExpr), TokenType::COMPEQ);
                return orStmt;
            }
            
            // Regular compeq statement
            auto thenBranch = statement();
            std::unique_ptr<Stmt> elseBranch = nullptr;
            if (match({TokenType::ELSE})) {
                if (peek().type == TokenType::IF) {
                    advance(); // consume IF
                    if (match({TokenType::COMPEQ})) {
                        elseBranch = compEqStatement();
                    }
                } else {
                    elseBranch = statement();
                }
            }
            
            return std::make_unique<CompEqStmt>(
                std::move(leftExpr), 
                std::move(rightExpr), 
                std::move(thenBranch), 
                std::move(elseBranch)
            );
        }
        if (match({TokenType::COMPNEQ})) {
            consume(TokenType::LEFT_PAREN, "Expect '(' after 'compneq'.");
            auto leftExpr = expression();
            consume(TokenType::COMMA, "Expect ',' after left operand.");
            auto rightExpr = expression();
            consume(TokenType::RIGHT_PAREN, "Expect ')' after right operand.");
            
            // First check for AND
            auto andStmt = handleAND(std::move(leftExpr), std::move(rightExpr), TokenType::COMPNEQ);
            if (andStmt) return andStmt;
            
            // Then check for OR
            if (check(TokenType::OR)) {
                auto orStmt = handleOR(std::move(leftExpr), std::move(rightExpr), TokenType::COMPNEQ);
                return orStmt;
            }
            
            // If not part of AND or OR, continue with regular compneq
            return compNeqStatementBody(std::move(leftExpr), std::move(rightExpr));
        }
        if (match({TokenType::COMPGE})) {
            consume(TokenType::LEFT_PAREN, "Expect '(' after 'compge'.");
            auto leftExpr = expression();
            consume(TokenType::COMMA, "Expect ',' after left operand.");
            auto rightExpr = expression();
            consume(TokenType::RIGHT_PAREN, "Expect ')' after right operand.");
            
            // First check for AND
            auto andStmt = handleAND(std::move(leftExpr), std::move(rightExpr), TokenType::COMPGE);
            if (andStmt) return andStmt;
            
            // Then check for OR
            if (check(TokenType::OR)) {
                auto orStmt = handleOR(std::move(leftExpr), std::move(rightExpr), TokenType::COMPGE);
                return orStmt;
            }
            
            // If not part of AND or OR, continue with regular compge
            return compGeStatementBody(std::move(leftExpr), std::move(rightExpr));
        }
        if (match({TokenType::COMPLE})) {
            consume(TokenType::LEFT_PAREN, "Expect '(' after 'comple'.");
            auto leftExpr = expression();
            consume(TokenType::COMMA, "Expect ',' after left operand.");
            auto rightExpr = expression();
            consume(TokenType::RIGHT_PAREN, "Expect ')' after right operand.");
            
            // First check for AND
            auto andStmt = handleAND(std::move(leftExpr), std::move(rightExpr), TokenType::COMPLE);
            if (andStmt) return andStmt;
            
            // Then check for OR
            if (check(TokenType::OR)) {
                auto orStmt = handleOR(std::move(leftExpr), std::move(rightExpr), TokenType::COMPLE);
                return orStmt;
            }
            
            // If not part of AND or OR, continue with regular comple
            return compLeStatementBody(std::move(leftExpr), std::move(rightExpr));
        }
        if (match({TokenType::COMPG})) {
            consume(TokenType::LEFT_PAREN, "Expect '(' after 'compg'.");
            auto leftExpr = expression();
            consume(TokenType::COMMA, "Expect ',' after left operand.");
            auto rightExpr = expression();
            consume(TokenType::RIGHT_PAREN, "Expect ')' after right operand.");
            
            // First check for AND
            auto andStmt = handleAND(std::move(leftExpr), std::move(rightExpr), TokenType::COMPG);
            if (andStmt) return andStmt;
            
            // Then check for OR
            if (check(TokenType::OR)) {
                auto orStmt = handleOR(std::move(leftExpr), std::move(rightExpr), TokenType::COMPG);
                return orStmt;
            }
            
            // If not part of AND or OR, continue with regular compg
            return compGStatementBody(std::move(leftExpr), std::move(rightExpr));
        }
        if (match({TokenType::COMPL})) {
            consume(TokenType::LEFT_PAREN, "Expect '(' after 'compl'.");
            auto leftExpr = expression();
            consume(TokenType::COMMA, "Expect ',' after left operand.");
            auto rightExpr = expression();
            consume(TokenType::RIGHT_PAREN, "Expect ')' after right operand.");
            
            // First check for AND
            auto andStmt = handleAND(std::move(leftExpr), std::move(rightExpr), TokenType::COMPL);
            if (andStmt) return andStmt;
            
            // Then check for OR
            if (check(TokenType::OR)) {
                auto orStmt = handleOR(std::move(leftExpr), std::move(rightExpr), TokenType::COMPL);
                return orStmt;
            }
            
            // If not part of AND or OR, continue with regular compl
            return compLStatementBody(std::move(leftExpr), std::move(rightExpr));
        }

        return expressionStatement();
    }

    std::unique_ptr<Stmt> expressionStatement()
    {
        auto expr = expression();
        consume(TokenType::SEMICOLON, "Expect ';' after expression.");
        return std::make_unique<ExpressionStmt>(std::move(expr));
    }

    std::unique_ptr<Stmt> compEqStatement() {
        consume(TokenType::LEFT_PAREN, "Expect '(' after 'compeq'.");
        auto left = expression();
        consume(TokenType::COMMA, "Expect ',' after left operand.");
        auto right = expression();
        consume(TokenType::RIGHT_PAREN, "Expect ')' after right operand.");

        auto thenBranch = statement();
        std::unique_ptr<Stmt> elseBranch = nullptr;
        
        // Handle else if and else
        if (match({TokenType::ELSE})) {
            if (peek().type == TokenType::IF) {
                advance(); // consume IF
                if (match({TokenType::COMPEQ})) {
                    elseBranch = compEqStatement();
                }
            } else {
                elseBranch = statement();
            }
        }
        
        return std::make_unique<CompEqStmt>(
            std::move(left), 
            std::move(right), 
            std::move(thenBranch), 
            std::move(elseBranch)
        );
    }

    std::unique_ptr<Stmt> compNeqStatement() {
        consume(TokenType::LEFT_PAREN, "Expect '(' after 'compneq'.");
        auto left = expression();
        consume(TokenType::COMMA, "Expect ',' after left operand.");
        auto right = expression();
        consume(TokenType::RIGHT_PAREN, "Expect ')' after right operand.");
        
        auto thenBranch = statement();
        std::vector<std::pair<std::unique_ptr<Expr>, std::unique_ptr<Stmt>>> elseIfBranches;
        std::unique_ptr<Stmt> elseBranch = nullptr;
        
        while (match({TokenType::ELSE})) {
            if (peek().type == TokenType::IF) {
                advance(); // consume 'if'
                if (match({TokenType::COMPEQ})) {
                    consume(TokenType::LEFT_PAREN, "Expect '(' after 'compeq'.");
                    auto elseIfLeft = expression();
                    consume(TokenType::COMMA, "Expect ',' after left operand.");
                    auto elseIfRight = expression();
                    consume(TokenType::RIGHT_PAREN, "Expect ')' after right operand.");
                    auto elseIfBranch = statement();
                    
                    auto condition = std::make_unique<CompEqExpr>(std::move(elseIfLeft), std::move(elseIfRight));
                    elseIfBranches.push_back(std::make_pair(std::move(condition), std::move(elseIfBranch)));
                }
            } else {
                elseBranch = statement();
                break;
            }
        }
        
        return std::make_unique<CompNeqStmt>(
            std::move(left), 
            std::move(right), 
            std::move(thenBranch),
            std::move(elseIfBranches),
            std::move(elseBranch)
        );
    }

    std::unique_ptr<Stmt> compGeStatement() {
        consume(TokenType::LEFT_PAREN, "Expect '(' after 'compge'.");
        auto left = expression();
        consume(TokenType::COMMA, "Expect ',' after left operand.");
        auto right = expression();
        consume(TokenType::RIGHT_PAREN, "Expect ')' after right operand.");
        
        auto thenBranch = statement();
        std::vector<std::pair<std::unique_ptr<Expr>, std::unique_ptr<Stmt>>> elseIfBranches;
        std::unique_ptr<Stmt> elseBranch = nullptr;
        
        while (match({TokenType::ELSE})) {
            if (peek().type == TokenType::IF) {
                advance(); // consume 'if'
                if (match({TokenType::COMPEQ})) {
                    consume(TokenType::LEFT_PAREN, "Expect '(' after 'compeq'.");
                    auto elseIfLeft = expression();
                    consume(TokenType::COMMA, "Expect ',' after left operand.");
                    auto elseIfRight = expression();
                    consume(TokenType::RIGHT_PAREN, "Expect ')' after right operand.");
                    auto elseIfBranch = statement();
                    
                    auto condition = std::make_unique<CompEqExpr>(std::move(elseIfLeft), std::move(elseIfRight));
                    elseIfBranches.push_back(std::make_pair(std::move(condition), std::move(elseIfBranch)));
                }
            } else {
                elseBranch = statement();
                break;
            }
        }
        
        return std::make_unique<CompGeStmt>(
            std::move(left), 
            std::move(right), 
            std::move(thenBranch),
            std::move(elseIfBranches),
            std::move(elseBranch)
        );
    }

    std::unique_ptr<Stmt> compLeStatement() {
        consume(TokenType::LEFT_PAREN, "Expect '(' after 'comple'.");
        auto left = expression();
        consume(TokenType::COMMA, "Expect ',' after left operand.");
        auto right = expression();
        consume(TokenType::RIGHT_PAREN, "Expect ')' after right operand.");
        
        auto thenBranch = statement();
        std::vector<std::pair<std::unique_ptr<Expr>, std::unique_ptr<Stmt>>> elseIfBranches;
        std::unique_ptr<Stmt> elseBranch = nullptr;
        
        while (match({TokenType::ELSE})) {
            if (peek().type == TokenType::IF) {
                advance(); // consume 'if'
                if (match({TokenType::COMPEQ})) {
                    consume(TokenType::LEFT_PAREN, "Expect '(' after 'compeq'.");
                    auto elseIfLeft = expression();
                    consume(TokenType::COMMA, "Expect ',' after left operand.");
                    auto elseIfRight = expression();
                    consume(TokenType::RIGHT_PAREN, "Expect ')' after right operand.");
                    auto elseIfBranch = statement();
                    
                    auto condition = std::make_unique<CompEqExpr>(std::move(elseIfLeft), std::move(elseIfRight));
                    elseIfBranches.push_back(std::make_pair(std::move(condition), std::move(elseIfBranch)));
                }
            } else {
                elseBranch = statement();
                break;
            }
        }
        
        return std::make_unique<CompLeStmt>(
            std::move(left), 
            std::move(right), 
            std::move(thenBranch),
            std::move(elseIfBranches),
            std::move(elseBranch)
        );
    }

    std::unique_ptr<Stmt> compGStatement() {
        consume(TokenType::LEFT_PAREN, "Expect '(' after 'compg'.");
        auto left = expression();
        consume(TokenType::COMMA, "Expect ',' after left operand.");
        auto right = expression();
        consume(TokenType::RIGHT_PAREN, "Expect ')' after right operand.");
        
        auto thenBranch = statement();
        std::vector<std::pair<std::unique_ptr<Expr>, std::unique_ptr<Stmt>>> elseIfBranches;
        std::unique_ptr<Stmt> elseBranch = nullptr;
        
        while (match({TokenType::ELSE})) {
            // Check for 'else if'
            if (peek().type == TokenType::IF) {
                advance(); // consume 'if'
                if (match({TokenType::COMPEQ})) {
                    consume(TokenType::LEFT_PAREN, "Expect '(' after 'compeq'.");
                    auto elseIfLeft = expression();
                    consume(TokenType::COMMA, "Expect ',' after left operand.");
                    auto elseIfRight = expression();
                    consume(TokenType::RIGHT_PAREN, "Expect ')' after right operand.");
                    auto elseIfBranch = statement();
                    
                    auto condition = std::make_unique<CompEqExpr>(std::move(elseIfLeft), std::move(elseIfRight));
                    elseIfBranches.push_back(std::make_pair(std::move(condition), std::move(elseIfBranch)));
                }
            } else {
                elseBranch = statement();
                break;
            }
        }
        
        return std::make_unique<CompGStmt>(
            std::move(left), 
            std::move(right), 
            std::move(thenBranch),
            std::move(elseIfBranches),
            std::move(elseBranch)
        );
    }

    std::unique_ptr<Stmt> compLStatement() {
        consume(TokenType::LEFT_PAREN, "Expect '(' after 'compl'.");
        auto left = expression();
        consume(TokenType::COMMA, "Expect ',' after left operand.");
        auto right = expression();
        consume(TokenType::RIGHT_PAREN, "Expect ')' after right operand.");
        
        auto thenBranch = statement();
        std::vector<std::pair<std::unique_ptr<Expr>, std::unique_ptr<Stmt>>> elseIfBranches;
        std::unique_ptr<Stmt> elseBranch = nullptr;
        
        while (match({TokenType::ELSE})) {
            if (peek().type == TokenType::IF) {
                advance(); // consume 'if'
                if (match({TokenType::COMPEQ})) {
                    consume(TokenType::LEFT_PAREN, "Expect '(' after 'compeq'.");
                    auto elseIfLeft = expression();
                    consume(TokenType::COMMA, "Expect ',' after left operand.");
                    auto elseIfRight = expression();
                    consume(TokenType::RIGHT_PAREN, "Expect ')' after right operand.");
                    auto elseIfBranch = statement();
                    
                    auto condition = std::make_unique<CompEqExpr>(std::move(elseIfLeft), std::move(elseIfRight));
                    elseIfBranches.push_back(std::make_pair(std::move(condition), std::move(elseIfBranch)));
                }
            } else {
                elseBranch = statement();
                break;
            }
        }
        
        return std::make_unique<CompLStmt>(
            std::move(left), 
            std::move(right), 
            std::move(thenBranch),
            std::move(elseIfBranches),
            std::move(elseBranch)
        );
    }

    std::unique_ptr<Stmt> printStatement()
    {
        auto value = expression();
        consume(TokenType::SEMICOLON, "Expect ';' after value.");
        return std::make_unique<PrintStmt>(std::move(value));
    }

    std::unique_ptr<Stmt> InputStatement()
    {
        Token variableName = consume(TokenType::IDENTIFIER, "Expect variable name.");
        consume(TokenType::SEMICOLON, "Expect ';' after variable name.");
        return std::make_unique<InputStmt>(variableName);
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

    std::unique_ptr<Expr> expression()
    {
        return equality();
    }

    std::unique_ptr<Expr> equality()
    {
        auto expr = comparison();

        while (match({TokenType::BANG_EQUAL, TokenType::EQUAL_EQUAL}))
        {
            Token op = previous();
            auto right = comparison();
            expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
        }

        if (match({TokenType::COMPEQ})) {
            auto left = std::move(expr);
            auto right = comparison();
            expr = std::make_unique<CompEqExpr>(std::move(left), std::move(right));
        }

        return expr;
    }

    std::unique_ptr<Expr> comparison()
    {
        auto expr = term();

        while (match({TokenType::GREATER, TokenType::GREATER_EQUAL,
                      TokenType::LESS, TokenType::LESS_EQUAL}))
        {
            Token op = previous();
            auto right = term();
            expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
        }

        return expr;
    }

    std::unique_ptr<Expr> term()
    {
        auto expr = factor();

        while (match({TokenType::MINUS, TokenType::PLUS}))
        {
            Token op = previous();
            auto right = factor();
            expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
        }

        return expr;
    }

    std::unique_ptr<Expr> factor()
    {
        auto expr = primary();

        while (match({TokenType::SLASH, TokenType::STAR}))
        {
            Token op = previous();
            auto right = primary();
            expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
        }

        return expr;
    }

    std::unique_ptr<Expr> primary()
    {
        if (match({TokenType::MINUS})) {
            if (match({TokenType::NUMBER})) {
                std::string numStr = "-" + previous().lexeme;
                return std::make_unique<NumberExpr>(std::stod(numStr));
            }
            throw std::runtime_error("Expected number after minus sign.");
        }
        
        if (match({TokenType::NUMBER})) {
            try {
                return std::make_unique<NumberExpr>(std::stod(previous().lexeme));
            } catch (const std::exception& e) {
                throw std::runtime_error("Invalid number format");
            }
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
        throw std::runtime_error("Expect expression.");
    }

    std::unique_ptr<Stmt> loopStatement()
    {
        Token var = consume(TokenType::IDENTIFIER, "Expect variable name after 'loop'.");
        consume(TokenType::EQUAL, "Expect '=' after variable name.");

        auto from = expression();
        consume(TokenType::TO, "Expect 'to' after start value.");
        auto to = expression();

        bool isDownward = match({TokenType::DOWN});

        std::unique_ptr<Expr> step = nullptr;
        if (match({TokenType::STEP}))
        {
            step = expression();
        }

        std::unique_ptr<Stmt> body;
        if (match({TokenType::LEFT_BRACE}))
        {
            std::vector<std::unique_ptr<Stmt>> statements;
            while (!check(TokenType::RIGHT_BRACE) && !isAtEnd())
            {
                statements.push_back(declaration());
            }
            consume(TokenType::RIGHT_BRACE, "Expect '}' after loop body.");
            body = std::make_unique<BlockStmt>(std::move(statements));
        }
        else
        {
            body = statement();
        }

        return std::make_unique<LoopStmt>(var, std::move(from), std::move(to),
                                          std::move(step), std::move(body), isDownward);
    }

    std::unique_ptr<Stmt> parseLogicalExpression() {
        std::vector<std::unique_ptr<Stmt>> conditions;
        
        conditions.push_back(parseCondition());
        
        bool isAnd = match({TokenType::AND});
        bool isOr = match({TokenType::OR});
        
        while (isAnd || isOr) {
            conditions.push_back(parseCondition());
            isAnd = match({TokenType::AND});
            isOr = match({TokenType::OR});
        }
        
        auto thenBranch = statement();
        
        if (isAnd) {
            return std::make_unique<AndConditionStmt>(std::move(conditions), std::move(thenBranch));
        } else if (isOr) {
            return std::make_unique<OrConditionStmt>(std::move(conditions), std::move(thenBranch));
        }
        
        return std::move(conditions[0]);
    }

    std::unique_ptr<Stmt> parseCondition() {
        if (match({TokenType::COMPEQ})) {
            return compEqStatement();
        }
        throw std::runtime_error("Expected condition");
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

    Token consume(TokenType type, const std::string &message)
    {
        if (check(type))
            return advance();
        throw std::runtime_error(message);
    }

    bool check(TokenType type) const
    {
        if (isAtEnd())
            return false;
        return peek().type == type;
    }

    Token advance()
    {
        if (!isAtEnd())
            current++;
        return previous();
    }

    bool isAtEnd() const
    {
        return peek().type == TokenType::EOF_TOKEN;
    }

    Token peek() const
    {
        return tokens[current];
    }

    Token previous() const
    {
        return tokens[current - 1];
    }

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
            default:
                advance();
            }
        }
    }

    // Add a more generic handleLogicalOperators method to handle both AND and OR
    std::unique_ptr<Stmt> handleLogicalOperator(std::unique_ptr<Expr> leftExpr, std::unique_ptr<Expr> rightExpr, 
                                               TokenType opType, TokenType logicalOp) {
        if (check(logicalOp)) {
            advance(); // consume AND or OR
            
            // Create first condition
            std::vector<std::unique_ptr<Stmt>> conditions;
            conditions.push_back(std::make_unique<ExpressionStmt>(
                std::make_unique<CompEqExpr>(std::move(leftExpr), std::move(rightExpr))
            ));
            
            // Parse next comparison
            if (match({TokenType::COMPEQ})) {
                consume(TokenType::LEFT_PAREN, "Expect '(' after comparison operator");
                auto nextLeft = expression();
                consume(TokenType::COMMA, "Expect ',' after left operand");
                auto nextRight = expression();
                consume(TokenType::RIGHT_PAREN, "Expect ')' after right operand");
                
                // Add second condition
                conditions.push_back(std::make_unique<ExpressionStmt>(
                    std::make_unique<CompEqExpr>(std::move(nextLeft), std::move(nextRight))
                ));
                
                // Handle any additional conditions
                while (match({logicalOp})) {
                    if (match({TokenType::COMPEQ})) {
                        consume(TokenType::LEFT_PAREN, "Expect '(' after comparison operator");
                        auto nextLeft = expression();
                        consume(TokenType::COMMA, "Expect ',' after left operand");
                        auto nextRight = expression();
                        consume(TokenType::RIGHT_PAREN, "Expect ')' after right operand");
                        
                        conditions.push_back(std::make_unique<ExpressionStmt>(
                            std::make_unique<CompEqExpr>(std::move(nextLeft), std::move(nextRight))
                        ));
                    }
                }
            }
            
            auto thenBranch = statement();
            std::unique_ptr<Stmt> elseBranch = nullptr;
            if (match({TokenType::ELSE})) {
                elseBranch = statement();
            }
            
            if (logicalOp == TokenType::AND) {
                return std::make_unique<AndConditionStmt>(
                    std::move(conditions),
                    std::move(thenBranch),
                    std::move(elseBranch)
                );
            } else {
                return std::make_unique<OrConditionStmt>(
                    std::move(conditions),
                    std::move(thenBranch),
                    std::move(elseBranch)
                );
            }
        }
        return nullptr;
    }

    // Simplified wrapper methods for specific logical operators
    std::unique_ptr<Stmt> handleAND(std::unique_ptr<Expr> leftExpr, std::unique_ptr<Expr> rightExpr, TokenType opType) {
        return handleLogicalOperator(std::move(leftExpr), std::move(rightExpr), opType, TokenType::AND);
    }
    
    std::unique_ptr<Stmt> handleOR(std::unique_ptr<Expr> leftExpr, std::unique_ptr<Expr> rightExpr, TokenType opType) {
        return handleLogicalOperator(std::move(leftExpr), std::move(rightExpr), opType, TokenType::OR);
    }

    // Helper methods for the statement body of each comparison type
    std::unique_ptr<Stmt> compNeqStatementBody(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right) {
        auto thenBranch = statement();
        std::vector<std::pair<std::unique_ptr<Expr>, std::unique_ptr<Stmt>>> elseIfBranches;
        std::unique_ptr<Stmt> elseBranch = nullptr;
        
        // Handle else-if and else branches
        while (match({TokenType::ELSE})) {
            if (peek().type == TokenType::IF) {
                advance(); // consume 'if'
                if (match({TokenType::COMPEQ})) {
                    consume(TokenType::LEFT_PAREN, "Expect '(' after 'compeq'.");
                    auto elseIfLeft = expression();
                    consume(TokenType::COMMA, "Expect ',' after left operand.");
                    auto elseIfRight = expression();
                    consume(TokenType::RIGHT_PAREN, "Expect ')' after right operand.");
                    auto elseIfBranch = statement();
                    
                    auto condition = std::make_unique<CompEqExpr>(std::move(elseIfLeft), std::move(elseIfRight));
                    elseIfBranches.push_back(std::make_pair(std::move(condition), std::move(elseIfBranch)));
                }
            } else {
                elseBranch = statement();
                break;
            }
        }
        
        return std::make_unique<CompNeqStmt>(
            std::move(left), 
            std::move(right), 
            std::move(thenBranch),
            std::move(elseIfBranches),
            std::move(elseBranch)
        );
    }

    std::unique_ptr<Stmt> compGeStatementBody(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right) {
        auto thenBranch = statement();
        std::vector<std::pair<std::unique_ptr<Expr>, std::unique_ptr<Stmt>>> elseIfBranches;
        std::unique_ptr<Stmt> elseBranch = nullptr;
        
        // Handle else-if and else branches
        while (match({TokenType::ELSE})) {
            if (peek().type == TokenType::IF) {
                advance(); // consume 'if'
                if (match({TokenType::COMPEQ})) {
                    consume(TokenType::LEFT_PAREN, "Expect '(' after 'compeq'.");
                    auto elseIfLeft = expression();
                    consume(TokenType::COMMA, "Expect ',' after left operand.");
                    auto elseIfRight = expression();
                    consume(TokenType::RIGHT_PAREN, "Expect ')' after right operand.");
                    auto elseIfBranch = statement();
                    
                    auto condition = std::make_unique<CompEqExpr>(std::move(elseIfLeft), std::move(elseIfRight));
                    elseIfBranches.push_back(std::make_pair(std::move(condition), std::move(elseIfBranch)));
                }
            } else {
                elseBranch = statement();
                break;
            }
        }
        
        return std::make_unique<CompGeStmt>(
            std::move(left), 
            std::move(right), 
            std::move(thenBranch),
            std::move(elseIfBranches),
            std::move(elseBranch)
        );
    }

    std::unique_ptr<Stmt> compLeStatementBody(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right) {
        auto thenBranch = statement();
        std::vector<std::pair<std::unique_ptr<Expr>, std::unique_ptr<Stmt>>> elseIfBranches;
        std::unique_ptr<Stmt> elseBranch = nullptr;
        
        // Handle else-if and else branches
        while (match({TokenType::ELSE})) {
            if (peek().type == TokenType::IF) {
                advance(); // consume 'if'
                if (match({TokenType::COMPEQ})) {
                    consume(TokenType::LEFT_PAREN, "Expect '(' after 'compeq'.");
                    auto elseIfLeft = expression();
                    consume(TokenType::COMMA, "Expect ',' after left operand.");
                    auto elseIfRight = expression();
                    consume(TokenType::RIGHT_PAREN, "Expect ')' after right operand.");
                    auto elseIfBranch = statement();
                    
                    auto condition = std::make_unique<CompEqExpr>(std::move(elseIfLeft), std::move(elseIfRight));
                    elseIfBranches.push_back(std::make_pair(std::move(condition), std::move(elseIfBranch)));
                }
            } else {
                elseBranch = statement();
                break;
            }
        }
        
        return std::make_unique<CompLeStmt>(
            std::move(left), 
            std::move(right), 
            std::move(thenBranch),
            std::move(elseIfBranches),
            std::move(elseBranch)
        );
    }

    std::unique_ptr<Stmt> compGStatementBody(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right) {
        auto thenBranch = statement();
        std::vector<std::pair<std::unique_ptr<Expr>, std::unique_ptr<Stmt>>> elseIfBranches;
        std::unique_ptr<Stmt> elseBranch = nullptr;
        
        // Handle else-if and else branches
        while (match({TokenType::ELSE})) {
            // Check for 'else if'
            if (peek().type == TokenType::IF) {
                advance(); // consume 'if'
                if (match({TokenType::COMPEQ})) {
                    consume(TokenType::LEFT_PAREN, "Expect '(' after 'compeq'.");
                    auto elseIfLeft = expression();
                    consume(TokenType::COMMA, "Expect ',' after left operand.");
                    auto elseIfRight = expression();
                    consume(TokenType::RIGHT_PAREN, "Expect ')' after right operand.");
                    auto elseIfBranch = statement();
                    
                    auto condition = std::make_unique<CompEqExpr>(std::move(elseIfLeft), std::move(elseIfRight));
                    elseIfBranches.push_back(std::make_pair(std::move(condition), std::move(elseIfBranch)));
                }
            } else {
                elseBranch = statement();
                break;
            }
        }
        
        return std::make_unique<CompGStmt>(
            std::move(left), 
            std::move(right), 
            std::move(thenBranch),
            std::move(elseIfBranches),
            std::move(elseBranch)
        );
    }

    std::unique_ptr<Stmt> compLStatementBody(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right) {
        auto thenBranch = statement();
        std::vector<std::pair<std::unique_ptr<Expr>, std::unique_ptr<Stmt>>> elseIfBranches;
        std::unique_ptr<Stmt> elseBranch = nullptr;
        
        // Handle else-if and else branches
        while (match({TokenType::ELSE})) {
            if (peek().type == TokenType::IF) {
                advance(); // consume 'if'
                if (match({TokenType::COMPEQ})) {
                    consume(TokenType::LEFT_PAREN, "Expect '(' after 'compeq'.");
                    auto elseIfLeft = expression();
                    consume(TokenType::COMMA, "Expect ',' after left operand.");
                    auto elseIfRight = expression();
                    consume(TokenType::RIGHT_PAREN, "Expect ')' after right operand.");
                    auto elseIfBranch = statement();
                    
                    auto condition = std::make_unique<CompEqExpr>(std::move(elseIfLeft), std::move(elseIfRight));
                    elseIfBranches.push_back(std::make_pair(std::move(condition), std::move(elseIfBranch)));
                }
            } else {
                elseBranch = statement();
                break;
            }
        }
        
        return std::make_unique<CompLStmt>(
            std::move(left), 
            std::move(right), 
            std::move(thenBranch),
            std::move(elseIfBranches),
            std::move(elseBranch)
        );
    }
};

#endif