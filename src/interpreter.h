// interpreter.h
#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "visitor.h"
#include "ast.h"
#include "environment.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cmath> 

class Interpreter : public Visitor
{
private:
    Value result;
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
    std::shared_ptr<Environment> environment = std::make_shared<Environment>();
    
    void executeBlock(const std::vector<std::unique_ptr<Stmt>>& statements, 
                     std::shared_ptr<Environment> environment) {
        // Save the current environment
        std::shared_ptr<Environment> previousEnvironment = this->environment;
        
        try {
            // Set the environment to the new one for the block
            this->environment = environment;
            
            // Execute all statements in the block
            for (const auto& statement : statements) {
                execute(statement);
                if (breakEncountered || continueEncountered) {
                    break;
                }
            }
        } catch (...) {
            // Restore the previous environment on any exception
            this->environment = previousEnvironment;
            throw;
        }
        
        // Restore the previous environment
        this->environment = previousEnvironment;
    }

    void visit(NumberExpr *expr) override
    {
        result = makeNumber(expr->value);
    }

    void visit(StringExpr *expr) override
    {
        result = makeString(expr->value);
    }
    
    void visit(BooleanExpr *expr) override
    {
        result = makeBoolean(expr->value);
    }

    void visit(VariableExpr *expr) override
    {
        if (environment->isDefined(expr->name.lexeme))
        {
            result = environment->get(expr->name.lexeme);
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
            if (isString(leftValue) || isString(rightValue)) {
                // String concatenation - convert both operands to string
                std::string leftStr = valueToString(leftValue);
                std::string rightStr = valueToString(rightValue);
                result = makeString(leftStr + rightStr);
            } else if (isNumber(leftValue) && isNumber(rightValue)) {
                // Numeric addition
                result = makeNumber(asNumber(leftValue) + asNumber(rightValue));
            } else if (isArray(leftValue) && isArray(rightValue)) {
                // Array concatenation
                auto resultArray = asArray(leftValue);
                const auto& rightArray = asArray(rightValue);
                resultArray.insert(resultArray.end(), rightArray.begin(), rightArray.end());
                result = makeArray(resultArray);
            } else {
                throw std::runtime_error("Operands must be two numbers, two arrays, or at least one string.");
            }
            break;
        case TokenType::MINUS:
            checkNumberOperands(expr->op, leftValue, rightValue);
            result = makeNumber(asNumber(leftValue) - asNumber(rightValue));
            break;
        case TokenType::STAR:
            checkNumberOperands(expr->op, leftValue, rightValue);
            result = makeNumber(asNumber(leftValue) * asNumber(rightValue));
            break;
        case TokenType::SLASH:
            checkNumberOperands(expr->op, leftValue, rightValue);
            if (asNumber(rightValue) == 0) {
                throw std::runtime_error("Error: Division by zero");
            }
            result = makeNumber(asNumber(leftValue) / asNumber(rightValue));
            break;
        case TokenType::PERCENT: // Added modulo operator
            checkNumberOperands(expr->op, leftValue, rightValue);
            if (asNumber(rightValue) == 0) {
                throw std::runtime_error("Error: Modulo by zero");
            }
            result = makeNumber(std::fmod(asNumber(leftValue), asNumber(rightValue)));
            break;
        case TokenType::GREATER:
            if (isNumber(leftValue) && isNumber(rightValue)) {
                result = makeBoolean(asNumber(leftValue) > asNumber(rightValue));
            } else if (isString(leftValue) && isString(rightValue)) {
                result = makeBoolean(asString(leftValue) > asString(rightValue));
            } else {
                throw std::runtime_error("Operands must be two numbers or two strings.");
            }
            break;
        case TokenType::GREATER_EQUAL:
            if (isNumber(leftValue) && isNumber(rightValue)) {
                result = makeBoolean(asNumber(leftValue) >= asNumber(rightValue));
            } else if (isString(leftValue) && isString(rightValue)) {
                result = makeBoolean(asString(leftValue) >= asString(rightValue));
            } else {
                throw std::runtime_error("Operands must be two numbers or two strings.");
            }
            break;
        case TokenType::LESS:
            if (isNumber(leftValue) && isNumber(rightValue)) {
                result = makeBoolean(asNumber(leftValue) < asNumber(rightValue));
            } else if (isString(leftValue) && isString(rightValue)) {
                result = makeBoolean(asString(leftValue) < asString(rightValue));
            } else {
                throw std::runtime_error("Operands must be two numbers or two strings.");
            }
            break;
        case TokenType::LESS_EQUAL:
            if (isNumber(leftValue) && isNumber(rightValue)) {
                result = makeBoolean(asNumber(leftValue) <= asNumber(rightValue));
            } else if (isString(leftValue) && isString(rightValue)) {
                result = makeBoolean(asString(leftValue) <= asString(rightValue));
            } else {
                throw std::runtime_error("Operands must be two numbers or two strings.");
            }
            break;
        case TokenType::EQUAL_EQUAL:
            result = makeBoolean(isEqual(leftValue, rightValue));
            break;
        case TokenType::BANG_EQUAL:
            result = makeBoolean(!isEqual(leftValue, rightValue));
            break;
        default:
            throw std::runtime_error("Invalid binary operator");
        }
    }

