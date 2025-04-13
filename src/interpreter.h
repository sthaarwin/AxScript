// interpreter.h
#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "visitor.h"
#include "ast.h"
#include "environment.h"
#include <iostream>
#include <variant>
#include <sstream>
#include <iomanip>

class Interpreter : public Visitor
{
private:
    std::variant<double, std::string> result;
    Environment environment;
    bool breakEncountered = false;
    bool continueEncountered = false;
    bool inLoop = false; // Track whether we're inside a loop for break/continue validation

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
            if (leftValue.index() == 1 || rightValue.index() == 1) {
                // String concatenation
                std::string leftStr = (leftValue.index() == 1) ? 
                    std::get<std::string>(leftValue) : 
                    std::to_string(std::get<double>(leftValue));
                
                std::string rightStr = (rightValue.index() == 1) ? 
                    std::get<std::string>(rightValue) : 
                    std::to_string(std::get<double>(rightValue));
                
                result = leftStr + rightStr;
            } else {
                result = std::get<double>(leftValue) + std::get<double>(rightValue);
            }
            break;
        case TokenType::MINUS:
            result = std::get<double>(leftValue) - std::get<double>(rightValue);
            break;
        case TokenType::STAR:
            result = std::get<double>(leftValue) * std::get<double>(rightValue);
            break;
        case TokenType::SLASH:
            if (std::get<double>(rightValue) == 0) {
                throw std::runtime_error("Error: Division by zero");
            }
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
        std::visit([](auto &&arg) {
            std::string output;
            if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, std::string>) {
                output = arg;
            } else {
                // Format number with better precision
                double value = arg;
                if (value == static_cast<int>(value)) {
                    // It's a whole number
                    output = std::to_string(static_cast<int>(value));
                } else {
                    // Format with precision to avoid trailing zeros
                    std::ostringstream ss;
                    ss << std::fixed << std::setprecision(15) << value;
                    output = ss.str();
                    // Remove trailing zeros
                    output = output.substr(0, output.find_last_not_of('0') + 1);
                    // Remove trailing decimal point if needed
                    if (output.back() == '.') {
                        output.pop_back();
                    }
                }
            }
            
            // Process escape sequences
            std::string processed;
            for (size_t i = 0; i < output.length(); i++) {
                if (output[i] == '\\' && i + 1 < output.length()) {
                    switch (output[i + 1]) {
                        case 'n':
                            processed += '\n';
                            i++;
                            break;
                        case 't':
                            processed += '\t';
                            i++;
                            break;
                        default:
                            processed += output[i];
                    }
                } else {
                    processed += output[i];
                }
            }
            
            // Print without adding a newline
            std::cout << processed;
        }, result);
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
        
        // Try to convert to number, but handle errors properly
        try {
            size_t pos;
            double value = std::stod(input, &pos);
            
            // Check if the entire string was converted
            if (pos == input.length()) {
                environment.define(stmt->variableName.lexeme, value);
            } else {
                environment.define(stmt->variableName.lexeme, input);
            }
        }
        catch (const std::invalid_argument&) {
            // Not a number
            environment.define(stmt->variableName.lexeme, input);
        }
        catch (const std::out_of_range&) {
            // Number out of range
            std::cerr << "Warning: Number out of range, treating as string" << std::endl;
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
        
        // Check if both are the same type
        if (leftValue.index() == rightValue.index()) {
            if (leftValue.index() == 0) { // Both are numbers
                isEqual = std::get<double>(leftValue) == std::get<double>(rightValue);
            } else { // Both are strings
                isEqual = std::get<std::string>(leftValue) == std::get<std::string>(rightValue);
            }
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
        
        // Check if both are the same type
        if (leftValue.index() == rightValue.index()) {
            if (leftValue.index() == 0) { // Both are numbers
                isNotEqual = std::get<double>(leftValue) != std::get<double>(rightValue);
            } else { // Both are strings
                isNotEqual = std::get<std::string>(leftValue) != std::get<std::string>(rightValue);
            }
        } else {
            // Different types are always not equal
            isNotEqual = true;
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
        
        // Handle string comparison if both values are strings
        if (leftValue.index() == 1 && rightValue.index() == 1) {
            isGreaterEqual = std::get<std::string>(leftValue) >= std::get<std::string>(rightValue);
        } 
        // Handle numeric comparison
        else if (leftValue.index() == 0 && rightValue.index() == 0) {
            isGreaterEqual = std::get<double>(leftValue) >= std::get<double>(rightValue);
        }
        
        result = static_cast<double>(isGreaterEqual);
        
        if (isGreaterEqual && stmt->thenBranch) {
            stmt->thenBranch->accept(this);
            return;
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
        
        // Handle string comparison if both values are strings
        if (leftValue.index() == 1 && rightValue.index() == 1) {
            isLessEqual = std::get<std::string>(leftValue) <= std::get<std::string>(rightValue);
        }
        // Handle numeric comparison
        else if (leftValue.index() == 0 && rightValue.index() == 0) {
            isLessEqual = std::get<double>(leftValue) <= std::get<double>(rightValue);
        }
        
        result = static_cast<double>(isLessEqual);
        
        if (isLessEqual && stmt->thenBranch) {
            stmt->thenBranch->accept(this);
            return;
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
        
        // Handle string comparison if both values are strings
        if (leftValue.index() == 1 && rightValue.index() == 1) {
            isGreater = std::get<std::string>(leftValue) > std::get<std::string>(rightValue);
        }
        // Handle numeric comparison
        else if (leftValue.index() == 0 && rightValue.index() == 0) {
            isGreater = std::get<double>(leftValue) > std::get<double>(rightValue);
        }
        
        result = static_cast<double>(isGreater);
        
        if (isGreater && stmt->thenBranch) {
            stmt->thenBranch->accept(this);
            return;
        }
        
        // Handle else-if branches
        if (!isGreater && !stmt->elseIfBranches.empty()) {
            for (const auto& elseIfPair : stmt->elseIfBranches) {
                // Try to execute as AndConditionStmt first
                if (auto andCondition = dynamic_cast<AndConditionStmt*>(elseIfPair.first.get())) {
                    bool allTrue = true;
                    for (const auto& condition : andCondition->conditions) {
                        condition->accept(this);
                        if (std::get<double>(result) == 0.0) {
                            allTrue = false;
                            break;
                        }
                    }
                    if (allTrue) {
                        elseIfPair.second->accept(this);
                        return;
                    }
                } 
                // Regular condition
                else {
                    elseIfPair.first->accept(this);
                    if (std::get<double>(result) != 0.0) {
                        elseIfPair.second->accept(this);
                        return;
                    }
                }
            }
        }
        
        // If no conditions matched, execute else branch
        if (!isGreater && stmt->elseBranch) {
            stmt->elseBranch->accept(this);
        }
    }

    void visit(CompLStmt *stmt) override {
        if (!stmt) return;
        
        stmt->left->accept(this);
        auto leftValue = result;
        stmt->right->accept(this);
        auto rightValue = result;
        
        bool isLess = false;
        
        // Handle string comparison if both values are strings
        if (leftValue.index() == 1 && rightValue.index() == 1) {
            isLess = std::get<std::string>(leftValue) < std::get<std::string>(rightValue);
        }
        // Handle numeric comparison
        else if (leftValue.index() == 0 && rightValue.index() == 0) {
            isLess = std::get<double>(leftValue) < std::get<double>(rightValue);
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
        if (!stmt) {
            return;
        }

        // Track that we're inside a loop for break/continue validation
        bool wasInLoop = inLoop;
        inLoop = true;

        try {
            // Initialize loop variable
            if (stmt->from) {
                stmt->from->accept(this);
                double fromValue = std::get<double>(result);
                
                // Check if already defined and update or create
                if (environment.isDefined(stmt->var.lexeme)) {
                    environment.assign(stmt->var.lexeme, fromValue);
                } else {
                    environment.define(stmt->var.lexeme, fromValue);
                }
            }

            // Get end value
            if (!stmt->to) {
                return; // Safety check
            }
            stmt->to->accept(this);
            double toValue = std::get<double>(result);

            // Get step value
            double stepValue = 1.0;
            if (stmt->step) {
                stmt->step->accept(this);
                stepValue = std::get<double>(result);
                if (stepValue == 0) {
                    throw std::runtime_error("Step value cannot be zero");
                }
            }

            if (stmt->isDownward) {
                stepValue = -std::abs(stepValue);
            }

            while (true) {
                // Check termination condition
                if (!environment.isDefined(stmt->var.lexeme)) {
                    throw std::runtime_error("Undefined loop variable '" + stmt->var.lexeme + "'");
                }
                
                double currentValue = std::get<double>(environment.get(stmt->var.lexeme));
                if (stmt->isDownward) {
                    if (currentValue < toValue)
                        break;
                } else {
                    if (currentValue > toValue)
                        break;
                }

                // Execute loop body
                if (stmt->body) {
                    stmt->body->accept(this);
                }

                // Handle break
                if (breakEncountered) {
                    breakEncountered = false;
                    break;
                }

                // Update loop variable before handling continue
                if (environment.isDefined(stmt->var.lexeme)) {
                    environment.assign(stmt->var.lexeme, currentValue + stepValue);
                } else {
                    environment.define(stmt->var.lexeme, currentValue + stepValue);
                }

                // Handle continue
                if (continueEncountered) {
                    continueEncountered = false;
                    continue; // Skip to the next iteration
                }
            }
        } catch (const std::exception &e) {
            // Handle any errors that occur during loop execution
            std::cerr << "Loop error: " << e.what() << std::endl;
        }
        
        // Restore previous loop state
        inLoop = wasInLoop;
    }

    void visit(BreakStmt *stmt) override
    {
        if (!inLoop) {
            throw std::runtime_error("Break statement used outside of a loop");
        }
        breakEncountered = true;
    }

    void visit(ContinueStmt *stmt) override
    {
        if (!inLoop) {
            throw std::runtime_error("Continue statement used outside of a loop");
        }
        continueEncountered = true;
    }

    void visit(ExpressionStmt *stmt) override
    {
        stmt->expression->accept(this);
        // Store the result in case it's needed later
        auto exprResult = result;
    }

    void visit(AssignExpr* expr) override {
        expr->value->accept(this);
        environment.assign(expr->name.lexeme, result);
    }

    void visit(CompEqExpr* expr) override {
        expr->left->accept(this);
        auto leftValue = result;
        expr->right->accept(this);
        auto rightValue = result;
        
        bool isEqual = false;
        
        // Handle equality based on the type of operands
        if (leftValue.index() == rightValue.index()) {
            if (leftValue.index() == 0) { // Both are numbers
                isEqual = std::get<double>(leftValue) == std::get<double>(rightValue);
            } else { // Both are strings
                isEqual = std::get<std::string>(leftValue) == std::get<std::string>(rightValue);
            }
        }
        
        result = static_cast<double>(isEqual);
    }

    void visit(AndConditionStmt* stmt) override {
        if (!stmt || stmt->conditions.empty()) return;
        
        bool allTrue = true;
        for (const auto& condition : stmt->conditions) {
            if (auto exprStmt = dynamic_cast<ExpressionStmt*>(condition.get())) {
                if (auto compEqExpr = dynamic_cast<CompEqExpr*>(exprStmt->expression.get())) {
                    compEqExpr->accept(this);
                    if (std::get<double>(result) == 0.0) {
                        allTrue = false;
                        break;
                    }
                }
            }
        }
        
        if (allTrue && stmt->thenBranch) {
            stmt->thenBranch->accept(this);
        } else if (!allTrue && stmt->elseBranch) {
            stmt->elseBranch->accept(this);
        }
        
        result = static_cast<double>(allTrue); // Store the overall result
    }

    void visit(OrConditionStmt* stmt) override {
        if (!stmt || stmt->conditions.empty()) return;
        
        bool anyTrue = false;
        for (const auto& condition : stmt->conditions) {
            if (auto exprStmt = dynamic_cast<ExpressionStmt*>(condition.get())) {
                if (auto compEqExpr = dynamic_cast<CompEqExpr*>(exprStmt->expression.get())) {
                    compEqExpr->accept(this);
                    if (std::get<double>(result) != 0.0) {
                        anyTrue = true;
                        break;
                    }
                }
            }
        }
        
        if (anyTrue && stmt->thenBranch) {
            stmt->thenBranch->accept(this);
        } else if (!anyTrue && stmt->elseBranch) {
            stmt->elseBranch->accept(this);
        }
        
        result = static_cast<double>(anyTrue); // Store the overall result
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