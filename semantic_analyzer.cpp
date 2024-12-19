#include "semantic_analyzer.h"
#include "symbol_table.h"
#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include "parser.h"
#include <sstream>

using namespace std;

int blocks2level = 0;
int funcLevel = 0;
int labelscope = 0;
int labelfor_bk_ctn = 0;
int if_order = 0;
int continue_order = 0;
int break_continu = 0;
int shortvalorder = 0;
vector<string> return_pop_varsparams;
vector<string> return_pop_varsin;

std::vector<int> hasReturnStatement(ASTNode* node) {
    std::vector<int> returnLines;
    if (!node) return returnLines;

    switch (node->type) {
        case NODE_FUNCDEF: {
            auto funcDefNode = static_cast<FuncDefNode*>(node);
            auto blockReturnLines = hasReturnStatement(funcDefNode->block.get());
            returnLines.insert(returnLines.end(), blockReturnLines.begin(), blockReturnLines.end());
            break;
        }
        case NODE_BLOCK: {
            auto blockNode = static_cast<BlockNode*>(node);
            for (auto& stmt : blockNode->stmts) {
                auto stmtReturnLines = hasReturnStatement(stmt.get());
                returnLines.insert(returnLines.end(), stmtReturnLines.begin(), stmtReturnLines.end());
            }
            break;
        }
        case NODE_IFSTMT: {
            auto ifStmtNode = static_cast<IfStmtNode*>(node);
            auto thenReturnLines = hasReturnStatement(ifStmtNode->thenStmt.get());
            returnLines.insert(returnLines.end(), thenReturnLines.begin(), thenReturnLines.end());

            if (ifStmtNode->elseStmt) {
                auto elseReturnLines = hasReturnStatement(ifStmtNode->elseStmt.get());
                returnLines.insert(returnLines.end(), elseReturnLines.begin(), elseReturnLines.end());
            }
            break;
        }
        case NODE_RETURNSTMT: {
            auto returnNode = static_cast<ReturnStmtNode*>(node);
            if (returnNode->exp)
                returnLines.push_back(returnNode->linenum);
            break;
        }
        default:
            break;
    }

    return returnLines;
}

int checkMainFunctionReturn(ASTNode* node) {
    if (!node) return -1;

    switch (node->type) {
        case NODE_MAINFUNCDEF: {
            auto mainFuncDefNode = static_cast<MainFuncDefNode*>(node);
            return checkMainFunctionReturn(mainFuncDefNode->block.get());
        }
        case NODE_FUNCDEF: {
            auto funcDefNode = static_cast<FuncDefNode*>(node);
            return checkMainFunctionReturn(funcDefNode->block.get());
        }
        case NODE_BLOCK: {
            auto blockNode = static_cast<BlockNode*>(node);
            for (auto& stmt : blockNode->stmts) {
                if (stmt->type == NODE_RETURNSTMT) {
                    return -1; // 如果找到 return 语句，返回 -1
                }
            }
            return blockNode->end_linenum; // 如果没有找到 return 语句，返回 block 的结束行号
        }
        default:
            return -1; // 其他节点类型，返回 -1 表示无效
    }
}

void SemanticAnalyzer::analyze(const string& Sem_OutputFile, const string& Sem_ErrorFile, const string& Intmi_codeFile) {
    errorOutput.open(Sem_ErrorFile);
    codeOutput.open(Intmi_codeFile);
    symbolTable.enterScope(++blocks2level); 
    traverseAST(ast.get());
    symbolTable.dumpSymbolTable(Sem_OutputFile);
    errorOutput.close();
    codeOutput.close();
}

void SymbolTable::dumpSymbolTable(const string& filename) const {
    ofstream outputFile(filename);
    if (!outputFile.is_open()) {
        cerr << "Error: Could not open file " << filename << " for writing." << endl;
        return;
    }
    vector<SymbolEntry> entries;
    // 收集所有符号表中的条目
    for (int i = 0; i < smb_size; ++i) {
        for (const auto& pair : scopeStack[i]) {
            entries.push_back(pair.second);
        }
    }

    // 按照 scopeLevel 和 order 进行排序
    sort(entries.begin(), entries.end(), [](const SymbolEntry& a, const SymbolEntry& b) {
        if (a.scopeLevel != b.scopeLevel) {
            return a.scopeLevel < b.scopeLevel;
        }
        return a.order < b.order;
    });

    // 输出排序后的条目
    for (const auto& entry : entries) {
        outputFile<< entry.scopeLevel <<" "<< entry.name <<" "<< entry.type <<endl;
    }

    outputFile.close();
}