    void visit(FixedArrayExpr* expr) override {
        std::vector<Value> array;
        
        // Populate array with elements
        for (const auto& element : expr->elements) {
            element->accept(this);
            array.push_back(result);
        }
        
        // Check if we need to pad the array to match the specified size
        while (array.size() < static_cast<size_t>(expr->size)) {
            // Pad with default value (0)
            array.push_back(makeNumber(0));
        }
        
        // If provided more elements than specified size, trim the array
        if (array.size() > static_cast<size_t>(expr->size)) {
            array.resize(expr->size);
        }
        
        result = makeArray(array);
    }

    void visit(ArrayExpr* expr) override {
        std::vector<Value> array;
        for (const auto& element : expr->elements) {
            element->accept(this);
            array.push_back(result);
        }
        result = makeArray(array);
    }

    void visit(IndexExpr* expr) override {
        // Evaluate the object being indexed
        expr->object->accept(this);
        auto object = result;
        
        // Evaluate the index
        expr->index->accept(this);
        auto index = result;
        
        // Make sure we're indexing an array
        if (!isArray(object)) {
            throw std::runtime_error("Cannot index a non-array value");
        }
        
        // Make sure the index is a number
        if (!isNumber(index)) {
            throw std::runtime_error("Array index must be a number");
        }
        
        auto& array = asArray(object);
        int idx = static_cast<int>(asNumber(index));
        
        // Check bounds
        if (idx < 0 || idx >= static_cast<int>(array.size())) {
            throw std::runtime_error("Array index out of bounds: " + std::to_string(idx));
        }
        
        // Return the element at the index
        result = array[idx];
    }

    void visit(AssignIndexExpr* expr) override {
        // Evaluate the object being indexed
        expr->object->accept(this);
        auto object = result;
        
        // Evaluate the index
        expr->index->accept(this);
        auto index = result;
        
        // Evaluate the value to assign
        expr->value->accept(this);
        auto value = result;
        
        // Make sure we're indexing an array
        if (!isArray(object)) {
            throw std::runtime_error("Cannot index a non-array value");
        }
        
        // Make sure the index is a number
        if (!isNumber(index)) {
            throw std::runtime_error("Array index must be a number");
        }
        
        auto& array = asArray(object);
        int idx = static_cast<int>(asNumber(index));
        
        // Check bounds
        if (idx < 0 || idx >= static_cast<int>(array.size())) {
            throw std::runtime_error("Array index out of bounds: " + std::to_string(idx));
        }
        
        // Assign the value to the array element
        array[idx] = value;
        
        // Return the assigned value
        result = value;
    }

    void visit(BlockStmt* stmt) override {
        for (const auto& statement : stmt->statements) {
            if (breakEncountered || continueEncountered) {
                break;
            }
            execute(statement);
        }
    }
    
