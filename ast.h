#ifndef AST_H
#define AST_H

#include <memory>
#include <vector>
#include <string>
#include "lexer.h"

using namespace std;

enum NodeType {
    NODE_COMPUNIT,
    NODE_DECL,
    NODE_CONSTDECL,
    NODE_CONSTDEF,
    NODE_VARDECL,
    NODE_VARDEF,
    NODE_FUNCDEF,
    NODE_MAINFUNCDEF,
    NODE_BLOCK,
    NODE_STMT,
    NODE_EXP,
    NODE_CONDEXP,
    NODE_LVAL,
    NODE_PRIMARYEXP,
    NODE_UNARYEXP,
    NODE_FUNCRPARAMS,
    NODE_MULEXP,
    NODE_ADDEXP,
    NODE_RELEXP,
    NODE_EQEXP,
    NODE_LANDEXP,
    NODE_LOREXP,
    NODE_NUMBER,
    NODE_CHARACTER,
    NODE_CONSTEXP,
    NODE_RETURNSTMT,  // 新增的节点类型
    NODE_BREAKSTMT,   // 新增的节点类型
    NODE_CONTINUESTMT, // 新增的节点类型
    NODE_PRINTFSTMT,  // 新增的节点类型
    NODE_EMPTYSTMT,   // 新增的节点类型
    NODE_ASSIGNSTMT,  // 新增的节点类型
    NODE_EXPSTMT,     // 新增的节点类型
    NODE_IFSTMT,      // 新增的节点类型
    NODE_FOR,
    NODE_FUNCFPARAM,
    NODE_FUNCFPARAMS,
    NODE_SmallFor,
};

class ASTNode {
public:
    ASTNode(NodeType type) : type(type) {}
    virtual ~ASTNode() {}

    NodeType type;
};

class CompUnitNode : public ASTNode {
public:
    CompUnitNode() : ASTNode(NODE_COMPUNIT) {}
    vector<unique_ptr<ASTNode>> decls;
    vector<unique_ptr<ASTNode>> funcDefs;
    unique_ptr<ASTNode> mainFuncDef;
};

class DeclNode : public ASTNode {
public:
    DeclNode(NodeType type) : ASTNode(type) {}
};

class ConstDeclNode : public DeclNode {
public:
    ConstDeclNode() : DeclNode(NODE_CONSTDECL) {}
    vector<unique_ptr<ASTNode>> constDefs;
};

class ConstDefNode : public ASTNode {
public:
    ConstDefNode() : ASTNode(NODE_CONSTDEF) {}
    string name;
    string constdeftype;
    int linenum;
    vector<unique_ptr<ASTNode>> initVals;
    unique_ptr<ASTNode> arraysize;
    int value_int;
    string value_str;
};

class VarDeclNode : public DeclNode {
public:
    VarDeclNode() : DeclNode(NODE_VARDECL) {}
    vector<unique_ptr<ASTNode>> varDefs;
};

class VarDefNode : public ASTNode {
public:
    VarDefNode() : ASTNode(NODE_VARDEF) {}
    string name;
    string vardeftype;
    int linenum;
    vector<unique_ptr<ASTNode>> initVals;
    unique_ptr<ASTNode> arraysize;
};

class FuncDefNode : public ASTNode {
public:
    FuncDefNode() : ASTNode(NODE_FUNCDEF) {}
    string name;
    string funcdeftype;
    int linenum;
    unique_ptr<ASTNode> block;
    unique_ptr<ASTNode> params; // 新增的成员
};

class MainFuncDefNode : public ASTNode {
public:
    MainFuncDefNode() : ASTNode(NODE_MAINFUNCDEF) {}
    unique_ptr<ASTNode> block;
};

class BlockNode : public ASTNode {
public:
    BlockNode() : ASTNode(NODE_BLOCK) {}
    vector<unique_ptr<ASTNode>> stmts;
    int end_linenum;
    bool isfor;
};

class StmtNode : public ASTNode {
public:
    StmtNode(NodeType type) : ASTNode(type) {}
};

class ExpNode : public ASTNode {
public:
    ExpNode() : ASTNode(NODE_EXP) {}
    // 表达式的具体内容
    int val;
};

class CondExpNode : public ASTNode {
public:
    CondExpNode() : ASTNode(NODE_CONDEXP) {}
    // 条件表达式的具体内容
    int val;
};

class LValNode : public ASTNode {
public:
    LValNode() : ASTNode(NODE_LVAL) {}
    string name;
    //string lvaltype;
    int linenum;
    bool maybeisarray = true;
    unique_ptr<ASTNode> indice;
    int val;
};

class PrimaryExpNode : public ASTNode {
public:
    PrimaryExpNode() : ASTNode(NODE_PRIMARYEXP) {}
    // 基本表达式的具体内容
    int val;
};

class UnaryExpNode : public ASTNode {
public:
    //int linenum;
    UnaryExpNode() : ASTNode(NODE_UNARYEXP) {}
    // 一元表达式的具体内容
    TokenType unaryop;
    unique_ptr<ASTNode> unaryexp;
    int val;
};

