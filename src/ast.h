#ifndef AST_H
#define AST_H

#include <memory>
#include <vector>
#include <string>
#include "tokens.h"
#include "visitor.h"

class Expr
{
public:
    virtual ~Expr() = default;
    virtual void accept(Visitor *visitor) = 0;
};

class BinaryExpr : public Expr
{
public:
    std::unique_ptr<Expr> left;
    std::unique_ptr<Expr> right;
    Token op;

    BinaryExpr(std::unique_ptr<Expr> left, Token op, std::unique_ptr<Expr> right)
        : left(std::move(left)), op(op), right(std::move(right)) {}

    void accept(Visitor *visitor) override
    {
        visitor->visit(this);
    }
};

class NumberExpr : public Expr
{
public:
    double value;

    NumberExpr(double value) : value(value) {}

    void accept(Visitor *visitor) override
    {
        visitor->visit(this);
    }
};

class StringExpr : public Expr
{
public:
    std::string value;

    StringExpr(const std::string &value) : value(value) {}

    void accept(Visitor *visitor) override
    {
        visitor->visit(this);
    }
};

class VariableExpr : public Expr
{
public:
    Token name;

    VariableExpr(Token name) : name(name) {}

    void accept(Visitor *visitor) override
    {
        visitor->visit(this);
    }
};

class CompEqExpr : public Expr {
public:
    std::unique_ptr<Expr> left;
    std::unique_ptr<Expr> right;

    CompEqExpr(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right)
        : left(std::move(left)), right(std::move(right)) {}

    void accept(Visitor* visitor) override {
        visitor->visit(this);
    }
};

class Stmt
{
public:
    virtual ~Stmt() = default;
    virtual void accept(Visitor *visitor) = 0;
};

class PrintStmt : public Stmt
{
public:
    std::unique_ptr<Expr> expression;
    PrintStmt(std::unique_ptr<Expr> expression) : expression(std::move(expression)) {};

    void accept(Visitor *visitor) override
    {
        visitor->visit(this);
    }
};

class VarStmt : public Stmt
{
public:
    Token name;
    std::unique_ptr<Expr> initializer;

    VarStmt(Token name, std::unique_ptr<Expr> initializer) : name(name), initializer(std::move(initializer)) {}

    void accept(Visitor *visitor) override
    {
        visitor->visit(this);
    }
};

class InputStmt : public Stmt
{
public:
    Token variableName;
    InputStmt(Token variablename) : variableName(variablename) {}
    void accept(Visitor *visitor) override
    {
        visitor->visit(this);
    }
};

class BlockStmt : public Stmt {
public:
    std::vector<std::unique_ptr<Stmt>> statements;
    
    BlockStmt(std::vector<std::unique_ptr<Stmt>>&& stmts) 
        : statements(std::move(stmts)) {}
        
    void accept(Visitor* visitor) override {
        visitor->visit(this);
    }
};

class LoopStmt : public Stmt {
public:
    Token var;
    std::unique_ptr<Expr> from;
    std::unique_ptr<Expr> to;
    std::unique_ptr<Expr> step;  // Optional step value
    std::unique_ptr<Stmt> body;
    bool isDownward;  // Indicates if it's counting down

    LoopStmt(Token var, std::unique_ptr<Expr> from, std::unique_ptr<Expr> to, 
             std::unique_ptr<Expr> step, std::unique_ptr<Stmt> body, bool isDownward)
        : var(var), from(std::move(from)), to(std::move(to)), 
          step(std::move(step)), body(std::move(body)), isDownward(isDownward) {}

    void accept(Visitor* visitor) override {
        visitor->visit(this);
    }
};

class BreakStmt : public Stmt {
public:
    BreakStmt() = default;
    void accept(Visitor* visitor) override {
        visitor->visit(this);
    }
};

class ContinueStmt : public Stmt {
public:
    ContinueStmt() = default;
    void accept(Visitor* visitor) override {
        visitor->visit(this);
    }
};

class ExpressionStmt : public Stmt
{
public:
    std::unique_ptr<Expr> expression;

    ExpressionStmt(std::unique_ptr<Expr> expression) : expression(std::move(expression)) {}

    void accept(Visitor *visitor) override
    {
        visitor->visit(this);
    }
};

class CompEqStmt : public Stmt {
public:
    std::unique_ptr<Expr> left;
    std::unique_ptr<Expr> right;
    std::unique_ptr<Stmt> thenBranch;
    std::unique_ptr<Stmt> elseBranch;

    CompEqStmt(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right, 
               std::unique_ptr<Stmt> thenBranch, std::unique_ptr<Stmt> elseBranch = nullptr)
        : left(std::move(left)), right(std::move(right)), 
          thenBranch(std::move(thenBranch)), elseBranch(std::move(elseBranch)) {}