    void visit(LoopStmt* stmt) override {
        bool oldInLoop = inLoop;
        inLoop = true;
        
        // Evaluate the from expression
        stmt->from->accept(this);
        double fromValue = asNumber(result);
        
        // Evaluate the to expression
        stmt->to->accept(this);
        double toValue = asNumber(result);
        
        // Evaluate the step expression if it exists
        double stepValue = 1.0;
        if (stmt->step) {
            stmt->step->accept(this);
            stepValue = asNumber(result);
        }
        
        bool isDownLoop = stmt->isDownward;
        
        // Set the loop variable
        environment->define(stmt->var.lexeme, makeNumber(fromValue));
        
        // Execute the loop
        while (true) {
            // Check the loop condition
            double currentValue = asNumber(environment->get(stmt->var.lexeme));
            if ((isDownLoop && currentValue < toValue) || (!isDownLoop && currentValue > toValue)) {
                break;
            }
            
            // Execute the body
            execute(stmt->body);
            
            // Handle break
            if (breakEncountered) {
                breakEncountered = false;
                break;
            }
            
            // Reset continue flag
            continueEncountered = false;
            
            // Update the loop variable
            double newValue = currentValue + (isDownLoop ? -stepValue : stepValue);
            environment->assign(stmt->var.lexeme, makeNumber(newValue));
        }
        
        inLoop = oldInLoop;
    }
    
    void visit(BreakStmt* stmt) override {
        if (!inLoop) {
            throw std::runtime_error("Cannot use 'break' outside of a loop.");
        }
        breakEncountered = true;
    }
    
    void visit(ContinueStmt* stmt) override {
        if (!inLoop) {
            throw std::runtime_error("Cannot use 'continue' outside of a loop.");
        }
        continueEncountered = true;
    }
    
    void visit(ExpressionStmt* stmt) override {
        stmt->expression->accept(this);
    }
    
    void visit(CompEqStmt* stmt) override {
        stmt->left->accept(this);
        auto leftValue = result;
        stmt->right->accept(this);
        auto rightValue = result;
        
        result = makeBoolean(isEqual(leftValue, rightValue));
        
        if (isTruthy(result)) {
            execute(stmt->thenBranch);
        } else if (stmt->elseBranch) {
            execute(stmt->elseBranch);
        }
    }
    
    void visit(CompNeqStmt* stmt) override {
        stmt->left->accept(this);
        auto leftValue = result;
        stmt->right->accept(this);
        auto rightValue = result;
        
        result = makeBoolean(!isEqual(leftValue, rightValue));
        
        if (isTruthy(result)) {
            execute(stmt->thenBranch);
        } else if (stmt->elseBranch) {
            execute(stmt->elseBranch);
        }
    }
    
    void visit(CompGeStmt* stmt) override {
        stmt->left->accept(this);
        auto leftValue = result;
        stmt->right->accept(this);
        auto rightValue = result;
        
        bool compResult = false;
        if (isNumber(leftValue) && isNumber(rightValue)) {
            compResult = asNumber(leftValue) >= asNumber(rightValue);
        } else if (isString(leftValue) && isString(rightValue)) {
            compResult = asString(leftValue) >= asString(rightValue);
        } else {
            throw std::runtime_error("Operands must be two numbers or two strings.");
        }
        
        result = makeBoolean(compResult);
        
        if (compResult) {
            execute(stmt->thenBranch);
        } else if (stmt->elseBranch) {
            execute(stmt->elseBranch);
        }
    }
    
    void visit(CompLeStmt* stmt) override {
        stmt->left->accept(this);
        auto leftValue = result;
        stmt->right->accept(this);
        auto rightValue = result;
        
        bool compResult = false;
        if (isNumber(leftValue) && isNumber(rightValue)) {
            compResult = asNumber(leftValue) <= asNumber(rightValue);
        } else if (isString(leftValue) && isString(rightValue)) {
            compResult = asString(leftValue) <= asString(rightValue);
        } else {
            throw std::runtime_error("Operands must be two numbers or two strings.");
        }
        
        result = makeBoolean(compResult);
        
        if (compResult) {
            execute(stmt->thenBranch);
        } else if (stmt->elseBranch) {
            execute(stmt->elseBranch);
        }
    }
    