void SemanticAnalyzer::traverseAST(ASTNode* node, int tmplabel, std::function<void(int)> callback) {
    if (!node) {
        //cout<<"null node !!!"<<endl;
        return;
    }
    switch (node->type) {
        case NODE_COMPUNIT:
            //cout<<"analyzer:Compunit"<<endl;
            analyzeCompUnit(static_cast<CompUnitNode*>(node));
            break;
        case NODE_DECL:
            //cout<<"analyzer:decl"<<endl;
            analyzeDecl(static_cast<DeclNode*>(node));
            break;
        case NODE_CONSTDECL:
            //cout<<"analyzer:constdecl"<<endl;
            analyzeConstDecl(static_cast<ConstDeclNode*>(node));
            break;
        case NODE_CONSTDEF:
            //cout<<"analyzer:constdef"<<endl;
            analyzeConstDef(static_cast<ConstDefNode*>(node));
            break;
        case NODE_VARDECL:
            //cout<<"analyzer:vardecl"<<endl;
            analyzeVarDecl(static_cast<VarDeclNode*>(node));
            break;
        case NODE_VARDEF:
            //cout<<"analyzer:vardef"<<endl;
            analyzeVarDef(static_cast<VarDefNode*>(node));
            break;
        case NODE_FUNCDEF:
            //cout<<"analyzer:funcdef"<<endl;
            analyzeFuncDef(static_cast<FuncDefNode*>(node));
            break;
        case NODE_MAINFUNCDEF:
            //cout<<"analyzer:mainfuncdef"<<endl;
            analyzeMainFuncDef(static_cast<MainFuncDefNode*>(node));
            break;
        case NODE_BLOCK:
            //cout<<"analyzer:block"<<endl;
            analyzeBlock(static_cast<BlockNode*>(node));
            break;
        case NODE_STMT:
            //cout<<"analyzer:stmt"<<endl;
            analyzeStmt(static_cast<StmtNode*>(node));
            break;
        case NODE_EXP:
            //cout<<"analyzer:exp"<<endl;
            analyzeExp(static_cast<ExpNode*>(node));
            break;
        case NODE_CONDEXP:
            //cout<<"analyzer:condexp"<<endl;
            analyzeCondExp(static_cast<CondExpNode*>(node));
            break;
        case NODE_LVAL:
            //cout<<"analyzer:lval"<<endl;
            analyzeLVal(static_cast<LValNode*>(node));
            break;
        case NODE_PRIMARYEXP:
            //cout<<"analyzer:primaryexp"<<endl;
            analyzePrimaryExp(static_cast<PrimaryExpNode*>(node));
            break;
        case NODE_UNARYEXP:
            //cout<<"analyzer:unaryexp"<<endl;
            analyzeUnaryExp(static_cast<UnaryExpNode*>(node));
            break;
        case NODE_FUNCRPARAMS:
            //cout<<"analyzer:funcRparams"<<endl;
            analyzeFuncRParams(static_cast<FuncRParamsNode*>(node));
            break;
        case NODE_MULEXP:
            //cout<<"analyzer:mulexp"<<endl;
            analyzeMulExp(static_cast<MulExpNode*>(node));
            break;
        case NODE_ADDEXP:
            //cout<<"analyzer:addexp"<<endl;
            analyzeAddExp(static_cast<AddExpNode*>(node));
            break;
        case NODE_RELEXP:
            //cout<<"analyzer:relexp"<<endl;
            analyzeRelExp(static_cast<RelExpNode*>(node));
            break;
        case NODE_EQEXP:
            //cout<<"analyzer:eqexp"<<endl;
            analyzeEqExp(static_cast<EqExpNode*>(node));
            break;
        case NODE_LANDEXP:
            //cout<<"analyzer:landexp"<<endl;
            analyzeLandExp(static_cast<LandExpNode*>(node));
            break;
        case NODE_LOREXP:
            //cout<<"analyzer:lorexp"<<endl;
            analyzeLorExp(static_cast<LorExpNode*>(node));
            break;
        case NODE_NUMBER:{
            auto numberNode = static_cast<NumberNode*>(node);
            //cout<<"analyzer:number"<<endl;
            if (callback)
                callback(numberNode->value);
            analyzeNumber(numberNode);
            break;
        }
        case NODE_CHARACTER:
            //cout<<"analyzer:character"<<endl;
            analyzeCharacter(static_cast<CharacterNode*>(node));
            break;
        case NODE_CONSTEXP:
            //cout<<"analyzer:constexp"<<endl;
            analyzeConstExp(static_cast<ConstExpNode*>(node));
            break;
        case NODE_FOR:
            //cout<<"analyzer:forstmt"<<endl;
            analyzeFor(static_cast<ForNode*>(node));
            break;
        case NODE_RETURNSTMT:
            analyzeReturnStmt(static_cast<ReturnStmtNode*>(node));
            break;
        case NODE_BREAKSTMT:
            analyzeBreakStmt(static_cast<BreakStmtNode*>(node),tmplabel);
            break;
        case NODE_CONTINUESTMT:
            analyzeContinueStmt(static_cast<ContinueStmtNode*>(node),tmplabel);
            break;
        case NODE_PRINTFSTMT:
            analyzePrintfStmt(static_cast<PrintfStmtNode*>(node));
            break;
        case NODE_EMPTYSTMT:
            analyzeEmptyStmt(static_cast<EmptyStmtNode*>(node));
            break;
        case NODE_ASSIGNSTMT:   //左值
            analyzeAssignStmt(static_cast<AssignStmtNode*>(node));
            break;
        case NODE_EXPSTMT:
            analyzeExpStmt(static_cast<ExpStmtNode*>(node));
            break;
        case NODE_IFSTMT:
            analyzeIfStmt(static_cast<IfStmtNode*>(node));
            break;
        // 其他节点类型
    }
}

void SemanticAnalyzer::analyzeCompUnit(CompUnitNode* node) {
    if (!node) return;
    for (auto& decl : node->decls) {
        traverseAST(decl.get());
    }
    for (auto& funcDef : node->funcDefs) {
        traverseAST(funcDef.get());
    }
    traverseAST(node->mainFuncDef.get());
}

void SemanticAnalyzer::analyzeDecl(DeclNode* node) {
    if (!node) return;
    switch (node->type) {
        case NODE_CONSTDECL:
            analyzeConstDecl(static_cast<ConstDeclNode*>(node));
            break;
        case NODE_VARDECL:
            analyzeVarDecl(static_cast<VarDeclNode*>(node));
            break;
        // 其他声明类型
    }
}

void SemanticAnalyzer::analyzeConstDecl(ConstDeclNode* node) {
    if (!node) return;
    for (auto& constDef : node->constDefs) {
        traverseAST(constDef.get());
    }
}

