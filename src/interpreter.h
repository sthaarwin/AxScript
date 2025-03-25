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
        
        result = static_cast<double>(isEqual);
        
        if (isEqual && stmt->thenBranch) {
            stmt->thenBranch->accept(this);
            return; // Return after executing then branch
        } 
        
        if (!isEqual && stmt->elseBranch) {
            stmt->elseBranch->accept(this);
        }
    }

    void visit(CompNeqStmt *stmt) override {
        if (!stmt) return;
        
        stmt->left->accept(this);
        auto leftValue = result;
        stmt->right->accept(this);
        auto rightValue = result;
        
        bool isNotEqual = false;
        try {
            isNotEqual = std::get<double>(leftValue) != std::get<double>(rightValue);
        } catch (...) {
            isNotEqual = false;
        }
        
        result = static_cast<double>(isNotEqual);
        
        if (isNotEqual && stmt->thenBranch) {
            stmt->thenBranch->accept(this);
            return; // Return after executing then branch
        }
        
        if (!isNotEqual && stmt->elseBranch) {
            stmt->elseBranch->accept(this);
        }
    }

    void visit(CompGeStmt *stmt) override {
        if (!stmt) return;
        
        stmt->left->accept(this);
        auto leftValue = result;
        stmt->right->accept(this);
        auto rightValue = result;
        
        bool isGreaterEqual = false;
        try {
            isGreaterEqual = std::get<double>(leftValue) >= std::get<double>(rightValue);
        } catch (...) {
            isGreaterEqual = false;
        }
        
        result = static_cast<double>(isGreaterEqual);
        
        if (isGreaterEqual && stmt->thenBranch) {
            stmt->thenBranch->accept(this);
            return; // Return after executing then branch
        }
        
        if (!isGreaterEqual && stmt->elseBranch) {
            stmt->elseBranch->accept(this);
        }
    }

    void visit(CompLeStmt *stmt) override {
        if (!stmt) return;
        
        stmt->left->accept(this);
        auto leftValue = result;
        stmt->right->accept(this);
        auto rightValue = result;
        
        bool isLessEqual = false;
        try {
            isLessEqual = std::get<double>(leftValue) <= std::get<double>(rightValue);
        } catch (...) {
            isLessEqual = false;
        }
        
        result = static_cast<double>(isLessEqual);
        
        if (isLessEqual && stmt->thenBranch) {
            stmt->thenBranch->accept(this);
            return; // Return after executing then branch
        }
        
        if (!isLessEqual && stmt->elseBranch) {
            stmt->elseBranch->accept(this);
        }
    }

    void visit(CompGStmt *stmt) override {
        if (!stmt) return;
        
        stmt->left->accept(this);
        auto leftValue = result;
        stmt->right->accept(this);
        auto rightValue = result;
        
        bool isGreater = false;
        try {
            isGreater = std::get<double>(leftValue) > std::get<double>(rightValue);
        } catch (...) {
            isGreater = false;
        }
        
        result = static_cast<double>(isGreater);
        
        if (isGreater && stmt->thenBranch) {
            stmt->thenBranch->accept(this);
            return; // Return after executing then branch
        }
        
        // Try each else-if branch if the condition was false
        if (!isGreater) {
            // Check else-if branches
            for (const auto& elseIfPair : stmt->elseIfBranches) {
                elseIfPair.first->accept(this);
                bool condition = std::get<double>(result) != 0.0;
                
                if (condition) {
                    elseIfPair.second->accept(this);
                    return; // Return after executing matching branch
                }
            }
            
            // If no else-if conditions matched, execute else branch
            if (stmt->elseBranch) {
                stmt->elseBranch->accept(this);
            }
        }
    }

    void visit(CompLStmt *stmt) override {
        if (!stmt) return;
        
        stmt->left->accept(this);
        auto leftValue = result;
        stmt->right->accept(this);
        auto rightValue = result;
        
        bool isLess = false;
        try {
            isLess = std::get<double>(leftValue) < std::get<double>(rightValue);
        } catch (...) {
            isLess = false;
        }
        
        result = static_cast<double>(isLess);
        
        if (isLess && stmt->thenBranch) {
            stmt->thenBranch->accept(this);
            return; // Return after executing then branch
        }
        
        if (!isLess && stmt->elseBranch) {
            stmt->elseBranch->accept(this);
        }
    }

    void visit(AndStmt *stmt) override {
        // First, ensure we have a valid statement
        if (!stmt) return;
        
        // Evaluate first condition
        stmt->left->accept(this);
        double firstResult = std::get<double>(result);
        
        // Short-circuit if first condition is false
        if (firstResult == 0.0) {
            if (stmt->elseBranch) {
                stmt->elseBranch->accept(this);
            }
            return;
        }
        
        // Evaluate second condition only if first was true
        stmt->right->accept(this);
        double secondResult = std::get<double>(result);
        
        // Execute the appropriate branch based on the result
        if (secondResult != 0.0) {
            // Both conditions are true
            stmt->thenBranch->accept(this);
        } else {
            // Second condition is false
            if (stmt->elseBranch) {
                stmt->elseBranch->accept(this);
            }
        }
    }

    void visit(OrStmt *stmt) override {
        // Ensure we have a valid statement
        if (!stmt) return;
        
        // Evaluate first condition
        stmt->left->accept(this);
        double firstResult = std::get<double>(result);
        
        // Short-circuit if first condition is true
        if (firstResult != 0.0) {
            if (stmt->thenBranch) {
                stmt->thenBranch->accept(this);
            }
            return;
        }
        
        // Evaluate second condition only if first was false
        stmt->right->accept(this);
        double secondResult = std::get<double>(result);
        
        // Execute appropriate branch based on result
        if (secondResult != 0.0) {
            // At least one condition is true
            if (stmt->thenBranch) {
                stmt->thenBranch->accept(this);
            }
        } else {
            // Both conditions are false
            if (stmt->elseBranch) {
                stmt->elseBranch->accept(this);
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
            condition->accept(this);
            auto value = result;
            
            try {
                if (std::get<double>(value) == 0.0) {
                    allTrue = false;
                    break;
                }
            } catch (...) {
                allTrue = false;
                break;
            }
        }
        
        if (allTrue && stmt->thenBranch) {
            stmt->thenBranch->accept(this);
        } else if (!allTrue && stmt->elseBranch) {
            stmt->elseBranch->accept(this);
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