// interpreter.h
#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "visitor.h"
#include "ast.h"
#include "environment.h"
#include <iostream>
#include <variant>

class Interpreter : public Visitor
{
private:
    std::variant<double, std::string> result;
    Environment environment;
    bool breakEncountered = false;
    bool continueEncountered = false;

private:
    void execute(const std::unique_ptr<Stmt>& stmt) {
        if (stmt) {
            stmt->accept(this);
        }
    }

public:
    void visit(NumberExpr *expr) override
    {
        result = expr->value;
    }

    void visit(StringExpr *expr) override
    {
        result = expr->value;
    }

    void visit(VariableExpr *expr) override
    {
        if (environment.isDefined(expr->name.lexeme))
        {
            result = environment.get(expr->name.lexeme);
        }
        else
        {
            throw std::runtime_error("Undefined variable '" + expr->name.lexeme + "'");
        }
    }

    void visit(BinaryExpr *expr) override
    {
        expr->left->accept(this);
        auto leftValue = result;
        expr->right->accept(this);
        auto rightValue = result;

        switch (expr->op.type)
        {
        case TokenType::PLUS:
            result = std::get<double>(leftValue) + std::get<double>(rightValue);
            break;
        case TokenType::MINUS:
            result = std::get<double>(leftValue) - std::get<double>(rightValue);
            break;
        case TokenType::STAR:
            result = std::get<double>(leftValue) * std::get<double>(rightValue);
            break;
        case TokenType::SLASH:
            result = std::get<double>(leftValue) / std::get<double>(rightValue);
            break;
        case TokenType::GREATER:
            result = static_cast<double>(std::get<double>(leftValue) > std::get<double>(rightValue));
            break;
        case TokenType::GREATER_EQUAL:
            result = static_cast<double>(std::get<double>(leftValue) >= std::get<double>(rightValue));
            break;
        case TokenType::LESS:
            result = static_cast<double>(std::get<double>(leftValue) < std::get<double>(rightValue));
            break;
        case TokenType::LESS_EQUAL:
            result = static_cast<double>(std::get<double>(leftValue) <= std::get<double>(rightValue));
            break;
        case TokenType::EQUAL_EQUAL:
            result = static_cast<double>(std::get<double>(leftValue) == std::get<double>(rightValue));
            break;
        case TokenType::BANG_EQUAL:
            result = static_cast<double>(std::get<double>(leftValue) != std::get<double>(rightValue));
            break;
        default:
            throw std::runtime_error("Invalid binary operator");
        }
    }

    void visit(PrintStmt *stmt) override
    {
        stmt->expression->accept(this);
        std::visit([](auto &&arg)
                   { std::cout << arg << std::endl; }, result);
    }

    void visit(VarStmt *stmt) override
    {
        std::variant<double, std::string> value;
        if (stmt->initializer != nullptr)
        {
            stmt->initializer->accept(this);
            value = result;
        }
        else
        {
            value = 0.0;
        }
        environment.define(stmt->name.lexeme, value);
    }

    void visit(InputStmt *stmt) override
    {
        std::string input;
        std::getline(std::cin, input);
        try
        {
            double value = std::stod(input);
            environment.define(stmt->variableName.lexeme, value);
        }
        catch (...)
        {
            environment.define(stmt->variableName.lexeme, input);
        }
    }

    void visit(CompEqStmt *stmt) override {
        if (!stmt) return;
        
        stmt->left->accept(this);
        auto leftValue = result;
        stmt->right->accept(this);
        auto rightValue = result;
        
        bool isEqual = false;
        try {
            isEqual = std::get<double>(leftValue) == std::get<double>(rightValue);
        } catch (...) {
            isEqual = false;
        }
        
        // Set the result for use in logical operations
        result = static_cast<double>(isEqual);
        
        // If this is a standalone comparison (not part of AND/OR),
        // execute the appropriate branch
        if (stmt->thenBranch || stmt->elseBranch) {
            if (isEqual && stmt->thenBranch) {
                stmt->thenBranch->accept(this);
            } else if (!isEqual && stmt->elseBranch) {
                stmt->elseBranch->accept(this);
            }
        }
    }