void SemanticAnalyzer::analyzeConstDef(ConstDefNode* node) {
    //codeOutput<<"constdef"<<endl;
    if (!node) return;
    // 检查常量定义的语义
    SymbolEntry entry;
    entry.name = node->name;
    entry.type = node->constdeftype;
    entry.isConst = true;
    entry.isFunction = false;
    entry.paramTypes = {};
    bool foundEntry = symbolTable.Isrepeated(entry.name);
    int tmpscope = -1;
    if (foundEntry) {
        reportError(node->linenum, "b");
    } else {
        symbolTable.addSymbol(entry);
        const SymbolEntry& lastAdded = symbolTable.getLastAddedSymbol();
        tmpscope = lastAdded.scopeLevel;
    }

    def_pcode(node->constdeftype,node->name,tmpscope);
    if(node->arraysize){
        traverseAST(node->arraysize.get());
        arraysize_pcode(node->name,tmpscope);
        for(int i=0;i<node->initVals.size();++i){
            traverseAST(node->initVals[i].get());
            arrayelement_pcode(node->name,i,tmpscope);
        }
    } else {
        traverseAST(node->initVals[0].get());
        store_var(node->name,tmpscope);
    }
}

void SemanticAnalyzer::analyzeVarDecl(VarDeclNode* node) {
    if (!node) return;
    for (auto& varDef : node->varDefs) {
        traverseAST(varDef.get());
    }
}

void SemanticAnalyzer::analyzeVarDef(VarDefNode* node) {
    if (!node) return;
    // 检查变量定义的语义
    SymbolEntry entry;
    entry.name = node->name;
    entry.type = node->vardeftype;
    entry.isConst = false;
    int tmpscope = -1;
    if (symbolTable.Isrepeated(entry.name)) {
        // 名字重定义错误
        reportError(node->linenum, "b");
    } else {
        symbolTable.addSymbol(entry);
        const SymbolEntry& lastAdded = symbolTable.getLastAddedSymbol();
        tmpscope = lastAdded.scopeLevel;
    }

    def_pcode(node->vardeftype,node->name,tmpscope);
    
    if(node->arraysize!=nullptr){
        traverseAST(node->arraysize.get());
        arraysize_pcode(node->name,tmpscope);
        if(node->initVals.size()!=0){
            for(int i=0;i<node->initVals.size();++i){
                traverseAST(node->initVals[i].get());
                arrayelement_pcode(node->name,i,tmpscope);
            }
        }
    } else {
        if(node->initVals.size()!=0){
            for(int i=0;i<node->initVals.size();++i){
                traverseAST(node->initVals[i].get());
                store_var(node->name,tmpscope);
            }
        }
    }
}

std::vector<std::string> getTopLevelDefNames(BlockNode* blockNode);

void SemanticAnalyzer::analyzeFuncDef(FuncDefNode* node) {
    if (!node) return;
    // 检查函数定义的语义
    int level = symbolTable.getCurrentLevel();
    funcLevel = blocks2level+1;
    SymbolEntry entry;
    entry.name = node->name;
    //cout<<"func name is "<<entry.name<<endl;
    entry.type = node->funcdeftype;
    entry.isFunction = true;
    entry.paramTypes = {};
    int tmpscope = -1;
    if (symbolTable.Isrepeated(entry.name)) {
        // 名字重定义错误
        reportError(node->linenum, "b");
    } else {
        symbolTable.addSymbol(entry);
        const SymbolEntry& lastAdded = symbolTable.getLastAddedSymbol();
        tmpscope = lastAdded.scopeLevel;
    }

    funcdef_pcode(node->funcdeftype, node->name,tmpscope);
    codeOutput<<"JUMP "+node->name+"END_FUNC"<<endl;

    // 处理函数的参数
    if (node->params) {
        entry.paramTypes = analyzeFuncFParams(static_cast<FuncFParamsNode*>(node->params.get()));
        symbolTable.insertparamtypes(entry);
    }
    /*中间代码*/
    codeOutput<<"FUNCBLOCKNOW"<<endl;/*进入func的block，记录numstack数量用于无效元素退栈*/
    //f
    if(entry.type == "VoidFunc"){
        vector<int> errlines = hasReturnStatement(node);
        if(errlines.size()!=0){
            for(auto it:errlines){
                reportError(it,"f");
            }
        }
    }

    labelscope = symbolTable.getCurrentLevel();
    if(node->params){
        auto pamnode = static_cast<FuncFParamsNode*>(node->params.get());
        for(int i=0;i<pamnode->params.size();i++){
            auto paramnode = static_cast<FuncFParamNode*>(pamnode->params[i].get());
            return_pop_varsparams.push_back(paramnode->name); /*需要pop定义的变量 */
        }
    }
    
    
    //处理语句块
    auto blocknode = static_cast<BlockNode*>(node->block.get());
    return_pop_varsin = getTopLevelDefNames(blocknode);
    traverseAST(node->block.get());
    
    
    
    if(node->funcdeftype != "VoidFunc"){
        int checkreturn = checkMainFunctionReturn(node);
        if(checkreturn != -1){
            reportError(checkreturn,"g");
        }
    }

    /*生成中间代码*/
    
    if(return_pop_varsparams.size()){
        for(auto it :return_pop_varsparams){
            pop_var(it);
        }
    }
    return_pop_varsparams.clear();
    return_pop_varsin.clear();

    labelfuncend(node->name);
    end_func();
    symbolTable.exitScope(level);
}