    void visit(CompGStmt* stmt) override {
        stmt->left->accept(this);
        auto leftValue = result;
        stmt->right->accept(this);
        auto rightValue = result;
        
        bool compResult = false;
        if (isNumber(leftValue) && isNumber(rightValue)) {
            compResult = asNumber(leftValue) > asNumber(rightValue);
        } else if (isString(leftValue) && isString(rightValue)) {
            compResult = asString(leftValue) > asString(rightValue);
        } else {
            throw std::runtime_error("Operands must be two numbers or two strings.");
        }
        
        result = makeBoolean(compResult);
        
        if (compResult) {
            execute(stmt->thenBranch);
        } else if (stmt->elseBranch) {
            execute(stmt->elseBranch);
        }
    }
    
    void visit(CompLStmt* stmt) override {
        stmt->left->accept(this);
        auto leftValue = result;
        stmt->right->accept(this);
        auto rightValue = result;
        
        bool compResult = false;
        if (isNumber(leftValue) && isNumber(rightValue)) {
            compResult = asNumber(leftValue) < asNumber(rightValue);
        } else if (isString(leftValue) && isString(rightValue)) {
            compResult = asString(leftValue) < asString(rightValue);
        } else {
            throw std::runtime_error("Operands must be two numbers or two strings.");
        }
        
        result = makeBoolean(compResult);
        
        if (compResult) {
            execute(stmt->thenBranch);
        } else if (stmt->elseBranch) {
            execute(stmt->elseBranch);
        }
    }
    
    void visit(AndConditionStmt* stmt) override {
        // Evaluate all conditions with short-circuiting
        bool allTrue = true;
        for (const auto& condition : stmt->conditions) {
            execute(condition);
            if (!isTruthy(result)) {
                allTrue = false;
                break;
            }
        }
        
        // Execute appropriate branch
        if (allTrue) {
            execute(stmt->thenBranch);
        } else if (stmt->elseBranch) {
            execute(stmt->elseBranch);
        }
    }
    
    void visit(OrConditionStmt* stmt) override {
        // Evaluate conditions with short-circuiting
        bool anyTrue = false;
        for (const auto& condition : stmt->conditions) {
            execute(condition);
            if (isTruthy(result)) {
                anyTrue = true;
                break;
            }
        }
        
        // Execute appropriate branch
        if (anyTrue) {
            execute(stmt->thenBranch);
        } else if (stmt->elseBranch) {
            execute(stmt->elseBranch);
        }
    }
    
    void visit(AndStmt* stmt) override {
        // Short-circuit evaluation for AND
        stmt->left->accept(this);
        
        if (!isTruthy(result)) {
            // If left side is false, short-circuit
            result = makeBoolean(false);
            if (stmt->elseBranch) {
                execute(stmt->elseBranch);
            }
            return;
        }
        
        // Evaluate right side
        stmt->right->accept(this);
        bool rightResult = isTruthy(result);
        result = makeBoolean(rightResult);
        
        if (rightResult) {
            execute(stmt->thenBranch);
        } else if (stmt->elseBranch) {
            execute(stmt->elseBranch);
        }
    }
    
    void visit(OrStmt* stmt) override {
        // Short-circuit evaluation for OR
        stmt->left->accept(this);
        
        if (isTruthy(result)) {
            // If left side is true, short-circuit
            result = makeBoolean(true);
            execute(stmt->thenBranch);
            return;
        }
        
        // Evaluate right side
        stmt->right->accept(this);
        bool rightResult = isTruthy(result);
        result = makeBoolean(rightResult);
        
        if (rightResult) {
            execute(stmt->thenBranch);
        } else if (stmt->elseBranch) {
            execute(stmt->elseBranch);
        }
    }
    