    void visit(CompNeqStmt *stmt) override {
        stmt->left->accept(this);
        auto leftValue = result;
        stmt->right->accept(this);
        auto rightValue = result;
        
        bool isNotEqual = std::get<double>(leftValue) != std::get<double>(rightValue);
        result = static_cast<double>(isNotEqual);
        
        if (isNotEqual && stmt->thenBranch) {
            stmt->thenBranch->accept(this);
        } else if (!isNotEqual && stmt->elseBranch) {
            stmt->elseBranch->accept(this);
        }
    }

    void visit(CompGeStmt *stmt) override {
        stmt->left->accept(this);
        auto leftValue = result;
        stmt->right->accept(this);
        auto rightValue = result;
        
        bool isGreaterEqual = std::get<double>(leftValue) >= std::get<double>(rightValue);
        result = static_cast<double>(isGreaterEqual);
        
        if (isGreaterEqual && stmt->thenBranch) {
            stmt->thenBranch->accept(this);
        } else if (!isGreaterEqual && stmt->elseBranch) {
            stmt->elseBranch->accept(this);
        }
    }

    void visit(CompLeStmt *stmt) override {
        stmt->left->accept(this);
        auto leftValue = result;
        stmt->right->accept(this);
        auto rightValue = result;
        
        bool isLessEqual = std::get<double>(leftValue) <= std::get<double>(rightValue);
        result = static_cast<double>(isLessEqual);
        
        if (isLessEqual && stmt->thenBranch) {
            stmt->thenBranch->accept(this);
        } else if (!isLessEqual && stmt->elseBranch) {
            stmt->elseBranch->accept(this);
        }
    }

    void visit(CompGStmt *stmt) override {
        if (!stmt) return;
        
        stmt->left->accept(this);
        auto leftValue = result;
        stmt->right->accept(this);
        auto rightValue = result;
        
        bool isGreater = std::get<double>(leftValue) > std::get<double>(rightValue);
        
        if (isGreater) {
            if (stmt->thenBranch) {
                stmt->thenBranch->accept(this);
            }
            return;
        }

        // Try each else-if branch
        for (const auto& elseIfPair : stmt->elseIfBranches) {
            // Evaluate the condition expression
            elseIfPair.first->accept(this);
            bool condition = std::get<double>(result) != 0.0;
            
            if (condition) {
                elseIfPair.second->accept(this);
                return;  // Important: return after executing matching branch
            }
        }
        
        // If no conditions matched, execute else branch
        if (stmt->elseBranch) {
            stmt->elseBranch->accept(this);
        }
    }

    void visit(CompLStmt *stmt) override {
        stmt->left->accept(this);
        auto leftValue = result;
        stmt->right->accept(this);
        auto rightValue = result;
        
        bool isLess = std::get<double>(leftValue) < std::get<double>(rightValue);
        result = static_cast<double>(isLess);
        
        if (isLess && stmt->thenBranch) {
            stmt->thenBranch->accept(this);
        } else if (!isLess && stmt->elseBranch) {
            stmt->elseBranch->accept(this);
        }
    }

    void visit(AndStmt *stmt) override {
        stmt->left->accept(this);
        auto leftValue = result;
        if (std::get<double>(leftValue) != 0.0) {
            stmt->right->accept(this);
            auto rightValue = result;
            if (std::get<double>(rightValue) != 0.0) {
                stmt->thenBranch->accept(this);
            }
        }
    }

    void visit(OrStmt *stmt) override {
        stmt->left->accept(this);
        auto leftValue = result;
        if (std::get<double>(leftValue) != 0.0) {
            stmt->thenBranch->accept(this);
        } else {
            stmt->right->accept(this);
            auto rightValue = result;
            if (std::get<double>(rightValue) != 0.0) {
                stmt->thenBranch->accept(this);
            }
        }
    }

    void visit(NotStmt *stmt) override {
        stmt->operand->accept(this);
        auto operandValue = result;
        if (std::get<double>(operandValue) == 0.0) {
            stmt->thenBranch->accept(this);
        }
    }

    void visit(BlockStmt *stmt) override
    {
        for (const auto &statement : stmt->statements)
        {
            statement->accept(this);
        }
    }

