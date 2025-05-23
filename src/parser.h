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
        if (match({TokenType::FUN})) {
            return functionDeclaration("function");
        }
        if (match({TokenType::VAR}))
        {
            return varDeclaration();
        }
        return statement();
    }

    std::unique_ptr<Stmt> functionDeclaration(const std::string& kind) {
        Token name = consume(TokenType::IDENTIFIER, "Expect " + kind + " name.");
        consume(TokenType::LEFT_PAREN, "Expect '(' after " + kind + " name.");
        
        std::vector<Token> parameters;
        if (!check(TokenType::RIGHT_PAREN)) {
            do {
                if (parameters.size() >= 255) {
                    error(peek(), "Cannot have more than 255 parameters.");
                }
                parameters.push_back(consume(TokenType::IDENTIFIER, "Expect parameter name."));
            } while (match({TokenType::COMMA}));
        }
        
        consume(TokenType::RIGHT_PAREN, "Expect ')' after parameters.");
        
        consume(TokenType::LEFT_BRACE, "Expect '{' before " + kind + " body.");
        auto body = block();
        
        std::vector<std::unique_ptr<Stmt>> functionBody;
        for (auto& stmt : dynamic_cast<BlockStmt*>(body.get())->statements) {
            functionBody.push_back(std::move(stmt));
        }
        
        return std::make_unique<FunctionStmt>(name, parameters, std::move(functionBody));
    }

    std::unique_ptr<Stmt> statement()
    {
        if (match({TokenType::RETURN_KW})) {
            return returnStatement();
        }
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
            return block();
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

    std::unique_ptr<Stmt> block() {
        std::vector<std::unique_ptr<Stmt>> statements;
        while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
            statements.push_back(declaration());
        }
        consume(TokenType::RIGHT_BRACE, "Expect '}' after block.");
        return std::make_unique<BlockStmt>(std::move(statements));
    }

    std::unique_ptr<Stmt> returnStatement() {
        Token keyword = previous();
        std::unique_ptr<Expr> value = nullptr;
        
        if (!check(TokenType::SEMICOLON)) {
            value = expression();
        }
        
        consume(TokenType::SEMICOLON, "Expect ';' after return value.");
        return std::make_unique<ReturnStmt>(keyword, std::move(value));
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

                // Handle AND/OR conditions in else-if
                auto condition = handleElseIfCondition();
                if (condition) {
                    elseIfBranches.push_back(std::make_pair(std::move(condition), statement()));
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

    std::unique_ptr<Expr> handleElseIfCondition() {
        if (match({TokenType::COMPEQ})) {
            consume(TokenType::LEFT_PAREN, "Expect '(' after comparison operator");
            auto left = expression();
            consume(TokenType::COMMA, "Expect ',' after left operand");
            auto right = expression();
            consume(TokenType::RIGHT_PAREN, "Expect ')' after right operand");

            auto condition = std::make_unique<CompEqExpr>(std::move(left), std::move(right));

            // Check for AND/OR
            if (check(TokenType::AND) || check(TokenType::OR)) {
                TokenType logicalOp = peek().type;
                advance(); // consume AND/OR

                auto nextCondition = handleElseIfCondition();
                // Create a binary expression joining the conditions
                auto combinedCondition = std::make_unique<CompEqExpr>(
                    std::move(condition),
                    std::move(nextCondition)
                );
                return combinedCondition;
            }

            return condition;
        }
        return nullptr;
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
        return assignment();
    }

    std::unique_ptr<Expr> assignment() {
        auto expr = equality();

        if (match({TokenType::EQUAL})) {
            auto equals = previous();
            
            // Check for fixed-size array initialization pattern: array[5] = {1, 2, 3, 4, 5}
            if (auto* indexExpr = dynamic_cast<IndexExpr*>(expr.get())) {
                // Check for array literal init with curly braces
                if (check(TokenType::LEFT_CURLY)) {
                    advance(); // consume '{'
                    
                    std::vector<std::unique_ptr<Expr>> elements;
                    if (!check(TokenType::RIGHT_CURLY)) {
                        do {
                            elements.push_back(expression());
                        } while (match({TokenType::COMMA}));
                    }
                    
                    consume(TokenType::RIGHT_CURLY, "Expect '}' after array elements.");
                    
                    // Extract size from the index expression
                    int arraySize = 0;
                    if (auto* numExpr = dynamic_cast<NumberExpr*>(indexExpr->index.get())) {
                        arraySize = static_cast<int>(numExpr->value);
                    } else {
                        throw std::runtime_error("Array size must be a number literal");
                    }
                    
                    // Create FixedArrayExpr with the size and elements
                    return std::make_unique<AssignExpr>(
                        dynamic_cast<VariableExpr*>(indexExpr->object.get())->name,
                        std::make_unique<FixedArrayExpr>(arraySize, std::move(elements))
                    );
                }
            }
            
            auto value = assignment();

            if (auto* varExpr = dynamic_cast<VariableExpr*>(expr.get())) {
                Token name = varExpr->name;
                return std::make_unique<AssignExpr>(name, std::move(value));
            } else if (auto* indexExpr = dynamic_cast<IndexExpr*>(expr.get())) {
                // Handle assigning to array index: arr[idx] = value
                return std::make_unique<AssignIndexExpr>(
                    std::move(indexExpr->object),
                    std::move(indexExpr->index),
                    std::move(value)
                );
            }

            throw std::runtime_error("Invalid assignment target.");
        }

        return expr;
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
        auto expr = unary();

        while (match({TokenType::SLASH, TokenType::STAR, TokenType::PERCENT}))
        {
            Token op = previous();
            auto right = unary();
            expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
        }

        return expr;
    }

    std::unique_ptr<Expr> primary()
    {
        if (match({TokenType::TRUE})) {
            return std::make_unique<BooleanExpr>(true);
        }
        
        if (match({TokenType::FALSE})) {
            return std::make_unique<BooleanExpr>(false);
        }
        
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
        
        // Handle array literals
        if (match({TokenType::LEFT_BRACKET})) {
            std::vector<std::unique_ptr<Expr>> elements;
            
            // Check for empty array first
            if (!check(TokenType::RIGHT_BRACKET)) {
                do {
                    elements.push_back(expression());
                } while (match({TokenType::COMMA}));
            }
            
            consume(TokenType::RIGHT_BRACKET, "Expect ']' after array elements.");
            return std::make_unique<ArrayExpr>(std::move(elements));
        }
        
        throw std::runtime_error("Expect expression.");
    }

    // Add array indexing in the call/subscript precedence level between primary and unary
    std::unique_ptr<Expr> call() {
        auto expr = primary();
        
        while (true) {
            if (match({TokenType::LEFT_PAREN})) {
                expr = finishCall(std::move(expr));
            } else if (match({TokenType::LEFT_BRACKET})) {
                // Array indexing: array[index]
                auto index = expression();
                consume(TokenType::RIGHT_BRACKET, "Expect ']' after index.");
                expr = std::make_unique<IndexExpr>(std::move(expr), std::move(index));
            } else {
                break;
            }
        }
        
        return expr;
    }

    std::unique_ptr<Expr> finishCall(std::unique_ptr<Expr> callee) {
        std::vector<std::unique_ptr<Expr>> arguments;
        
        if (!check(TokenType::RIGHT_PAREN)) {
            do {
                if (arguments.size() >= 255) {
                    error(peek(), "Cannot have more than 255 arguments.");
                }
                arguments.push_back(expression());
            } while (match({TokenType::COMMA}));
        }
        
        Token paren = consume(TokenType::RIGHT_PAREN, "Expect ')' after arguments.");
        
        return std::make_unique<CallExpr>(std::move(callee), paren, std::move(arguments));
    }

    std::unique_ptr<Expr> unary() {
        // Call 'call()' instead of 'primary()' to handle array indexing
        return call();
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
            case TokenType::RETURN_KW:
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

                // Handle AND/OR conditions in else-if
                auto condition = handleElseIfCondition();
                if (condition) {
                    elseIfBranches.push_back(std::make_pair(std::move(condition), statement()));
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

    void error(const Token& token, const std::string& message) {
        if (token.type == TokenType::EOF_TOKEN) {
            std::cerr << "[line " << token.line << "] Error at end: " << message << std::endl;
        } else {
            std::cerr << "[line " << token.line << "] Error at '" << token.lexeme << "': " << message << std::endl;
        }
        throw std::runtime_error(message);
    }
};

#endif