    void visit(NotStmt* stmt) override {
        stmt->operand->accept(this);
        bool notResult = !isTruthy(result);
        result = makeBoolean(notResult);
        
        if (notResult) {
            execute(stmt->thenBranch);
        } else if (stmt->elseBranch) {
            execute(stmt->elseBranch);
        }
    }
    
    void visit(CompEqExpr* expr) override {
        expr->left->accept(this);
        auto leftValue = result;
        expr->right->accept(this);
        auto rightValue = result;
        
        result = makeBoolean(isEqual(leftValue, rightValue));
    }

    // Helper for converting any value to a string
    std::string valueToString(const Value& value) {
        if (isString(value)) {
            return asString(value);
        } else if (isNumber(value)) {
            double num = asNumber(value);
            if (num == static_cast<int>(num)) {
                // It's a whole number, remove decimal part
                return std::to_string(static_cast<int>(num));
            } else {
                // Format with precision to avoid trailing zeros
                std::ostringstream ss;
                ss << std::fixed << std::setprecision(15) << num;
                std::string str = ss.str();
                // Remove trailing zeros
                str = str.substr(0, str.find_last_not_of('0') + 1);
                // Remove trailing decimal point if needed
                if (str.back() == '.') str.pop_back();
                return str;
            }
        } else if (isBoolean(value)) {
            return asBoolean(value) ? "true" : "false";
        } else if (isArray(value)) {
            // Create string representation of array
            std::string result = "[";
            const auto& array = asArray(value);
            for (size_t i = 0; i < array.size(); i++) {
                result += valueToString(array[i]);
                if (i < array.size() - 1) {
                    result += ", ";
                }
            }
            result += "]";
            return result;
        }
        return "nil";
    }

    // Helper for boolean equality comparison
    bool isEqual(const Value& a, const Value& b) {
        // Check if they're the same object
        if (a == b) return true;
        
        // Different types are never equal
        if (a->type != b->type) return false;
        
        // Same type comparison
        switch (a->type) {
            case ValueImpl::Type::NUMBER:
                return asNumber(a) == asNumber(b);
            case ValueImpl::Type::STRING:
                return asString(a) == asString(b);
            case ValueImpl::Type::BOOLEAN:
                return asBoolean(a) == asBoolean(b);
            case ValueImpl::Type::ARRAY: {
                const auto& arrayA = asArray(a);
                const auto& arrayB = asArray(b);
                
                // Different lengths means different arrays
                if (arrayA.size() != arrayB.size()) return false;
                
                // Compare each element
                for (size_t i = 0; i < arrayA.size(); i++) {
                    if (!isEqual(arrayA[i], arrayB[i])) return false;
                }
                return true;
            }
            default:
                return false;
        }
    }

    // Helper to check number operands
    void checkNumberOperands(const Token& op, 
                            const Value& left,
                            const Value& right) {
        if (isNumber(left) && isNumber(right)) return;
        throw std::runtime_error(std::string("Operands must be numbers for operator '") + 
                                op.lexeme + "'.");
    }

    void visit(PrintStmt *stmt) override
    {
        stmt->expression->accept(this);
        
        std::cout << valueToString(result);
    }

