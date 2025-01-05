// interpreter.h
#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "visitor.h"
#include "ast.h"
#include "environment.h"
#include <iostream>
#include <variant>

class Interpreter : public Visitor {
private:
    std::variant<double, std::string> result;
    Environment environment;

public:
    void visit(NumberExpr* expr) override {
        result = expr->value;
    }

    void visit(StringExpr* expr) override {
        result = expr->value;
    }

    void visit(VariableExpr* expr) override {
        result = environment.get(expr->name.lexeme);
    }

    void visit(BinaryExpr* expr) override {
        expr->left->accept(this);
        auto leftValue = result;
        expr->right->accept(this);
        auto rightValue = result;

        switch (expr->op.type) {
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

    void visit(PrintStmt* stmt) override {
        stmt->expression->accept(this);
        std::visit([](auto&& arg) { std::cout << arg << std::endl; }, result);
    }

    void visit(VarStmt* stmt) override {
        std::variant<double, std::string> value;
        if (stmt->initializer != nullptr) {
            stmt->initializer->accept(this);
            value = result;
        } else {
            value = 0.0;
        }
        environment.define(stmt->name.lexeme, value);
    }

    void visit(InputStmt* stmt) override {
        std::string input;
        std::getline(std::cin, input);
        try {
            double value = std::stod(input);
            environment.define(stmt->variableName.lexeme, value);
        } catch (...) {
            environment.define(stmt->variableName.lexeme, input);
        }
    }

    void visit(IfStmt* stmt) override {
        stmt->condition->accept(this);
        if (std::get<double>(result) != 0.0) {
            stmt->thenBranch->accept(this);
        } else if (stmt->elseBranch != nullptr) {
            stmt->elseBranch->accept(this);
        }
    }

    void visit(ElseIfStmt* stmt) override {
        stmt->condition->accept(this);
        if (std::get<double>(result) != 0.0) {
            stmt->thenBranch->accept(this);
        } else if (stmt->elseBranch != nullptr) {
            stmt->elseBranch->accept(this);
        }
    }

    void interpret(const std::vector<std::unique_ptr<Stmt>>& statements) {
        try {
            for (const auto& stmt : statements) {
                execute(stmt);
            }
        } catch (const std::runtime_error& error) {
            std::cerr << "Runtime error: " << error.what() << std::endl;
        }
    }

private:
    void execute(const std::unique_ptr<Stmt>& stmt) {
        stmt->accept(this);
    }
};

#endif