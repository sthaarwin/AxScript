#ifndef VISITOR_H
#define VISITOR_H

class NumberExpr;
class StringExpr;
class BinaryExpr;
class PrintStmt;
class VarStmt;
class VariableExpr;
class InputStmt;
class BlockStmt;
class LoopStmt;  
class BreakStmt;
class ContinueStmt;
class ExpressionStmt;
class CompEqStmt;
class CompNeqStmt;
class CompGeStmt;
class CompLeStmt;
class AndStmt;
class OrStmt;
class NotStmt;
class CompEqExpr;
class CompGStmt;
class CompLStmt;
class AndConditionStmt;
class OrConditionStmt;
class AssignExpr;

class Visitor {
public:
    virtual void visit(NumberExpr* expr) = 0;
    virtual void visit(StringExpr* expr) = 0;
    virtual void visit(VariableExpr* expr) = 0; 
    virtual void visit(BinaryExpr* expr) = 0;
    virtual void visit(PrintStmt* stmt) = 0;
    virtual void visit(VarStmt* stmt) = 0;
    virtual void visit(InputStmt* stmt) = 0;
    virtual void visit(BlockStmt* stmt) = 0;
    virtual void visit(LoopStmt* stmt) = 0; 
    virtual void visit(BreakStmt* stmt) = 0;  
    virtual void visit(ContinueStmt* stmt) = 0; 
    virtual void visit(ExpressionStmt* stmt) = 0;
    virtual void visit(CompEqStmt* stmt) = 0;
    virtual void visit(CompNeqStmt* stmt) = 0;
    virtual void visit(CompGeStmt* stmt) = 0;
    virtual void visit(CompLeStmt* stmt) = 0;
    virtual void visit(AndStmt* stmt) = 0;
    virtual void visit(OrStmt* stmt) = 0;
    virtual void visit(NotStmt* stmt) = 0;
    virtual void visit(CompEqExpr* expr) = 0;
    virtual void visit(CompGStmt* stmt) = 0;
    virtual void visit(CompLStmt* stmt) = 0;
    virtual void visit(AndConditionStmt* stmt) = 0;
    virtual void visit(OrConditionStmt* stmt) = 0;
    virtual void visit(AssignExpr* expr) = 0;
};

#endif // VISITOR_H