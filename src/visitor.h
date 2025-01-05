#ifndef VISITOR_H
#define VISITOR_H

class NumberExpr;
class StringExpr;
class BinaryExpr;
class PrintStmt;
class VarStmt;
class VariableExpr;
class InputStmt;
class IfStmt;
class ElseIfStmt;

class Visitor {
public:
    virtual void visit(NumberExpr* expr) = 0;
    virtual void visit(StringExpr* expr) = 0;
    virtual void visit(VariableExpr* expr) = 0; 
    virtual void visit(BinaryExpr* expr) = 0;
    virtual void visit(PrintStmt* stmt) = 0;
    virtual void visit(VarStmt* stmt) = 0;
    virtual void visit(InputStmt* stmt) = 0;
    virtual void visit(IfStmt* stmt) = 0;
    virtual void visit(ElseIfStmt* stmt) = 0;
};

#endif // VISITOR_H