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
    std::variant<double, std::string, bool> result;
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
    
    void visit(BooleanExpr *expr) override
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
                // String concatenation - strings are at index 1
                std::string leftStr;
                if (leftValue.index() == 0) {
                    leftStr = std::to_string(std::get<double>(leftValue));
                } else if (leftValue.index() == 1) {
                    leftStr = std::get<std::string>(leftValue);
                } else if (leftValue.index() == 2) {
                    leftStr = std::get<bool>(leftValue) ? "true" : "false";
                }
                
                std::string rightStr;
                if (rightValue.index() == 0) {
                    rightStr = std::to_string(std::get<double>(rightValue));
                } else if (rightValue.index() == 1) {
                    rightStr = std::get<std::string>(rightValue);
                } else if (rightValue.index() == 2) {
                    rightStr = std::get<bool>(rightValue) ? "true" : "false";
                }
                
                result = leftStr + rightStr;
            } else if (leftValue.index() == 0 && rightValue.index() == 0) {
                // Numeric addition
                result = std::get<double>(leftValue) + std::get<double>(rightValue);
            } else {
                throw std::runtime_error("Operands must be two numbers or at least one string.");
            }
            break;
        case TokenType::MINUS:
            checkNumberOperands(expr->op, leftValue, rightValue);
            result = std::get<double>(leftValue) - std::get<double>(rightValue);
            break;
        case TokenType::STAR:
            checkNumberOperands(expr->op, leftValue, rightValue);
            result = std::get<double>(leftValue) * std::get<double>(rightValue);
            break;
        case TokenType::SLASH:
            checkNumberOperands(expr->op, leftValue, rightValue);
            if (std::get<double>(rightValue) == 0) {
                throw std::runtime_error("Error: Division by zero");
            }
            result = std::get<double>(leftValue) / std::get<double>(rightValue);
            break;
        case TokenType::GREATER:
            if (leftValue.index() == 0 && rightValue.index() == 0) {
                result = std::get<double>(leftValue) > std::get<double>(rightValue);
            } else if (leftValue.index() == 1 && rightValue.index() == 1) {
                result = std::get<std::string>(leftValue) > std::get<std::string>(rightValue);
            } else {
                throw std::runtime_error("Operands must be two numbers or two strings.");
            }
            break;
        case TokenType::GREATER_EQUAL:
            if (leftValue.index() == 0 && rightValue.index() == 0) {
                result = std::get<double>(leftValue) >= std::get<double>(rightValue);
            } else if (leftValue.index() == 1 && rightValue.index() == 1) {
                result = std::get<std::string>(leftValue) >= std::get<std::string>(rightValue);
            } else {
                throw std::runtime_error("Operands must be two numbers or two strings.");
            }
            break;
        case TokenType::LESS:
            if (leftValue.index() == 0 && rightValue.index() == 0) {
                result = std::get<double>(leftValue) < std::get<double>(rightValue);
            } else if (leftValue.index() == 1 && rightValue.index() == 1) {
                result = std::get<std::string>(leftValue) < std::get<std::string>(rightValue);
            } else {
                throw std::runtime_error("Operands must be two numbers or two strings.");
            }
            break;
        case TokenType::LESS_EQUAL:
            if (leftValue.index() == 0 && rightValue.index() == 0) {
                result = std::get<double>(leftValue) <= std::get<double>(rightValue);
            } else if (leftValue.index() == 1 && rightValue.index() == 1) {
                result = std::get<std::string>(leftValue) <= std::get<std::string>(rightValue);
            } else {
                throw std::runtime_error("Operands must be two numbers or two strings.");
            }
            break;
        case TokenType::EQUAL_EQUAL:
            result = isEqual(leftValue, rightValue);
            break;
        case TokenType::BANG_EQUAL:
            result = !isEqual(leftValue, rightValue);
            break;
        default:
            throw std::runtime_error("Invalid binary operator");
        }
    }

    // Helper for boolean equality comparison
    bool isEqual(const std::variant<double, std::string, bool>& a, 
                 const std::variant<double, std::string, bool>& b) {
        // Different types are never equal
        if (a.index() != b.index()) return false;
        
        // Same type comparison
        switch (a.index()) {
            case 0: // double
                return std::get<double>(a) == std::get<double>(b);
            case 1: // string
                return std::get<std::string>(a) == std::get<std::string>(b);
            case 2: // boolean
                return std::get<bool>(a) == std::get<bool>(b);
            default:
                return false;
        }
    }

    // Helper to check number operands
    void checkNumberOperands(const Token& op, 
                            const std::variant<double, std::string, bool>& left,
                            const std::variant<double, std::string, bool>& right) {
        if (left.index() == 0 && right.index() == 0) return;
        throw std::runtime_error(std::string("Operands must be numbers for operator '") + 
                                op.lexeme + "'.");
    }

    void visit(PrintStmt *stmt) override
    {
        stmt->expression->accept(this);
        std::visit([](auto &&arg) {
            std::string output;
            if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, std::string>) {
                output = arg;
            } else if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, bool>) {
                output = arg ? "true" : "false";
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
        std::variant<double, std::string, bool> value;
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
                // Check for boolean values
                if (input == "true") {
                    environment.define(stmt->variableName.lexeme, true);
                } else if (input == "false") {
                    environment.define(stmt->variableName.lexeme, false);
                } else {
                    environment.define(stmt->variableName.lexeme, input);
                }
            }
        }
        catch (const std::invalid_argument&) {
            // Check for boolean values
            if (input == "true") {
                environment.define(stmt->variableName.lexeme, true);
            } else if (input == "false") {
                environment.define(stmt->variableName.lexeme, false);
            } else {
                // Not a number or boolean, treat as string
                environment.define(stmt->variableName.lexeme, input);
            }
        }
        catch (const std::out_of_range&) {
            // Number out of range
            std::cerr << "Warning: Number out of range, treating as string" << std::endl;
            environment.define(stmt->variableName.lexeme, input);
        }
    }
    
    // Helper to check if a value is truthy
    bool isTruthy(const std::variant<double, std::string, bool>& value) {
        if (value.index() == 2) { // boolean
            return std::get<bool>(value);
        } else if (value.index() == 0) { // number
            return std::get<double>(value) != 0.0;
        } else { // string
            return !std::get<std::string>(value).empty();
        }
    }

    void visit(CompEqStmt *stmt) override {
        if (!stmt) return;
        
        stmt->left->accept(this);
        auto leftValue = result;
        stmt->right->accept(this);
        auto rightValue = result;
        
        bool isEqual = this->isEqual(leftValue, rightValue);
        result = isEqual; // Store as boolean directly
        
        if (isEqual && stmt->thenBranch) {
            stmt->thenBranch->accept(this);
            return;
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
        
        // Use the isEqual helper and negate the result
        bool isNotEqual = !isEqual(leftValue, rightValue);
        // Store the boolean value
        result = isNotEqual;
        
        // Execute the appropriate branch
        if (isNotEqual && stmt->thenBranch) {
            stmt->thenBranch->accept(this);
        } else if (!isNotEqual && stmt->elseBranch) {
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
        
        // Handle comparisons based on types
        if (leftValue.index() == rightValue.index()) {
            if (leftValue.index() == 0) { // numbers
                isGreaterEqual = std::get<double>(leftValue) >= std::get<double>(rightValue);
            } else if (leftValue.index() == 1) { // strings
                isGreaterEqual = std::get<std::string>(leftValue) >= std::get<std::string>(rightValue);
            } else if (leftValue.index() == 2) { // booleans
                // For booleans: true >= true, true >= false, false >= false
                isGreaterEqual = !std::get<bool>(leftValue) || std::get<bool>(rightValue);
            }
        }
        
        result = isGreaterEqual; // Store result directly as a boolean
        
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
        
        // Handle comparisons based on types
        if (leftValue.index() == rightValue.index()) {
            if (leftValue.index() == 0) { // numbers
                isLessEqual = std::get<double>(leftValue) <= std::get<double>(rightValue);
            } else if (leftValue.index() == 1) { // strings
                isLessEqual = std::get<std::string>(leftValue) <= std::get<std::string>(rightValue);
            } else if (leftValue.index() == 2) { // booleans
                // For booleans: false <= false, false <= true, true <= true
                isLessEqual = std::get<bool>(leftValue) <= std::get<bool>(rightValue);
            }
        }
        
        result = isLessEqual; // Store as a boolean directly
        
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
        
        // Handle comparisons based on types
        if (leftValue.index() == rightValue.index()) {
            if (leftValue.index() == 0) { // numbers
                isGreater = std::get<double>(leftValue) > std::get<double>(rightValue);
            } else if (leftValue.index() == 1) { // strings
                isGreater = std::get<std::string>(leftValue) > std::get<std::string>(rightValue);
            } else if (leftValue.index() == 2) { // booleans
                // For booleans: true > false, but not false > true or others
                isGreater = std::get<bool>(leftValue) && !std::get<bool>(rightValue);
            }
        }
        
        result = isGreater; // Store boolean result
        
        if (isGreater && stmt->thenBranch) {
            stmt->thenBranch->accept(this);
            return;
        }
        
        // Handle else branch if not greater
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
        
        // Handle comparisons based on types
        if (leftValue.index() == rightValue.index()) {
            if (leftValue.index() == 0) { // numbers
                isLess = std::get<double>(leftValue) < std::get<double>(rightValue);
            } else if (leftValue.index() == 1) { // strings
                isLess = std::get<std::string>(leftValue) < std::get<std::string>(rightValue);
            } else if (leftValue.index() == 2) { // booleans
                // For booleans: false < true, but not others
                isLess = !std::get<bool>(leftValue) && std::get<bool>(rightValue);
            }
        }
        
        result = isLess; // Store as boolean directly
        
        if (isLess && stmt->thenBranch) {
            stmt->thenBranch->accept(this);
            return;
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
        
        // Perform equality check without calling isEqual to avoid recursion
        bool equal = false;
        
        // First check if types match
        if (leftValue.index() == rightValue.index()) {
            switch (leftValue.index()) {
                case 0: // double
                    equal = std::get<double>(leftValue) == std::get<double>(rightValue);
                    break;
                case 1: // string
                    equal = std::get<std::string>(leftValue) == std::get<std::string>(rightValue);
                    break;
                case 2: // boolean
                    equal = std::get<bool>(leftValue) == std::get<bool>(rightValue);
                    break;
            }
        }
        
        result = equal;
    }

    void visit(AndConditionStmt* stmt) override {
        if (!stmt || stmt->conditions.empty()) return;
        
        bool allTrue = true;
        for (const auto& condition : stmt->conditions) {
            condition->accept(this);
            if (!isTruthy(result)) {
                allTrue = false;
                break;
            }
        }
        
        if (allTrue && stmt->thenBranch) {
            stmt->thenBranch->accept(this);
        } else if (!allTrue && stmt->elseBranch) {
            stmt->elseBranch->accept(this);
        }
        
        result = allTrue; // Store result as boolean
    }

    void visit(OrConditionStmt* stmt) override {
        if (!stmt || stmt->conditions.empty()) return;
        
        bool anyTrue = false;
        for (const auto& condition : stmt->conditions) {
            condition->accept(this);
            if (isTruthy(result)) {
                anyTrue = true;
                break;
            }
        }
        
        if (anyTrue && stmt->thenBranch) {
            stmt->thenBranch->accept(this);
        } else if (!anyTrue && stmt->elseBranch) {
            stmt->elseBranch->accept(this);
        }
        
        result = anyTrue; // Store result as boolean
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