    void visit(LoopStmt *stmt) override
    {
        if (!stmt)
        {
            return;
        }

        try
        {
            // Initialize loop variable
            if (stmt->from)
            {
                stmt->from->accept(this);
                double fromValue = std::get<double>(result);
                environment.define(stmt->var.lexeme, fromValue);
            }

            // Get end value
            if (!stmt->to)
            {
                return; // Safety check
            }
            stmt->to->accept(this);
            double toValue = std::get<double>(result);

            // Get step value
            double stepValue = 1.0;
            if (stmt->step)
            {
                stmt->step->accept(this);
                stepValue = std::get<double>(result);
                if (stepValue == 0)
                {
                    throw std::runtime_error("Step value cannot be zero");
                }
            }

            if (stmt->isDownward)
            {
                stepValue = -std::abs(stepValue);
            }

            while (true)
            {
                // Check termination condition
                if (!environment.isDefined(stmt->var.lexeme))
                {
                    throw std::runtime_error("Undefined loop variable '" + stmt->var.lexeme + "'");
                }
                double currentValue = std::get<double>(environment.get(stmt->var.lexeme));
                if (stmt->isDownward)
                {
                    if (currentValue < toValue)
                        break;
                }
                else
                {
                    if (currentValue > toValue)
                        break;
                }

                // Execute loop body
                if (stmt->body)
                {
                    stmt->body->accept(this);
                }

                // Handle break
                if (breakEncountered)
                {
                    breakEncountered = false;
                    break;
                }

                // Update loop variable before handling continue
                environment.define(stmt->var.lexeme, currentValue + stepValue);

                // Handle continue
                if (continueEncountered)
                {
                    continueEncountered = false;
                    continue; // Skip to the next iteration
                }
            }
        }
        catch (const std::exception &e)
        {
            // Handle any errors that occur during loop execution
            std::cerr << "Loop error: " << e.what() << std::endl;
        }
    }

    void visit(BreakStmt *stmt) override
    {
        breakEncountered = true;
    }

    void visit(ContinueStmt *stmt) override
    {
        continueEncountered = true;
    }

    void visit(ExpressionStmt *stmt) override
    {
        stmt->expression->accept(this);
    }

    void visit(CompEqExpr* expr) override {
        expr->left->accept(this);
        auto leftValue = result;
        expr->right->accept(this);
        auto rightValue = result;
        result = static_cast<double>(std::get<double>(leftValue) == std::get<double>(rightValue));
    }

    void visit(AndConditionStmt* stmt) override {
        if (!stmt || stmt->conditions.empty()) return;
        
        bool allTrue = true;
        for (const auto& condition : stmt->conditions) {
            if (auto compEq = dynamic_cast<CompEqStmt*>(condition.get())) {
                compEq->left->accept(this);
                auto leftValue = result;
                compEq->right->accept(this);
                auto rightValue = result;
                
                try {
                    if (std::get<double>(leftValue) != std::get<double>(rightValue)) {
                        allTrue = false;
                        break;
                    }
                } catch (...) {
                    allTrue = false;
                    break;
                }
            }
        }
        
        if (allTrue) {
            if (stmt->thenBranch) {
                stmt->thenBranch->accept(this);
            }
        } else {
            if (stmt->elseBranch) {
                stmt->elseBranch->accept(this);
            }
        }
    }

    void visit(OrConditionStmt* stmt) override {
        if (!stmt) return;
        
        bool anyTrue = false;
        for (const auto& condition : stmt->conditions) {
            if (auto compEq = dynamic_cast<CompEqStmt*>(condition.get())) {
                compEq->left->accept(this);
                auto leftValue = result;
                compEq->right->accept(this);
                auto rightValue = result;
                
                try {
                    if (std::get<double>(leftValue) == std::get<double>(rightValue)) {
                        anyTrue = true;
                        break;
                    }
                } catch (...) {
                    continue;
                }
            }
        }
        
        result = static_cast<double>(anyTrue);  // Store the overall result
        
        if (anyTrue && stmt->thenBranch) {
            stmt->thenBranch->accept(this);
        } else if (!anyTrue && stmt->elseBranch) {
            stmt->elseBranch->accept(this);
        }
    }

    void interpret(const std::vector<std::unique_ptr<Stmt>> &statements)
    {
        try
        {
            breakEncountered = false;
            continueEncountered = false;
            for (const auto &stmt : statements)
            {
                if (stmt)
                {
                    execute(stmt);
                    continueEncountered = false;
                }
            }
        }
        catch (const std::runtime_error &error)
        {
            std::cerr << "Runtime error: " << error.what() << std::endl;
        }
    }
};

#endif