    void accept(Visitor *visitor) override {
        visitor->visit(this);
    }
};

class CompNeqStmt : public Stmt {
public:
    std::unique_ptr<Expr> left;
    std::unique_ptr<Expr> right;
    std::unique_ptr<Stmt> thenBranch;

    CompNeqStmt(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right, std::unique_ptr<Stmt> thenBranch)
        : left(std::move(left)), right(std::move(right)), thenBranch(std::move(thenBranch)) {}

    void accept(Visitor *visitor) override {
        visitor->visit(this);
    }
};

class CompGeStmt : public Stmt {
public:
    std::unique_ptr<Expr> left;
    std::unique_ptr<Expr> right;
    std::unique_ptr<Stmt> thenBranch;

    CompGeStmt(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right, std::unique_ptr<Stmt> thenBranch)
        : left(std::move(left)), right(std::move(right)), thenBranch(std::move(thenBranch)) {}

    void accept(Visitor *visitor) override {
        visitor->visit(this);
    }
};

class CompLeStmt : public Stmt {
public:
    std::unique_ptr<Expr> left;
    std::unique_ptr<Expr> right;
    std::unique_ptr<Stmt> thenBranch;

    CompLeStmt(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right, std::unique_ptr<Stmt> thenBranch)
        : left(std::move(left)), right(std::move(right)), thenBranch(std::move(thenBranch)) {}

    void accept(Visitor *visitor) override {
        visitor->visit(this);
    }
};

class CompGStmt : public Stmt {
public:
    std::unique_ptr<Expr> left;
    std::unique_ptr<Expr> right;
    std::unique_ptr<Stmt> thenBranch;

    CompGStmt(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right, std::unique_ptr<Stmt> thenBranch)
        : left(std::move(left)), right(std::move(right)), thenBranch(std::move(thenBranch)) {}

    void accept(Visitor *visitor) override {
        visitor->visit(this);
    }
};

class CompLStmt : public Stmt {
public:
    std::unique_ptr<Expr> left;
    std::unique_ptr<Expr> right;
    std::unique_ptr<Stmt> thenBranch;

    CompLStmt(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right, std::unique_ptr<Stmt> thenBranch)
        : left(std::move(left)), right(std::move(right)), thenBranch(std::move(thenBranch)) {}

    void accept(Visitor *visitor) override {
        visitor->visit(this);
    }
};

class AndStmt : public Stmt {
public:
    std::unique_ptr<Expr> left;
    std::unique_ptr<Expr> right;
    std::unique_ptr<Stmt> thenBranch;

    AndStmt(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right, std::unique_ptr<Stmt> thenBranch)
        : left(std::move(left)), right(std::move(right)), thenBranch(std::move(thenBranch)) {}

    void accept(Visitor *visitor) override {
        visitor->visit(this);
    }
};

class OrStmt : public Stmt {
public:
    std::unique_ptr<Expr> left;
    std::unique_ptr<Expr> right;
    std::unique_ptr<Stmt> thenBranch;

    OrStmt(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right, std::unique_ptr<Stmt> thenBranch)
        : left(std::move(left)), right(std::move(right)), thenBranch(std::move(thenBranch)) {}

    void accept(Visitor *visitor) override {
        visitor->visit(this);
    }
};

class NotStmt : public Stmt {
public:
    std::unique_ptr<Expr> operand;
    std::unique_ptr<Stmt> thenBranch;

    NotStmt(std::unique_ptr<Expr> operand, std::unique_ptr<Stmt> thenBranch)
        : operand(std::move(operand)), thenBranch(std::move(thenBranch)) {}

    void accept(Visitor *visitor) override {
        visitor->visit(this);
    }
};

class ConditionStmt : public Stmt {
public:
    std::vector<std::unique_ptr<Stmt>> conditions;
    std::unique_ptr<Stmt> thenBranch;
    std::unique_ptr<Stmt> elseBranch;  // Add else branch

    ConditionStmt(std::vector<std::unique_ptr<Stmt>>&& conditions, 
                  std::unique_ptr<Stmt> thenBranch,
                  std::unique_ptr<Stmt> elseBranch = nullptr)
        : conditions(std::move(conditions)), 
          thenBranch(std::move(thenBranch)),
          elseBranch(std::move(elseBranch)) {}

    virtual ~ConditionStmt() = default;
};

class AndConditionStmt : public ConditionStmt {
public:
    using ConditionStmt::ConditionStmt;
    void accept(Visitor* visitor) override {
        visitor->visit(this);
    }
};

class OrConditionStmt : public ConditionStmt {
public:
    using ConditionStmt::ConditionStmt;
    void accept(Visitor* visitor) override {
        visitor->visit(this);
    }
};

#endif // AST_H