vector<string> SemanticAnalyzer::analyzeFuncFParams(FuncFParamsNode* node) {
    vector<string> types;
    int arrvarnumorder = 0;
    int varnumorder = 0;
    for(int i=0;i<node->params.size();i++){
        auto paramnode = static_cast<FuncFParamNode*>(node->params[i].get());
        if(paramnode->realtype.find("Array")!=string::npos){
            arrvarnumorder++;
        }else{
            varnumorder++;
        }
    }
    int tmparr=0;
    int tmpvar=0;
    for(int i=0;i<node->params.size();i++){
        auto paramnode = static_cast<FuncFParamNode*>(node->params[i].get());
        types.push_back(analyzeFuncFParam(paramnode));
        def_pcode(paramnode->realtype,paramnode->name,0);/*标*/
        if(paramnode->realtype.find("Array")!=string::npos){
            codeOutput<<"STORE_funcf_arraysize "+paramnode->name<<endl;
            load_arrparam(arrvarnumorder-1-tmparr,paramnode->name);
            tmparr++;
        } else {
            load_param(varnumorder-1-tmpvar,paramnode->name);
            tmpvar++;
        }    //store_var(paramnode->name,0);
    }
    
    return types;
}

string SemanticAnalyzer::analyzeFuncFParam(FuncFParamNode* node) {
    // 检查函数参数的语义
    SymbolEntry entry;
    entry.name = node->name;
    if(!node->isArray){
        if(node->type == INTTK)
            entry.type = "Int";
        else if(node->type == CHARTK)
            entry.type = "Char";
    } else {
        if(node->type == INTTK)
            entry.type = "IntArray";
        else if(node->type == CHARTK)
            entry.type = "CharArray";
    }
    entry.isConst = false;
    entry.isFunction = false;
    entry.isArray = node->isArray;
    symbolTable.enterScope(blocks2level+1);//形参作用域

    int tmpscope = -1;

    if (symbolTable.Isrepeated(entry.name)) {
        // 名字重定义错误
        reportError(node->linenum, "b");
    } else {
        symbolTable.addSymbol(entry);
        const SymbolEntry& lastAdded = symbolTable.getLastAddedSymbol();
        tmpscope = lastAdded.scopeLevel;
    }

    //def_pcode(entry.type,entry.name,tmpscope); //传参不会有数组？

    return entry.type;
}

void SemanticAnalyzer::analyzeMainFuncDef(MainFuncDefNode* node) {
    if (!node) return;
    // 检查主函数定义的语义
    funcdef_pcode("","main",0);
    funcLevel = blocks2level+1;
    traverseAST(node->block.get());
    int checkreturn = checkMainFunctionReturn(node);
    if(checkreturn != -1){
        reportError(checkreturn,"g");
    }
    labelfuncend("main");
    end_func();
}
//m
vector<int> hasContinueOrBreak(BlockNode* blockNode) {
    std::vector<int> result;
    if (!blockNode) return result;

    for (auto& stmt : blockNode->stmts) {
        if (!stmt) continue;
        switch (stmt->type) {
            case NODE_BREAKSTMT: {
                auto breakstmt = static_cast<BreakStmtNode*>(stmt.get());
                result.push_back(breakstmt->breaklinenum);
                break;
            }
            case NODE_CONTINUESTMT: {
                auto continuestmt = static_cast<ContinueStmtNode*>(stmt.get());
                result.push_back(continuestmt->continuelinenum);
                break;
            }
            case NODE_BLOCK: {
                std::vector<int> nestedResult = hasContinueOrBreak(static_cast<BlockNode*>(stmt.get()));
                result.insert(result.end(), nestedResult.begin(), nestedResult.end());
                break;
            }
            default:
                break;
        }
    }

    return result;
}
/*做POP_VAR用的 */
std::vector<std::string> getTopLevelDefNames(BlockNode* blockNode) {
    std::vector<std::string> defNames;
    if (!blockNode) return defNames;

    for (auto& stmt : blockNode->stmts) {
        if (stmt->type == NODE_CONSTDECL || stmt->type == NODE_VARDECL) {
            // 处理常量声明或变量声明
            if (stmt->type == NODE_CONSTDECL) {
                auto constDeclNode = static_cast<ConstDeclNode*>(stmt.get());
                for (auto& constDef : constDeclNode->constDefs) {
                    auto constDefNode = static_cast<ConstDefNode*>(constDef.get());
                    defNames.push_back(constDefNode->name);
                }
            } else if (stmt->type == NODE_VARDECL) {
                auto varDeclNode = static_cast<VarDeclNode*>(stmt.get());
                for (auto& varDef : varDeclNode->varDefs) {
                    auto varDefNode = static_cast<VarDefNode*>(varDef.get());
                    defNames.push_back(varDefNode->name);
                }
            }
        } else if (stmt->type == NODE_BLOCK) {
            // 忽略嵌套的 BlockNode
            continue;
        }
    }

    return defNames;
}
/*标*/
void SemanticAnalyzer::analyzeBlock(BlockNode* node) {
    if (!node) return;
    //stmts包含decl和stmt
    int level = symbolTable.getCurrentLevel();
    symbolTable.enterScope(++blocks2level);
    for (auto& stmt : node->stmts) {
        traverseAST(stmt.get());
    }
    
    //错误m
    if(!node->isfor){
        vector<int> errline = hasContinueOrBreak(node);
        if(errline.size()!=0){
            for(auto line: errline){
                reportError(line,"m");
            }
        }
    }

    labelscope = symbolTable.getCurrentLevel();
    /*生成中间代码*/
    vector<string> names = getTopLevelDefNames(node);
    /*有问题*/
    for(auto popvarname: names){
        pop_var(popvarname);
    }

    symbolTable.exitScope(level);
}

void SemanticAnalyzer::analyzeStmt(StmtNode* node) {
    if (!node) return;
    cout<<"analyze stmt"<<endl;
    // 检查语句的语义
    //语法分析无返回
}

void SemanticAnalyzer::analyzeExp(ExpNode* node) {
    if (!node) return;
    cout << "analyze exp" << endl;
    // 检查表达式的语义
    // 语法分析的时候就转add了
}