class FuncRParamsNode : public ASTNode {
public:
    FuncRParamsNode() : ASTNode(NODE_FUNCRPARAMS) {}
    vector<unique_ptr<ASTNode>> params;
    string name;//函数名
    int linenum;
};

class MulExpNode : public ASTNode {
public:
    MulExpNode() : ASTNode(NODE_MULEXP) {}
    vector<unique_ptr<ASTNode>> operands;
    vector<TokenType> operators;
    int val;
};

class AddExpNode : public ASTNode {
public:
    AddExpNode() : ASTNode(NODE_ADDEXP) {}
    vector<unique_ptr<ASTNode>> operands;
    vector<TokenType> operators;
    int val;
};

class RelExpNode : public ASTNode {
public:
    RelExpNode() : ASTNode(NODE_RELEXP) {}
    vector<unique_ptr<ASTNode>> operands;
    vector<TokenType> operators;
    int val;
};

class EqExpNode : public ASTNode {
public:
    EqExpNode() : ASTNode(NODE_EQEXP) {}
    vector<unique_ptr<ASTNode>> operands;
    vector<TokenType> operators;
    int val;
};

class LandExpNode : public ASTNode {
public:
    LandExpNode() : ASTNode(NODE_LANDEXP) {}
    vector<unique_ptr<ASTNode>> operands;
    vector<TokenType> operators;
    int val;
};

class LorExpNode : public ASTNode {
public:
    LorExpNode() : ASTNode(NODE_LOREXP) {}
    vector<unique_ptr<ASTNode>> operands;
    vector<TokenType> operators;
    int val;
};

class NumberNode : public ASTNode {
public:
    NumberNode() : ASTNode(NODE_NUMBER) {}
    int value;
};

class CharacterNode : public ASTNode {
public:
    CharacterNode() : ASTNode(NODE_CHARACTER) {}
    string value;
};

class ConstExpNode : public ASTNode {
public:
    ConstExpNode() : ASTNode(NODE_CONSTEXP) {}
    // 常量表达式的具体内容
    int val;
};

class ReturnStmtNode : public ASTNode {
public:
    ReturnStmtNode() : ASTNode(NODE_RETURNSTMT) {}
    int linenum;
    unique_ptr<ASTNode> exp;  // 返回的表达式
};

class BreakStmtNode : public ASTNode {
public:
    BreakStmtNode(int lineNum) : ASTNode(NODE_BREAKSTMT), breaklinenum(lineNum) {}
    int breaklinenum;
};

class ContinueStmtNode : public ASTNode {
public:
    ContinueStmtNode(int lineNum) : ASTNode(NODE_CONTINUESTMT), continuelinenum(lineNum) {}
    int continuelinenum;
};

class PrintfStmtNode : public ASTNode {
public:
    PrintfStmtNode() : ASTNode(NODE_PRINTFSTMT) {}
    string format;
    vector<unique_ptr<ASTNode>> args;
    int printlinenum;
};

class EmptyStmtNode : public ASTNode {
public:
    EmptyStmtNode() : ASTNode(NODE_EMPTYSTMT) {}
};

class AssignStmtNode : public ASTNode {
public:
    AssignStmtNode() : ASTNode(NODE_ASSIGNSTMT) {}
    unique_ptr<ASTNode> lval;
    unique_ptr<ASTNode> exp;
    bool getint;
    bool getchar;
};

class ExpStmtNode : public ASTNode {
public:
    ExpStmtNode() : ASTNode(NODE_EXPSTMT) {}
    unique_ptr<ASTNode> exp;
    bool unable;
};

class IfStmtNode : public ASTNode {
public:
    IfStmtNode() : ASTNode(NODE_IFSTMT) {}
    unique_ptr<ASTNode> ifcond;
    unique_ptr<ASTNode> thenStmt;
    unique_ptr<ASTNode> elseStmt;
    bool shortval;
};

class ForNode : public ASTNode { //总的for
public:
    ForNode() : ASTNode(NODE_FOR) {}
    unique_ptr<ASTNode> init;
    unique_ptr<ASTNode> forcond;
    unique_ptr<ASTNode> step;
    unique_ptr<ASTNode> body;
};

class SmallforstmtNode : public ASTNode { //总的for
public:
    SmallforstmtNode() : ASTNode(NODE_SmallFor) {}
    unique_ptr<ASTNode> lval;
    unique_ptr<ASTNode> exp;
};

class FuncFParamNode : public ASTNode { //形参
public:
    FuncFParamNode() : ASTNode(NODE_FUNCFPARAM) {}
    TokenType type; // INTTK or CHARTK
    string realtype;
    string name;
    int linenum;
    bool isArray;
};

class FuncFParamsNode : public ASTNode {   //函数名
public:
    FuncFParamsNode() : ASTNode(NODE_FUNCFPARAMS) {}
    vector<unique_ptr<ASTNode>> params;
};

#endif // AST_H