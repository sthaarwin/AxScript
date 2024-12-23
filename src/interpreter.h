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

    void visit(BinaryExpr* expr) override {
        expr->left->accept(this);
        auto leftValue = result;
        expr->right->accept(this);
        auto rightValue = result;

        if (expr->op.type == TokenType::PLUS) {
            result = std::get<double>(leftValue) + std::get<double>(rightValue);
        } else if (expr->op.type == TokenType::MINUS) {
            result = std::get<double>(leftValue) - std::get<double>(rightValue);
        } else if (expr->op.type == TokenType::STAR) {
            result = std::get<double>(leftValue) * std::get<double>(rightValue);
        } else if (expr->op.type == TokenType::SLASH) {
            result = std::get<double>(leftValue) / std::get<double>(rightValue);
        }
    }

    void visit(PrintStmt* stmt) override {
        stmt->expression->accept(this);
        std::visit([](auto&& arg) { std::cout << arg << std::endl; }, result);
    }

    std::variant<double, std::string> interpret(std::unique_ptr<Stmt>& expr) {
        expr->accept(this);
        return result;
    }

    void visit(VarStmt* stmt)override{
        std::variant<double, std::string> value;
        if(stmt -> initializer != nullptr){
            stmt -> initializer -> accept(this);
            value = result;
        }else{
            value = 0.0;
        }
        environment.define(stmt -> name.lexeme, value);
    }
};

#endif //INTERPRETER_H