void SemanticAnalyzer::analyzeCondExp(CondExpNode* node) {
    if (!node) return;
    // 检查条件表达式的语义
    //语法没有返回这个
}

void SemanticAnalyzer::analyzeLVal(LValNode* node, bool islight) {
    if (!node) return;
    //cout<<"LVAL"<<endl;
    
    // 检查左值的语义
    auto symbol = symbolTable.Isundefined(node->name,funcLevel);
    if (symbol) {
        reportError(node->linenum, "c");
    }

    auto entry = symbolTable.lookup(node->name);
    // 分析索引表达式
    if(node->indice){
        traverseAST(node->indice.get()); //类似于a[0]的[0]
        if(entry){
            store_arrayindex();
            if(islight){
                arrayelement_pcode(entry->name,-1,entry->scopeLevel);
                //-1代表去取index
            } else {
                load_arrayelement(entry->name,entry->scopeLevel);
            }
        }
    }else{
        if(entry){
            if(islight){
                store_var(entry->name,entry->scopeLevel);
            } else {
                load_var(entry->name,entry->scopeLevel);
            }
        }
    }
    
}

void SemanticAnalyzer::analyzePrimaryExp(PrimaryExpNode* node) {
    if (!node) return;
    // 检查基本表达式的语义
    //语法没有返回这个
}

void SemanticAnalyzer::analyzeUnaryExp(UnaryExpNode* node) {
    if (!node) return;
    // 检查一元表达式的语义
    if(node->unaryexp){
        traverseAST(node->unaryexp.get());
    }

    if(node->unaryop){
        switch (node->unaryop)
        {
        case PLUS:
            codeOutput<<"ZHENG"<<endl;
            break;
        case MINU:
            codeOutput<<"FU"<<endl;
            break;
        case NOT:{
            codeOutput<<"FEI"<<endl;
            break;
        }
        default:
            break;
        }
    }
}
/*
处理d和e的函数
*/
string SemanticAnalyzer::getExpType(ASTNode* node) {
    if (!node) return "";

    switch (node->type) {
        case NODE_NUMBER:
            return "Int";
        case NODE_CHARACTER:
            return "Char";
        case NODE_LVAL: {
            LValNode* lvalNode = dynamic_cast<LValNode*>(node);
            if (!lvalNode) return "";
            auto entry = symbolTable.lookup(lvalNode->name);
            if (entry) {
                if(entry->type == "IntArray" && !lvalNode->maybeisarray)
                    return "Int";
                else if(entry->type == "CharArray" && !lvalNode->maybeisarray)
                    return "Char";
                return entry->type;
            } else {
                return "";
            }
        }
        case NODE_FUNCRPARAMS: {
            FuncRParamsNode* funcRParamsNode = dynamic_cast<FuncRParamsNode*>(node);
            if (!funcRParamsNode) return "";
            auto entry = symbolTable.lookup(funcRParamsNode->name);
            if (entry) {
                if (entry->type == "VoidFunc")
                    return "Void";
                else if (entry->type == "IntFunc")
                    return "Int";
                else if (entry->type == "CharFunc")
                    return "Char";
                else
                    return "Error";
            } else {
                return "";
            }
        }
        case NODE_ADDEXP: {
            AddExpNode* addExpNode = dynamic_cast<AddExpNode*>(node);
            if (!addExpNode || addExpNode->operands.empty()) return "";
            return getExpType(addExpNode->operands[0].get());
        }
        case NODE_MULEXP: {
            MulExpNode* mulExpNode = dynamic_cast<MulExpNode*>(node);
            if (!mulExpNode || mulExpNode->operands.empty()) return "";
            return getExpType(mulExpNode->operands[0].get());
        }
        case NODE_RELEXP: {
            RelExpNode* relExpNode = dynamic_cast<RelExpNode*>(node);
            if (!relExpNode || relExpNode->operands.empty()) return "";
            return getExpType(relExpNode->operands[0].get());
        }
        case NODE_EQEXP: {
            EqExpNode* eqExpNode = dynamic_cast<EqExpNode*>(node);
            if (!eqExpNode || eqExpNode->operands.empty()) return "";
            return getExpType(eqExpNode->operands[0].get());
        }
        case NODE_LANDEXP: {
            LandExpNode* landExpNode = dynamic_cast<LandExpNode*>(node);
            if (!landExpNode || landExpNode->operands.empty()) return "";
            return getExpType(landExpNode->operands[0].get());
        }
        case NODE_LOREXP: {
            LorExpNode* lorExpNode = dynamic_cast<LorExpNode*>(node);
            if (!lorExpNode || lorExpNode->operands.empty()) return "";
            return getExpType(lorExpNode->operands[0].get());
        }
        // 其他类型可以根据需要继续添加
        default:
            return "";
    }
}

vector<string> SemanticAnalyzer::getFuncRParamsTypes(FuncRParamsNode* node) {
    vector<string> paramTypes;
    if (!node || node->params.empty()) return paramTypes;

    for (auto& param : node->params) {
        paramTypes.push_back(getExpType(param.get()));
    }

    return paramTypes;
}

