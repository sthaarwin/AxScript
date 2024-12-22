#pragma once

#include <memory>
#include <vector>
#include <string>
#include "tokens.h"
#include "visitor.h"

class Expr {
public:
    virtual ~Expr() = default;
    virtual void accept(Visitor* visitor) = 0;
};

class BinaryExpr : public Expr {
public:
    std::unique_ptr<Expr> left;
    std::unique_ptr<Expr> right;
    Token op;

    BinaryExpr(std::unique_ptr<Expr> left, Token op, std::unique_ptr<Expr> right)
        : left(std::move(left)), op(op), right(std::move(right)) {}

    void accept(Visitor* visitor) override {
        visitor->visit(this);
    }
};

class NumberExpr : public Expr {
public:
    double value;

    NumberExpr(double value) : value(value) {}

    void accept(Visitor* visitor) override {
        visitor->visit(this);
    }
};

class StringExpr : public Expr {
public:
    std::string value;

    StringExpr(const std::string& value) : value(value) {}

    void accept(Visitor* visitor) override {
        visitor->visit(this);
    }
};

class PrintStmt : public Expr {
public:
    std::unique_ptr<Expr> expression;

    PrintStmt(std::unique_ptr<Expr> expression) : expression(std::move(expression)) {}

    void accept(Visitor* visitor) override {
        visitor->visit(this);
    }
};