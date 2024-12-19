#ifndef SEMANTIC_ANALYZER_H
#define SEMANTIC_ANALYZER_H

#include "ast.h"
#include "symbol_table.h"
#include <memory>
#include <string>

using namespace std;

class SemanticAnalyzer {
public:
    SemanticAnalyzer(unique_ptr<ASTNode>& ast) : ast(move(ast)) {}

    void analyze(const string& OutputFile, const string& ErrorFile, const string& Intmi_codeFile);

    const SymbolTable& getSymbolTable() const { return symbolTable; }

private:
    unique_ptr<ASTNode> ast;
    SymbolTable symbolTable;

    //ofstream outputfile;
    ofstream errorOutput;
    ofstream codeOutput;

    void traverseAST(ASTNode* node,int tmplabel = 0,std::function<void(int)> callback = nullptr);
    void analyzeCompUnit(CompUnitNode* node);
    void analyzeDecl(DeclNode* node);
    void analyzeConstDecl(ConstDeclNode* node);
    void analyzeConstDef(ConstDefNode* node);
    void analyzeVarDecl(VarDeclNode* node);
    void analyzeVarDef(VarDefNode* node);
    void analyzeFuncDef(FuncDefNode* node);
    vector<string> analyzeFuncFParams(FuncFParamsNode *node);
    string analyzeFuncFParam(FuncFParamNode *node);
    void analyzeMainFuncDef(MainFuncDefNode *node);
    void analyzeBlock(BlockNode* node);
    void analyzeStmt(StmtNode* node);
    void analyzeExp(ExpNode* node);
    void analyzeCondExp(CondExpNode* node);
    void analyzeLVal(LValNode* node, bool islight = false);
    void analyzePrimaryExp(PrimaryExpNode* node);
    void analyzeUnaryExp(UnaryExpNode* node);
    string getExpType(ASTNode *node);
    vector<string> getFuncRParamsTypes(FuncRParamsNode *node);
    void analyzeFuncRParams(FuncRParamsNode *node);
    void analyzeMulExp(MulExpNode* node);
    void analyzeAddExp(AddExpNode* node);
    void analyzeRelExp(RelExpNode* node);
    void analyzeEqExp(EqExpNode* node);
    void analyzeLandExp(LandExpNode* node);
    void analyzeLorExp(LorExpNode* node);
    void analyzeNumber(NumberNode* node);
    void analyzeCharacter(CharacterNode* node);
    void analyzeConstExp(ConstExpNode* node);
    void analyzeFor(ForNode* node);
    void analyzeSmallfor(SmallforstmtNode *node);
    // 新增的 analyze 方法
    void analyzeReturnStmt(ReturnStmtNode* node);
    void analyzeBreakStmt(BreakStmtNode* node,int tmplabel);
    void analyzeContinueStmt(ContinueStmtNode* node,int tmplabel);
    void analyzePrintfStmt(PrintfStmtNode* node);
    void analyzeEmptyStmt(EmptyStmtNode* node);
    void analyzeAssignStmt(AssignStmtNode* node);
    void analyzeExpStmt(ExpStmtNode* node);
    void analyzeIfStmt(IfStmtNode* node);

    void reportError(int linenum, const string& errorCode);

   


    /*P_CODE */
    /*中间代码生成*/
    void def_pcode(string type, string name, int scope){
        codeOutput<<"DEF_VAR "<<type<<" "<<name<<endl;
        //codeOutput<<"DEF_VAR "<<type<<" "<<name<<" "<<"scope"<<" "<<scope<<endl;
    }
    void arraysize_pcode(string name, int scope){
        codeOutput<<"STORE_arraysize"<<" "<<name<<endl;
        //codeOutput<<"STORE_arraysize"<<" "<<name<<" "<<"scope"<<" "<<scope<<endl;
    }
    void arrayelement_pcode(string name, int index, int scope){
        codeOutput<<"STORE_arrayelement"<<" "<<name<<" "<<index<<endl;
        //codeOutput<<"STORE_arrayelement"<<" "<<name<<" "<<index<<" "<<"scope"<<" "<<scope<<endl;
    }
    void store_var(string name,int scope){
        codeOutput<<"STORE"<<" "<<name<<endl;
        //codeOutput<<"STORE"<<" "<<name<<" "<<"scope"<<" "<<scope<<endl;
    }
    void funcdef_pcode(string type, string name, int scope){
        codeOutput<<"FUNC_DEF "<<name<<endl;
        //codeOutput<<"FUNC_DEF"<<" "<<type<<" "<<name<<" "<<"scope"<<" "<<scope<<endl;
    }
    void labelfuncend(string name){
        codeOutput<<"LABEL "<<name+"END_FUNC"<<endl;
    }
    void end_func(){
        codeOutput<<"END_FUNC"<<endl;
    }
    void load_var(string name,int scope){
        codeOutput<<"LOAD"<<" "<<name<<endl;
        //codeOutput<<"LOAD"<<" "<<name<<" "<<"scope"<<" "<<scope<<endl;
    }
    void pop_var(string name){
        codeOutput<<"POP_VAR "<<name<<endl;
    }
    void load_arrayelement(string name,int scope){
        codeOutput<<"LOAD_arrayelement"<<" "<<name<<" "<<-1<<endl;
        //codeOutput<<"LOAD_arrayelement"<<" "<<name<<" "<<-1<<" "<<"scope"<<" "<<scope<<endl;
    }
    void store_arrayindex(){
        codeOutput<<"STORE_arrayindex"<<endl;
    }
    void return_pcode(){
        codeOutput<<"RETURN"<<endl;
    }
    void returnnull_pcode(){
        codeOutput<<"RETURN_NULL"<<endl;
    }
    void break_pcode(int scope){
        codeOutput<<"JUMP BREAK"+to_string(scope)<<endl;
    }
    void continue_pcode(int scope){
        codeOutput<<"JUMP CONTINUE"+to_string(scope)<<endl;
    }
    void printf_pcode(string format){
        codeOutput<<"PRINT "<<format<<endl;
    }
    void jumpiffalse_pcode(string label,int scope){
        codeOutput<<"JUMP_IF_FALSE "<<label+to_string(scope)<<endl;
    }
    void shortjumpiffalse_pcode(int scope){
        codeOutput<<"JUMP_IF_FALSE_SHORT "<<"shortval"+to_string(scope)<<endl;
    }
    void shortjumpiftrue_pcode(int scope){
        /*短路求值*/
        codeOutput<<"JUMP_IF_TRUE_SHORT "<<"shortval"+to_string(scope)<<endl;
    }
    void shortlabel(int scope){
        codeOutput<<"LABEL "<<"shortval"+to_string(scope)<<endl;
    }
    void jump_pcode(string label,int scope){
        codeOutput<<"JUMP "<<label+to_string(scope)<<endl;
    }
    void func_call(string name){
        codeOutput<<"CALL "<<name<<endl;
    }
    void load_param(int index,string name){
        codeOutput<<"LOAD_PARAM "<<index<<" "<<name<<endl;
    }
    void load_arrparam(int index,string name){
        codeOutput<<"LOAD_ARRPARAM "<<index<<" "<<name<<endl;
    }
    void label(string label,int scope){
        codeOutput<<"LABEL "<<label+to_string(scope)<<endl;
    }
};

#endif // SEMANTIC_ANALYZER_H