//实参
void SemanticAnalyzer::analyzeFuncRParams(FuncRParamsNode* node) {
    if (!node) return;
    // 检查函数实参的语义
    //c:未定义
    //cout<<"实参函数名:"<<node->name<<endl;
    if (symbolTable.Isundefined(node->name,funcLevel)) {
        reportError(node->linenum, "c");
    } else {
        //d:参数个数不匹配
        vector<string>param_types = getFuncRParamsTypes(node);
        auto entry = symbolTable.lookup(node->name);
        if(param_types.size() != entry->paramTypes.size()){
            cout<<"实参params is "<<param_types.size()<<endl;
            cout<<"形参params is "<<entry->paramTypes.size()<<endl;
            reportError(node->linenum,"d");
        } else {
            //e:参数类型不匹配
            if(param_types.size()!=0 && entry->paramTypes.size()!=0){
                for(int i=0;i<param_types.size();++i){
                    if(param_types[i] != entry->paramTypes[i] && param_types[i] != "Const"+entry->paramTypes[i]){
                        if(param_types[i] == "Void"){
                            cout<<"实参:"<<param_types[i]<<" 形参:"<<entry->paramTypes[i]<<endl;
                            reportError(node->linenum,"e");
                            break;
                        }
                        if(param_types[i].find("Array")!=string::npos&&entry->paramTypes[i].find("Array")==string::npos){
                            cout<<"实参:"<<param_types[i]<<" 形参:"<<entry->paramTypes[i]<<endl;
                            reportError(node->linenum,"e");
                            break;
                        }
                        else if(param_types[i].find("Array")==string::npos&&entry->paramTypes[i].find("Array")!=string::npos){
                            cout<<"实参:"<<param_types[i]<<" 形参:"<<entry->paramTypes[i]<<endl;
                            reportError(node->linenum,"e");
                            break;
                        }
                    }
                }
            }
        }
    }
    
    for(auto& paramsnode : node->params){
        traverseAST(paramsnode.get());
    }
    /*生成中间代码*/
    func_call(node->name);
}

void SemanticAnalyzer::analyzeMulExp(MulExpNode* node) {
    if (!node) return;
    for (int i = 0; i<node->operands.size();i++){
        traverseAST(node->operands[i].get());
        if (i > 0) {
            switch (node->operators[i - 1]) {
                case MULT:
                    codeOutput << "MULT" << endl;
                    break;
                case DIV:
                    codeOutput << "DIV" << endl;
                    break;
                case MOD:
                    codeOutput << "MOD" << endl;
                    break;
                default:
                    break;
            }
        }
    }
}

void SemanticAnalyzer::analyzeAddExp(AddExpNode* node) {
    if (!node) return;
    for (int i = 0; i<node->operands.size();i++){
        traverseAST(node->operands[i].get());
        if (i > 0) {
            switch (node->operators[i - 1]) {
                case PLUS:
                    codeOutput << "ADD" << endl;
                    break;
                case MINU:
                    codeOutput << "SUB" << endl;
                    break;
                default:
                    break;
            }
        }
    }
}

void SemanticAnalyzer::analyzeRelExp(RelExpNode* node) {
    if (!node) return;
    // 检查关系表达式的语义
    for (size_t i = 0; i < node->operands.size(); ++i) {
        traverseAST(node->operands[i].get());
        if (i > 0) {
            switch (node->operators[i - 1]) {
                case LSS:
                    codeOutput << "LT" << endl;
                    break;
                case GRE:
                    codeOutput << "GT" << endl;
                    break;
                case LEQ:
                    codeOutput << "LE" << endl;
                    break;
                case GEQ:
                    codeOutput << "GE" << endl;
                    break;
                default:
                    break;
            }
        }
    }
}

void SemanticAnalyzer::analyzeEqExp(EqExpNode* node) {
    if (!node) return;
    // 检查相等性表达式的语义
    for (size_t i = 0; i < node->operands.size(); ++i) {
        traverseAST(node->operands[i].get());
        if (i > 0) {
            switch (node->operators[i - 1]) {
                case EQL:
                    codeOutput << "EQ" << endl;
                    break;
                case NEQ:
                    codeOutput << "NE" << endl;
                    break;
                default:
                    break;
            }
        }
    }
}

void SemanticAnalyzer::analyzeLandExp(LandExpNode* node) {
    if (!node) return;
    // 检查逻辑与表达式的语义
    /*需要在这里处理短路求值问题*/
    int tmp = 0;
    if(node->operands.size()>=2)
        tmp = ++shortvalorder;
    for (size_t i = 0; i < node->operands.size(); ++i) {
        //printAST(node->operands[i].get());
        cout<<node->operands[i].get()->type<<endl;
        /*短路求值*/
        traverseAST(node->operands[i].get(), 0,[this](int value) {    
        });
        if (i > 0) {
            codeOutput << "AND" << endl;
        }
        if(node->operands.size()>=2){
            shortjumpiffalse_pcode(tmp);
        }
    }
    if(node->operands.size()>=2)
        shortlabel(tmp);
}

void SemanticAnalyzer::analyzeLorExp(LorExpNode* node) {
    if (!node) return;
    // 检查逻辑或表达式的语义
    int tmp = 0;
    if(node->operands.size()>=2)
        tmp = ++shortvalorder;
    for (size_t i = 0; i < node->operands.size(); ++i) {
        cout<<node->operands[i].get()->type<<endl;
        traverseAST(node->operands[i].get(), 0,[this](int value) {
            
        });
        if (i > 0) {
            codeOutput << "OR" << endl;
        }
        if(node->operands.size()>=2){
            shortjumpiftrue_pcode(tmp);
        }
    }
    if(node->operands.size()>=2)
        shortlabel(tmp);
}

void SemanticAnalyzer::analyzeNumber(NumberNode* node) {
    if (!node) return;
    // 检查数值的语义
    switch (node->type) {
        case NODE_NUMBER:
            codeOutput<<"PUSH"<<" "<<node->value<<endl;
            /*短路求值*/
            break;
        // 其他数值类型
    }
}