    void visit(VarStmt *stmt) override
    {
        Value value;
        if (stmt->initializer != nullptr)
        {
            stmt->initializer->accept(this);
            value = result;
        }
        else
        {
            value = makeNumber(0.0);
        }
        environment->define(stmt->name.lexeme, value);
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
                environment->define(stmt->variableName.lexeme, makeNumber(value));
            } else {
                // Check for boolean values
                if (input == "true") {
                    environment->define(stmt->variableName.lexeme, makeBoolean(true));
                } else if (input == "false") {
                    environment->define(stmt->variableName.lexeme, makeBoolean(false));
                } else if (input.front() == '[' && input.back() == ']') {
                    // Basic array parsing for input (simple format)
                    std::vector<Value> array;
                    // Parse a simple comma-separated list of values
                    std::string contents = input.substr(1, input.length() - 2);
                    std::istringstream ss(contents);
                    std::string item;
                    
                    while (std::getline(ss, item, ',')) {
                        // Trim spaces
                        item.erase(0, item.find_first_not_of(" \t"));
                        item.erase(item.find_last_not_of(" \t") + 1);
                        
                        // Try to parse as number, boolean, or string
                        if (item == "true") {
                            array.push_back(makeBoolean(true));
                        } else if (item == "false") {
                            array.push_back(makeBoolean(false));
                        } else {
                            try {
                                size_t pos;
                                double num = std::stod(item, &pos);
                                if (pos == item.length()) {
                                    array.push_back(makeNumber(num));
                                } else {
                                    array.push_back(makeString(item));
                                }
                            } catch (const std::exception&) {
                                array.push_back(makeString(item));
                            }
                        }
                    }
                    
                    environment->define(stmt->variableName.lexeme, makeArray(array));
                } else {
                    environment->define(stmt->variableName.lexeme, makeString(input));
                }
            }
        }
        catch (const std::invalid_argument&) {
            // Check for boolean values
            if (input == "true") {
                environment->define(stmt->variableName.lexeme, makeBoolean(true));
            } else if (input == "false") {
                environment->define(stmt->variableName.lexeme, makeBoolean(false));
            } else if (input.front() == '[' && input.back() == ']') {
                // Basic array parsing
                std::vector<Value> array;
                // Very simple parsing - split by commas
                std::string contents = input.substr(1, input.length() - 2);
                std::istringstream ss(contents);
                std::string item;
                
                while (std::getline(ss, item, ',')) {
                    // Trim spaces
                    item.erase(0, item.find_first_not_of(" \t"));
                    item.erase(item.find_last_not_of(" \t") + 1);
                    
                    // Just store everything as strings in this basic version
                    array.push_back(makeString(item));
                }
                
                environment->define(stmt->variableName.lexeme, makeArray(array));
            } else {
                // Not a number or boolean, treat as string
                environment->define(stmt->variableName.lexeme, makeString(input));
            }
        }
        catch (const std::out_of_range&) {
            // Number out of range
            std::cerr << "Warning: Number out of range, treating as string" << std::endl;
            environment->define(stmt->variableName.lexeme, makeString(input));
        }
    }
    
    // Helper to check if a value is truthy
    bool isTruthy(const Value& value) {
        if (isBoolean(value)) { // boolean
            return asBoolean(value);
        } else if (isNumber(value)) { // number
            return asNumber(value) != 0.0;
        } else if (isString(value)) { // string
            return !asString(value).empty();
        } else if (isArray(value)) { // array
            return !asArray(value).empty();
        }
        return false;
    }

    void visit(AssignExpr* expr) override {
        expr->value->accept(this);
        environment->assign(expr->name.lexeme, result);
    }

    // Function declaration visitor
    void visit(FunctionStmt* stmt) override {
        auto function = std::make_shared<AxScriptFunction>(stmt, environment);
        auto value = makeFunction(function);
        environment->define(stmt->name.lexeme, value);
    }

    // Function call visitor
    void visit(CallExpr* expr) override {
        // Evaluate the callee (should be a function)
        expr->callee->accept(this);
        auto callee = result;
        
        if (!isFunction(callee)) {
            throw std::runtime_error("Can only call functions.");
        }
        
        // Evaluate all arguments
        std::vector<Value> arguments;
        for (const auto& arg : expr->arguments) {
            arg->accept(this);
            arguments.push_back(result);
        }
        
        auto function = asFunction(callee);
        
        // Check arity
        if (arguments.size() != function->arity()) {
            throw std::runtime_error(
                "Expected " + std::to_string(function->arity()) + 
                " arguments but got " + std::to_string(arguments.size()) + "."
            );
        }
        
        // Call the function
        result = function->call(this, arguments);
    }

    // Return statement visitor
    void visit(ReturnStmt* stmt) override {
        Value value = makeNumber(0); // Default return value
        
        if (stmt->value != nullptr) {
            stmt->value->accept(this);
            value = result;
        }
        
        // Throw special exception to unwind the call stack
        throw Return(value);
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