int getCharConstAscii(const string& charConst) {
    // 检查字符常量的长度是否为 1, 2, 3 或 4
    if (charConst.length() != 1 && charConst.length() != 2 && charConst.length() != 3 && charConst.length() != 4) {
        cerr << "Invalid CharConst: " << charConst << ". Expected format: 'c', '\\x', 'c', or '\\x'." << endl;
        return -1; // 返回 -1 表示错误
    }

    // 如果是长度为 1 的字符串（不带单引号）
    if (charConst.length() == 1) {
        char c = charConst[0];
        // 检查字符是否在 32-126 范围内
        if (c >= 32 && c <= 126 || c == 10) {
            return static_cast<int>(c); // 返回字符的 ASCII 码
        } else {
            cerr << "Invalid CharConst: " << charConst << ". Character must be in range 32-126." << endl;
            return -1;
        }
    }

    // 如果是长度为 2 的字符串（转义字符，不带单引号）
    if (charConst.length() == 2) {
        // 检查是否以反斜杠开头
        if (charConst[0] != '\\') {
            cerr << "Invalid CharConst: " << charConst << ". Escape sequence must start with '\\'." << endl;
            return -1;
        }

        // 转义字符映射表
        unordered_map<char, int> escapeMap = {
            {'a', 7},   // 响铃
            {'b', 8},   // 退格
            {'t', 9},   // 制表符
            {'n', 10},  // 换行
            {'v', 11},  // 垂直制表符
            {'f', 12},  // 换页
            {'r', 13},  // 回车
            {'\\', 92}, // 反斜杠
            {'\'', 39}, // 单引号
            {'\"', 34}, // 双引号
            {'0', 0}    // 空字符
        };

        char escapeChar = charConst[1];
        if (escapeMap.find(escapeChar) != escapeMap.end()) {
            return escapeMap[escapeChar]; // 返回转义字符的 ASCII 码
        } else {
            cerr << "Invalid escape sequence: " << charConst << ". Unknown escape character: " << escapeChar << endl;
            return -1;
        }
    }

    // 如果是普通字符（长度为 3）
    if (charConst.length() == 3) {
        // 检查字符常量是否以单引号开头和结尾
        if (charConst[0] != '\'' || charConst[2] != '\'') {
            cerr << "Invalid CharConst: " << charConst << ". Must start and end with single quotes." << endl;
            return -1;
        }

        char c = charConst[1];
        // 检查字符是否在 32-126 范围内
        if (c >= 32 && c <= 126) {
            return static_cast<int>(c); // 返回字符的 ASCII 码
        } else {
            cerr << "Invalid CharConst: " << charConst << ". Character must be in range 32-126." << endl;
            return -1;
        }
    }

    // 如果是转义字符（长度为 4）
    if (charConst.length() == 4) {
        // 检查字符常量是否以单引号开头和结尾
        if (charConst[0] != '\'' || charConst[3] != '\'') {
            cerr << "Invalid CharConst: " << charConst << ". Must start and end with single quotes." << endl;
            return -1;
        }

        // 检查是否以反斜杠开头
        if (charConst[1] != '\\') {
            cerr << "Invalid CharConst: " << charConst << ". Escape sequence must start with '\\'." << endl;
            return -1;
        }

        // 转义字符映射表
        unordered_map<char, int> escapeMap = {
            {'a', 7},   // 响铃
            {'b', 8},   // 退格
            {'t', 9},   // 制表符
            {'n', 10},  // 换行
            {'v', 11},  // 垂直制表符
            {'f', 12},  // 换页
            {'r', 13},  // 回车
            {'\\', 92}, // 反斜杠
            {'\'', 39}, // 单引号
            {'\"', 34}, // 双引号
            {'0', 0}    // 空字符
        };

        char escapeChar = charConst[2];
        if (escapeMap.find(escapeChar) != escapeMap.end()) {
            return escapeMap[escapeChar]; // 返回转义字符的 ASCII 码
        } else {
            cerr << "Invalid escape sequence: " << charConst << ". Unknown escape character: " << escapeChar << endl;
            return -1;
        }
    }

    return -1; // 默认返回错误
}


void SemanticAnalyzer::analyzeCharacter(CharacterNode* node) {
    if (!node) return;
    // 检查字符的语义
    switch (node->type) {
        case NODE_CHARACTER:
            codeOutput<<"PUSH"<<" "<<getCharConstAscii(node->value)<<endl;
            break;
        // 其他字符类型
    }
}

void SemanticAnalyzer::analyzeConstExp(ConstExpNode* node) {
    if (!node) return;
    // 检查常量表达式的语义
    // 语法分析的时候就转add了
}

void SemanticAnalyzer::analyzeFor(ForNode* node) {
    if (!node) return;
    // 检查for语句的语义
    labelfor_bk_ctn ++;
    int tmp = labelfor_bk_ctn;
    break_continu = tmp;
    //for{}里的作用域
    if (node->init){
        analyzeSmallfor(static_cast<SmallforstmtNode*>(node->init.get()));
    } 
    label("FOR_START",tmp);
    if (node->forcond) {
        analyzeLorExp(static_cast<LorExpNode*>(node->forcond.get()));
        jumpiffalse_pcode("FOR_END",tmp);
    }
        
    traverseAST(static_cast<StmtNode*>(node->body.get()),tmp);
    //cout<<"tmp is "<<tmp<<endl;
    label("CONTINUE",tmp);

    if (node->step) {
        analyzeSmallfor(static_cast<SmallforstmtNode*>(node->step.get()));
    }
    jump_pcode("FOR_START",tmp);
    label("BREAK",tmp);
    label("FOR_END",tmp);
    break_continu --;
}

void SemanticAnalyzer::analyzeSmallfor(SmallforstmtNode* node) {
    if (!node) return;
    // 检查for语句的语义
    
    if (node->exp){
        auto lvalnode = static_cast<LValNode*>(node->lval.get());
        auto lvalentry = symbolTable.lookup(lvalnode->name);
        if(lvalentry){
            if(lvalentry->type.find("Const")!=string::npos){
                reportError(lvalnode->linenum,"h");
            }
        }
        analyzeAddExp(static_cast<AddExpNode*>(node->exp.get()));
    }
    //先右后左
    if (node->lval)
        analyzeLVal(static_cast<LValNode*>(node->lval.get()),true);
        
}

void SemanticAnalyzer::analyzeReturnStmt(ReturnStmtNode* node) {
    if (!node) return;
    // 检查 return 语句的语义
    //cout << "Analyzing ReturnStmtNode" << endl;
    // 你可以在这里添加更多的语义检查逻辑
    if (node->exp) {
        traverseAST(node->exp.get());
    }
    if(return_pop_varsparams.size()){
        for(auto it: return_pop_varsparams){
            pop_var(it);
        }
    }
    if(return_pop_varsin.size()){
        for(auto it: return_pop_varsin){
            pop_var(it);
        }
    }
    if(node->exp) {
        return_pcode();
    }else{
        returnnull_pcode();
    }

}

void SemanticAnalyzer::analyzeBreakStmt(BreakStmtNode* node,int tmplabel) {
    if (!node) return;
    // 检查 break 语句的语义
    //cout << "Analyzing BreakStmtNode" << endl;
    // 你可以在这里添加更多的语义检查逻辑
    break_pcode(break_continu);
}

void SemanticAnalyzer::analyzeContinueStmt(ContinueStmtNode* node,int tmplabel) {
    if (!node) return;
    // 检查 continue 语句的语义
    //cout << "Analyzing ContinueStmtNode" << endl;
    // 你可以在这里添加更多的语义检查逻辑
    continue_pcode(break_continu);
}
//错误l
bool checkPrintfFormatErr(PrintfStmtNode* printfStmtNode) {
    if (!printfStmtNode) return false;
    string format = printfStmtNode->format;
    vector<unique_ptr<ASTNode>>& args = printfStmtNode->args;
    int formatCount = 0;
    size_t pos = 0;
    // 查找 %d
    while ((pos = format.find("%d", pos)) != string::npos) {
        formatCount++;
        pos += 2; // 跳过 "%d"
    }
    // 查找 %c
    pos = 0;
    while ((pos = format.find("%c", pos)) != string::npos) {
        formatCount++;
        pos += 2; // 跳过 "%c"
    }
    if (formatCount != args.size()) {
        cout << "formatcount: " << formatCount << " args num: " << args.size() << endl;
        cout << printfStmtNode->format << endl;
        return true;
    }
    return false;
}
void SemanticAnalyzer::analyzePrintfStmt(PrintfStmtNode* node) {
    if (!node) return;
    for (auto& arg : node->args) {
        traverseAST(arg.get());
    }
    if(checkPrintfFormatErr(node)){
        reportError(node->printlinenum,"l");
    }
    printf_pcode(node->format);
}

void SemanticAnalyzer::analyzeEmptyStmt(EmptyStmtNode* node) {
    if (!node) return;
    // 检查空语句的语义
    //cout << "Analyzing EmptyStmtNode" << endl;
    // 你可以在这里添加更多的语义检查逻辑
}

void SemanticAnalyzer::analyzeAssignStmt(AssignStmtNode* node) {
    if (!node) return;
    // 检查赋值语句的语义
    //cout << "Analyzing AssignStmtNode" << endl;
    // 你可以在这里添加更多的语义检查逻辑
    // 检查左值
    auto lvalnode = static_cast<LValNode*>(node->lval.get());
    // 检查右值
    if (node->exp) {
        traverseAST(static_cast<ExpNode*>(node->exp.get()));
    } else if (node->getint){
        codeOutput<<"GETINT"<<endl;
    } else if (node->getchar){
        codeOutput<<"GETCHAR"<<endl;
    }
    //检查左值
    if (node->lval) {
        analyzeLVal(lvalnode,true);
    }
    auto entry = symbolTable.lookup(lvalnode->name);
    if (entry) {
        if (entry->type.find("Const")!= string::npos) {
            reportError(lvalnode->linenum,"h");
        }
    }

    // 检查左值和右值的类型是否匹配
    
}

void SemanticAnalyzer::analyzeExpStmt(ExpStmtNode* node) {
    if (!node) return;
    // 检查表达式语句的语义
    //cout << "Analyzing ExpStmtNode" << endl;
    // 你可以在这里添加更多的语义检查逻辑
    /*
    if(node->unable){
        printAST(node);
    }
    */
    if (node->exp){ // && (!node->unable)) {
        traverseAST(node->exp.get());
    }
}

void SemanticAnalyzer::analyzeIfStmt(IfStmtNode* node) {
    if (!node) return;
    // 检查 if 语句的语义
    //cout << "Analyzing IfStmtNode" << endl;
    // 检查条件表达式
    if_order++;
    int cur_if = if_order;
    analyzeLorExp(static_cast<LorExpNode*>(node->ifcond.get()));
    // 检查 else 分支
    if (node->elseStmt) {
        jumpiffalse_pcode("ELSE",cur_if);
        // 检查 then 分支
        traverseAST(node->thenStmt.get());//stmt
        jump_pcode("END_IF",cur_if);
        label("ELSE",cur_if);
        traverseAST(node->elseStmt.get());//stmt
    } else {
        jumpiffalse_pcode("END_IF",cur_if);
        traverseAST(node->thenStmt.get());//stmt
    }
    label("END_IF",cur_if);
}

void SemanticAnalyzer::reportError(int linenum, const string& errorCode) {
    // 报告错误
    errorOutput <<linenum <<" " << errorCode << endl;
}