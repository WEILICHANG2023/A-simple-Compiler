#include "parser.h"
#include "lexer.h"
#include "ast.h"
#include <string>

Parser::Parser(const vector<Token>& tokens, const string& parserOutputFile, const string& errorOutputFile)
    : tokens(tokens), parserOutputFile(parserOutputFile), errorOutputFile(errorOutputFile) {}

unique_ptr<ASTNode> Parser::parse() {
    openFiles();
    auto ast = compUnit();
    closeFiles();
    //printAST(ast.get(),0);
    return ast;
}

void Parser::openFiles() {
    parserOutput.open(parserOutputFile);
    errorOutput.open(errorOutputFile);
}

void Parser::closeFiles() {
    parserOutput.close();
    errorOutput.close();
}

void Parser::processToken(TokenType type, const string& value) {
    parserOutput << tokenTypeMap[type] << " " << value << endl;
}

void Parser::processError(int lineNumber, const string& error) {
    errorOutput << lineNumber << " " << error << endl;
}

unique_ptr<ASTNode> Parser::compUnit() {
    auto compUnitNode = make_unique<CompUnitNode>();
    while (currentIndex < tokens.size()) {
        if (currentToken().type == CONSTTK) {
            compUnitNode->decls.push_back(decl());
        } else if (currentToken().type == INTTK || currentToken().type == CHARTK) {
            if (currentToken().type == INTTK && lookAhead(1).type == MAINTK) {
                compUnitNode->mainFuncDef = mainFuncDef();
                break;
            } else {
                if (lookAhead(1).type == IDENFR && lookAhead(2).type == LPARENT) {
                    string functype;
                    if(currentToken().type == INTTK)
                        functype = "IntFunc";
                    else if(currentToken().type == CHARTK)
                        functype = "CharFunc";
                    compUnitNode->funcDefs.push_back(funcDef(functype,currentToken().lineNumber));
                } else {
                    compUnitNode->decls.push_back(decl());
                }
            }
        } else if (currentToken().type == VOIDTK) {
            compUnitNode->funcDefs.push_back(funcDef("VoidFunc",currentToken().lineNumber));
        } else {
            cout<<currentToken().value<<endl;
            processError(currentToken().lineNumber, "Unexpected token in CompUnit");
            break;
        }
    }
    return compUnitNode;
}

unique_ptr<ASTNode> Parser::decl() {
    if (currentToken().type == CONSTTK) {
        return constDecl();
    } else if (currentToken().type == INTTK || currentToken().type == CHARTK) {
        return varDecl();
    }
    return nullptr;
}

unique_ptr<ASTNode> Parser::constDecl() {
    string consttype;
    match(CONSTTK);
    if(currentToken().type == INTTK)
        consttype = "ConstInt";
    else if(currentToken().type == CHARTK)
        consttype = "ConstChar";
    match(currentToken().type); // INTTK or CHARTK
    auto constDeclNode = make_unique<ConstDeclNode>();
    if (currentToken().type == IDENFR) {
        constDeclNode->constDefs.push_back(constDef(consttype,currentToken().lineNumber));
    }
    while (currentToken().type == COMMA) {
        match(COMMA);
        constDeclNode->constDefs.push_back(constDef(consttype,currentToken().lineNumber));
    }
    match(SEMICN);
    return constDeclNode;
}

unique_ptr<ASTNode> Parser::constDef(string type,int line) {
    auto constDefNode = make_unique<ConstDefNode>();
    constDefNode->name = currentToken().value;
    constDefNode->constdeftype = type;
    constDefNode->linenum = line;
    match(IDENFR);
    if (currentToken().type == LBRACK) {
        constDefNode->constdeftype += "Array";
        match(LBRACK);
        constDefNode->arraysize = constExp();
        match(RBRACK);
    }
    match(ASSIGN);
    constInitVal(std::move(constDefNode));
    return constDefNode;
}

unique_ptr<ASTNode> Parser::constInitVal(unique_ptr<ConstDefNode> &&constDefNode) {
    if (currentToken().type == STRCON) {
        strcon(std::move(constDefNode));
    } else if (currentToken().type == LBRACE) {
        match(LBRACE);
        if (currentToken().type != RBRACE) {
            constDefNode->initVals.push_back(constExp()); // 递归调用以处理嵌套的数组初始化
            while (currentToken().type == COMMA) {
                match(COMMA);
                constDefNode->initVals.push_back(constExp()); // 递归调用以处理嵌套的数组初始化
            }
        }
        match(RBRACE);
    } else {
        constDefNode->initVals.push_back(constExp());
    }
    return nullptr;
}

unique_ptr<ASTNode> Parser::varDecl() {
    string vartype;
    if(currentToken().type == INTTK)
        vartype = "Int";
    else if(currentToken().type == CHARTK)
        vartype = "Char";
    match(currentToken().type); // INTTK or CHARTK
    auto varDeclNode = make_unique<VarDeclNode>();
    varDeclNode->varDefs.push_back(varDef(vartype,currentToken().lineNumber));
    while (currentToken().type == COMMA) {
        match(COMMA);
        varDeclNode->varDefs.push_back(varDef(vartype,currentToken().lineNumber));
    }
    match(SEMICN);
    return varDeclNode;
}

unique_ptr<ASTNode> Parser::varDef(string type,int line) {
    auto varDefNode = make_unique<VarDefNode>();
    varDefNode->name = currentToken().value;
    varDefNode->vardeftype = type;
    varDefNode->linenum = line;
    match(IDENFR);
    if (currentToken().type == LBRACK) {
        varDefNode->vardeftype += "Array";
        match(LBRACK);
        varDefNode->arraysize = constExp();
        match(RBRACK);
    }
    if (currentToken().type == ASSIGN) {
        match(ASSIGN);
        initVal(std::move(varDefNode));
    }
    return varDefNode;
}

unique_ptr<ASTNode> Parser::initVal(unique_ptr<VarDefNode> &&varDefNode) {
    if (currentToken().type == STRCON) {
        strcon(std::move(varDefNode));
    } else if (currentToken().type == LBRACE) {
        match(LBRACE);
        if (currentToken().type != RBRACE) {
            varDefNode->initVals.push_back(exp());
            while (currentToken().type == COMMA) {
                match(COMMA);
                varDefNode->initVals.push_back(exp());
            }
        }
        match(RBRACE);
    } else {
        //cout<<"走的exp？"<<endl;
        varDefNode->initVals.push_back(exp());
    }
    return nullptr;
}

unique_ptr<ASTNode> Parser::funcDef(string type,int line) {
    match(currentToken().type); // VOIDTK, INTTK, or CHARTK
    auto funcDefNode = make_unique<FuncDefNode>();
    funcDefNode->name = currentToken().value;
    funcDefNode->funcdeftype = type;
    funcDefNode->linenum = line;
    match(IDENFR);
    match(LPARENT);
    if (currentToken().type == INTTK || currentToken().type == CHARTK) {
        funcDefNode->params = funcFParams();
    }
    match(RPARENT);
    funcDefNode->block = block(false);
    return funcDefNode;
}

unique_ptr<ASTNode> Parser::mainFuncDef() {
    match(INTTK);
    match(MAINTK);
    match(LPARENT);
    match(RPARENT);
    auto mainFuncDefNode = make_unique<MainFuncDefNode>();
    mainFuncDefNode->block = block(false);
    return mainFuncDefNode;
}

unique_ptr<ASTNode> Parser::funcFParams() {
    auto funcFParamsNode = make_unique<FuncFParamsNode>();
    funcFParamsNode->params.push_back(funcFParam());
    while (currentToken().type == COMMA) {
        match(COMMA);
        funcFParamsNode->params.push_back(funcFParam());
    }
    return funcFParamsNode;
}

unique_ptr<ASTNode> Parser::funcFParam() {
    auto funcFParamNode = make_unique<FuncFParamNode>();
    funcFParamNode->type = currentToken().type; // INTTK or CHARTK
    if(currentToken().type == INTTK) {
        funcFParamNode->realtype = "Int";
    } else if(currentToken().type == CHARTK) {
        funcFParamNode->realtype = "Char";
    }
    match(currentToken().type);
    funcFParamNode->name = currentToken().value;
    funcFParamNode->linenum = currentToken().lineNumber;
    match(IDENFR);
    if (currentToken().type == LBRACK) {
        match(LBRACK);
        match(RBRACK);
        funcFParamNode->isArray = true;
        funcFParamNode->realtype += "Array";
    } else {
        funcFParamNode->isArray = false;
    }
    return funcFParamNode;
}

unique_ptr<ASTNode> Parser::block(bool blockisfor) {
    match(LBRACE);
    auto blockNode = make_unique<BlockNode>();
    while (currentToken().type != RBRACE) {
        if (currentToken().type == CONSTTK || currentToken().type == INTTK || currentToken().type == CHARTK) {
            blockNode->stmts.push_back(decl());
        } else {
            blockNode->stmts.push_back(stmt(false));
        }
    }
    blockNode->end_linenum = currentToken().lineNumber;
    blockNode->isfor = blockisfor;
    match(RBRACE);
    return blockNode;
}

unique_ptr<ASTNode> Parser::stmt(bool blockisfor) {
    if (currentToken().type == LBRACE) {
        return block(blockisfor);
    } else if (currentToken().type == IFTK) {
        match(IFTK);
        auto ifStmtNode = make_unique<IfStmtNode>();
        match(LPARENT);
        unique_ptr<ASTNode> realcond = cond();
        match(RPARENT);
        auto thenStmt = stmt(false);
        unique_ptr<ASTNode> elseStmt = nullptr;
        if (currentToken().type == ELSETK) {
            match(ELSETK);
            elseStmt = stmt(false);
        }
        ifStmtNode->ifcond = move(realcond);
        ifStmtNode->thenStmt = move(thenStmt);
        ifStmtNode->elseStmt = move(elseStmt);
        return ifStmtNode;
    } else if (currentToken().type == FORTK) {
        match(FORTK);
        match(LPARENT);
        unique_ptr<ASTNode> init = nullptr;
        if (currentToken().type != SEMICN) {
            init = forStmt();
        }
        match(SEMICN);
        unique_ptr<ASTNode> realcond = nullptr;
        if (currentToken().type != SEMICN) {
            realcond = cond();
        }
        match(SEMICN);
        unique_ptr<ASTNode> step = nullptr;
        if (currentToken().type != RPARENT) {
            step = forStmt();
        }
        match(RPARENT);
        auto body = stmt(true);
        auto forNode = make_unique<ForNode>();
        forNode->init = move(init);
        forNode->forcond = move(realcond);
        forNode->step = move(step);
        forNode->body = move(body);
        return forNode;
    } else if (currentToken().type == BREAKTK) {
        int linenum = currentToken().lineNumber;
        match(BREAKTK);
        match(SEMICN);
        return make_unique<BreakStmtNode>(linenum);
    } else if (currentToken().type == CONTINUETK) {
        int linenum = currentToken().lineNumber;
        match(CONTINUETK);
        match(SEMICN);
        return make_unique<ContinueStmtNode>(linenum);
    } else if (currentToken().type == RETURNTK) {
        auto returnStmtNode = make_unique<ReturnStmtNode>();
        returnStmtNode->linenum = currentToken().lineNumber;
        match(RETURNTK);
        if (currentToken().type != SEMICN) {
            returnStmtNode->exp = exp();
        }
        match(SEMICN);
        return returnStmtNode;
    } else if (currentToken().type == PRINTFTK) {
        match(PRINTFTK);
        match(LPARENT);
        auto printfStmtNode = make_unique<PrintfStmtNode>();
        printfStmtNode->format = currentToken().value;
        printfStmtNode->printlinenum = currentToken().lineNumber;
        match(STRCON);
        while (currentToken().type == COMMA) {
            match(COMMA);
            printfStmtNode->args.push_back(exp());
        }
        match(RPARENT);
        match(SEMICN);
        return printfStmtNode;
    } else if (currentToken().type == SEMICN) {
        match(SEMICN);
        return make_unique<EmptyStmtNode>();
    } else if (currentToken().type == IDENFR) {
        if (lookAhead(1).type == LBRACK || lookAhead(1).type == ASSIGN) {
            auto lValNode = lVal();
            match(ASSIGN);
            if (currentToken().type == GETINTTK) {
                match(GETINTTK);
                match(LPARENT);
                match(RPARENT);
                match(SEMICN);
                auto assignStmtNode = make_unique<AssignStmtNode>();
                assignStmtNode->lval = move(lValNode);
                assignStmtNode->getint = true;
                assignStmtNode->getchar = false;
                return assignStmtNode;
            } else if (currentToken().type == GETCHARTK) {
                match(GETCHARTK);
                match(LPARENT);
                match(RPARENT);
                match(SEMICN);
                auto assignStmtNode = make_unique<AssignStmtNode>();
                assignStmtNode->lval = move(lValNode);
                assignStmtNode->getchar = true;
                assignStmtNode->getint = false;
                return assignStmtNode;
            } else {
                auto expNode = exp();
                match(SEMICN);
                auto assignStmtNode = make_unique<AssignStmtNode>();
                assignStmtNode->lval = move(lValNode);
                assignStmtNode->exp = move(expNode);
                return assignStmtNode;
            }
        } else {
            /*exp在pcode中不算数*/
            auto expNode = exp();
            match(SEMICN);
            auto expStmtNode = make_unique<ExpStmtNode>();
            expStmtNode->exp = move(expNode);    
            return expStmtNode;//exp
        }
    } else if (currentToken().type == INTCON || currentToken().type == CHRCON || currentToken().type == STRCON || currentToken().type == LPARENT || currentToken().type == PLUS || currentToken().type == MINU || currentToken().type == NOT) {
        /*exp在pcode中不算数*/
        auto expNode = exp();
        match(SEMICN);
        auto expStmtNode = make_unique<ExpStmtNode>();
        return expStmtNode;
    } else {
        processError(currentToken().lineNumber, "Unexpected token in Stmt");
        return nullptr;
    }
}

unique_ptr<ASTNode> Parser::exp() {
    return addExp();
}

unique_ptr<ASTNode> Parser::cond() {
    return loExp();
}

unique_ptr<ASTNode> Parser::lVal() {
    auto lValNode = make_unique<LValNode>();
    lValNode->name = currentToken().value;
    lValNode->linenum = currentToken().lineNumber;
    match(IDENFR);
    if (currentToken().type == LBRACK) {
        lValNode->maybeisarray = false;
        match(LBRACK);
        lValNode->indice = exp();
        match(RBRACK);
    }
    return lValNode;
}

unique_ptr<ASTNode> Parser::primaryExp() {
    if (currentToken().type == LPARENT) {
        match(LPARENT);
        auto expNode = exp();
        match(RPARENT);
        return expNode;
    } else if (currentToken().type == IDENFR) {
        return lVal();
    } else if (currentToken().type == INTCON) {
        return number();
    } else if (currentToken().type == CHRCON) {
        return character();
    } else {
        cout<<"UNEXPECTED "<<currentToken().type<<" "<<currentToken().value<<endl;
        processError(currentToken().lineNumber, "Unexpected token in PrimaryExp");
        return nullptr;
    }
}

unique_ptr<ASTNode> Parser::unaryExp() {
    if (currentToken().type == PLUS || currentToken().type == MINU || currentToken().type == NOT) {
        auto unaryExpnode = make_unique<UnaryExpNode>();
        unaryExpnode->unaryop = currentToken().type;
        match(currentToken().type);
        unaryExpnode->unaryexp = unaryExp();
        return unaryExpnode;
    } else if (currentToken().type == IDENFR && lookAhead(1).type == LPARENT) {
        auto funcRParamsNode = make_unique<FuncRParamsNode>();
        funcRParamsNode->linenum = currentToken().lineNumber;   //c和d和e：实参
        funcRParamsNode->name = currentToken().value;
        match(IDENFR);
        match(LPARENT);
        if (currentToken().type==IDENFR||currentToken().type==INTCON||currentToken().type==CHRCON||currentToken().type==STRCON||currentToken().type==LPARENT||currentToken().type==PLUS||currentToken().type==MINU||currentToken().type==NOT) {
            funcRParamsNode->params.push_back(exp());
            while (currentToken().type == COMMA) {
                match(COMMA);
                funcRParamsNode->params.push_back(exp());
            }
        }
        match(RPARENT);
        return funcRParamsNode;
    } else {
        return primaryExp();
    }
}

unique_ptr<ASTNode> Parser::funcRParams() { //一直没用上
    auto funcRParamsNode = make_unique<FuncRParamsNode>();
    funcRParamsNode->params.push_back(exp());
    while (currentToken().type == COMMA) {
        match(COMMA);
        funcRParamsNode->params.push_back(exp());
    }
    return funcRParamsNode;
}

unique_ptr<ASTNode> Parser::mulExp() {
    auto mulExpNode = make_unique<MulExpNode>();
    mulExpNode->operands.push_back(unaryExp());
    while (currentToken().type == MULT || currentToken().type == DIV || currentToken().type == MOD) {
        mulExpNode->operators.push_back(currentToken().type);
        match(currentToken().type);
        mulExpNode->operands.push_back(unaryExp());
    }
    return mulExpNode;
}

unique_ptr<ASTNode> Parser::addExp() {
    auto addExpNode = make_unique<AddExpNode>();
    addExpNode->operands.push_back(mulExp());
    while (currentToken().type == PLUS || currentToken().type == MINU) {
        addExpNode->operators.push_back(currentToken().type);
        match(currentToken().type);
        addExpNode->operands.push_back(mulExp());
    }
    return addExpNode;
}

unique_ptr<ASTNode> Parser::reExp() {
    auto relExpNode = make_unique<RelExpNode>();
    relExpNode->operands.push_back(addExp());
    while (currentToken().type == LSS || currentToken().type == GRE || currentToken().type == LEQ || currentToken().type == GEQ) {
        relExpNode->operators.push_back(currentToken().type);
        match(currentToken().type);
        relExpNode->operands.push_back(addExp());
    }
    return relExpNode;
}

unique_ptr<ASTNode> Parser::eqExp() {
    auto eqExpNode = make_unique<EqExpNode>();
    eqExpNode->operands.push_back(reExp());
    while (currentToken().type == EQL || currentToken().type == NEQ) {
        eqExpNode->operators.push_back(currentToken().type);
        match(currentToken().type);
        eqExpNode->operands.push_back(reExp());
    }
    return eqExpNode;
}

unique_ptr<ASTNode> Parser::laExp() {
    auto landExpNode = make_unique<LandExpNode>();
    landExpNode->operands.push_back(eqExp());
    while (currentToken().type == AND) {
        landExpNode->operators.push_back(currentToken().type);
        match(currentToken().type);
        landExpNode->operands.push_back(eqExp());
    }
    return landExpNode;
}

unique_ptr<ASTNode> Parser::loExp() {
    auto lorExpNode = make_unique<LorExpNode>();
    lorExpNode->operands.push_back(laExp());
    while (currentToken().type == OR) {
        lorExpNode->operators.push_back(currentToken().type);
        match(currentToken().type);
        lorExpNode->operands.push_back(laExp());
    }
    return lorExpNode;
}

unique_ptr<ASTNode> Parser::number() {
    auto numberNode = make_unique<NumberNode>();
    numberNode->value = stoi(currentToken().value);
    match(INTCON);
    return numberNode;
}

unique_ptr<ASTNode> Parser::character() {
    auto characterNode = make_unique<CharacterNode>();
    characterNode->value = currentToken().value;
    match(CHRCON);
    return characterNode;
}
/*
unique_ptr<ASTNode> Parser::str2char(char ch) {
    auto characterNode = make_unique<CharacterNode>();
    characterNode->value = ch;
    return characterNode;
}

unique_ptr<ASTNode> Parser::strcon(unique_ptr<ConstDefNode> && constDefNode) {
    for(auto thischar: currentToken().value){
        if(thischar != '"'){
            constDefNode->initVals.push_back(str2char(thischar));
        }
    }
    match(STRCON);
    return nullptr;
}
*/
unique_ptr<ASTNode> Parser::str2char(char ch) {
    auto characterNode = make_unique<CharacterNode>();
    characterNode->value = ch; // 直接存储字符
    return characterNode;
}

// 修改后的 strcon 函数
unique_ptr<ASTNode> Parser::strcon(unique_ptr<ConstDefNode> &&constDefNode) {
    std::string str = currentToken().value.substr(1, currentToken().value.length() - 2); // 去掉首尾的双引号

    for (size_t i = 0; i < str.length(); ++i) {
        if (str[i] == '\\' && i + 1 < str.length() && str[i + 1] == 'n') {
            // 处理转义字符 '\n'
            constDefNode->initVals.push_back(str2char('\n'));
            ++i; // 跳过 'n'
        } else if (str[i] == '\\' && (i + 1 >= str.length() || str[i + 1] != 'n')) {
            // 非法转义字符
            cerr << "Error: Invalid escape sequence in string constant: " << str << endl;
            return nullptr;
        } else {
            // 处理普通字符
            constDefNode->initVals.push_back(str2char(str[i]));
        }
    }

    match(STRCON); // 匹配字符串常量
    return nullptr;
}
unique_ptr<ASTNode> Parser::strcon(unique_ptr<VarDefNode> && varDefNode) {
    for(auto thischar: currentToken().value){
        if(thischar != '"'){
            varDefNode->initVals.push_back(str2char(thischar));
        }
    }
    match(STRCON);
    return nullptr;
}

unique_ptr<ASTNode> Parser::constExp() {
    return addExp();
}

unique_ptr<ASTNode> Parser::forStmt() {//小for
    auto smallforStmtNode = make_unique<SmallforstmtNode>();
    auto lValNode = lVal();
    match(ASSIGN);
    auto expNode = exp();
    smallforStmtNode->lval = move(lValNode);
    smallforStmtNode->exp = move(expNode);
    return smallforStmtNode;
}

Token Parser::currentToken() const {
    if (currentIndex < tokens.size()) {
        return tokens[currentIndex];
    }
    return {UNKNOWN, "", -1};
}

Token Parser::lookAhead(int offset) const {
    int index = currentIndex + offset;
    if (index < tokens.size()) {
        return tokens[index];
    }
    return {UNKNOWN, "", -1};
}

void Parser::match(TokenType expectedType) {
    if (currentToken().type == expectedType) {
        processToken(currentToken().type, currentToken().value);
        currentIndex++;
    } else if (expectedType == RPARENT) {
        processError(tokens[currentIndex - 1].lineNumber, "j");
    } else if (expectedType == SEMICN) {
        processError(tokens[currentIndex - 1].lineNumber, "i");
    } else if (expectedType == RBRACK) {
        processError(tokens[currentIndex - 1].lineNumber, "k");
    } else {
        processError(tokens[currentIndex - 1].lineNumber, "Expected " + tokenTypeMap[expectedType] + ", found " + tokenTypeMap[currentToken().type]);
    }
}

void printAST(ASTNode* node, int indent) {
    if (!node) return;

    switch (node->type) {
        case NODE_COMPUNIT: {
            auto compUnitNode = static_cast<CompUnitNode*>(node);
            cout << string(indent, ' ') << "CompUnitNode" << endl;
            for (auto& decl : compUnitNode->decls) {
                printAST(decl.get(), indent + 2);
            }
            for (auto& funcDef : compUnitNode->funcDefs) {
                printAST(funcDef.get(), indent + 2);
            }
            printAST(compUnitNode->mainFuncDef.get(), indent + 2);
            break;
        }
        case NODE_DECL: {
            auto declNode = static_cast<DeclNode*>(node);
            cout << string(indent, ' ') << "DeclNode" << endl;
            break;
        }
        case NODE_CONSTDECL: {
            auto constDeclNode = static_cast<ConstDeclNode*>(node);
            cout << string(indent, ' ') << "ConstDeclNode" << endl;
            for (auto& constDef : constDeclNode->constDefs) {
                printAST(constDef.get(), indent + 2);
            }
            break;
        }
        case NODE_CONSTDEF: {
            auto constDefNode = static_cast<ConstDefNode*>(node);
            cout << string(indent, ' ') << "ConstDefNode: " << constDefNode->name << endl;
            if(constDefNode->arraysize)
                printAST(constDefNode->arraysize.get(), indent + 2);
            for (auto& initval : constDefNode->initVals) {
                printAST(initval.get(), indent + 2);
            }
            break;
        }
        case NODE_VARDECL: {
            auto varDeclNode = static_cast<VarDeclNode*>(node);
            cout << string(indent, ' ') << "VarDeclNode" << endl;
            for (auto& varDef : varDeclNode->varDefs) {
                printAST(varDef.get(), indent + 2);
            }
            break;
        }
        case NODE_VARDEF: {
            auto varDefNode = static_cast<VarDefNode*>(node);
            cout << string(indent, ' ') << "VarDefNode: " << varDefNode->name << endl;
            if(varDefNode->arraysize)
                printAST(varDefNode->arraysize.get(), indent + 2);
            for (auto& initval : varDefNode->initVals) {
                printAST(initval.get(), indent + 2);
            }
            break;
        }
        case NODE_FUNCDEF: {
            auto funcDefNode = static_cast<FuncDefNode*>(node);
            cout << string(indent, ' ') << "FuncDefNode: " << funcDefNode->name << endl;
            if (funcDefNode->params) {
                printAST(funcDefNode->params.get(), indent + 2);
            }
            printAST(funcDefNode->block.get(), indent + 2);
            break;
        }
        case NODE_FUNCFPARAM: {
            auto funcFParamNode = static_cast<FuncFParamNode*>(node);
            cout << string(indent, ' ') << "FuncFParamNode: " << tokenTypeMap[funcFParamNode->type] << " " << funcFParamNode->name;
            if (funcFParamNode->isArray) {
                cout << "[]";
            }
            cout << endl;
            break;
        }
        case NODE_FUNCFPARAMS: {
            auto funcFParamsNode = static_cast<FuncFParamsNode*>(node);
            cout << string(indent, ' ') << "FuncFParamsNode" << endl;
            for (auto& param : funcFParamsNode->params) {
                printAST(param.get(), indent + 2);
            }       
            break;
        }
        case NODE_MAINFUNCDEF: {
            auto mainFuncDefNode = static_cast<MainFuncDefNode*>(node);
            cout << string(indent, ' ') << "MainFuncDefNode" << endl;
            printAST(mainFuncDefNode->block.get(), indent + 2);
            break;
        }
        case NODE_BLOCK: {
            auto blockNode = static_cast<BlockNode*>(node);
            cout << string(indent, ' ') << "BlockNode" << endl;
            for (auto& stmt : blockNode->stmts) {
                printAST(stmt.get(), indent + 2);
            }
            break;
        }
        case NODE_STMT: {
            auto stmtNode = static_cast<StmtNode*>(node);
            cout << string(indent, ' ') << "StmtNode" << endl;
            break;
        }
        case NODE_EXP: {
            auto expNode = static_cast<ExpNode*>(node);
            cout << string(indent, ' ') << "ExpNode" << endl;
            break;
        }
        case NODE_CONDEXP: {
            auto condExpNode = static_cast<CondExpNode*>(node);
            cout << string(indent, ' ') << "CondExpNode" << endl;
            break;
        }
        case NODE_LVAL: {
            auto lValNode = static_cast<LValNode*>(node);
            cout << string(indent, ' ') << "LValNode: " << lValNode->name << endl;
            if(lValNode->indice){
                printAST(lValNode->indice.get(), indent + 2 );
            }
            break;
        }
        case NODE_PRIMARYEXP: {
            auto primaryExpNode = static_cast<PrimaryExpNode*>(node);
            cout << string(indent, ' ') << "PrimaryExpNode" << endl;
            break;
        }
        case NODE_UNARYEXP: {
            auto unaryExpNode = static_cast<UnaryExpNode*>(node);
            cout << string(indent, ' ') << "UnaryExpNode" << endl;
            break;
        }
        case NODE_FUNCRPARAMS: {
            auto funcRParamsNode = static_cast<FuncRParamsNode*>(node);
            cout << string(indent, ' ') << "FuncRParamsNode" << endl;
            for (auto& param : funcRParamsNode->params) {
                printAST(param.get(), indent + 2);
            }
            break;
        }
        case NODE_MULEXP: {
            auto mulExpNode = static_cast<MulExpNode*>(node);
            cout << string(indent, ' ') << "MulExpNode" << endl;
            for (size_t i = 0; i < mulExpNode->operands.size(); ++i) {
                printAST(mulExpNode->operands[i].get(), indent + 2);
                if (i < mulExpNode->operators.size()) {
                    cout << string(indent + 2, ' ') << tokenTypeMap[mulExpNode->operators[i]] << endl;
                }
            }
            break;
        }
        case NODE_ADDEXP: {
            auto addExpNode = static_cast<AddExpNode*>(node);
            cout << string(indent, ' ') << "AddExpNode" << endl;
            for (size_t i = 0; i < addExpNode->operands.size(); ++i) {
                printAST(addExpNode->operands[i].get(), indent + 2);
                if (i < addExpNode->operators.size()) {
                    cout << string(indent + 2, ' ') << tokenTypeMap[addExpNode->operators[i]] << endl;
                }
            }
            break;
        }
        case NODE_RELEXP: {
            auto relExpNode = static_cast<RelExpNode*>(node);
            cout << string(indent, ' ') << "RelExpNode" << endl;
            for (size_t i = 0; i < relExpNode->operands.size(); ++i) {
                printAST(relExpNode->operands[i].get(), indent + 2);
                if (i < relExpNode->operators.size()) {
                    cout << string(indent + 2, ' ') << tokenTypeMap[relExpNode->operators[i]] << endl;
                }
            }
            break;
        }
        case NODE_EQEXP: {
            auto eqExpNode = static_cast<EqExpNode*>(node);
            cout << string(indent, ' ') << "EqExpNode" << endl;
            for (size_t i = 0; i < eqExpNode->operands.size(); ++i) {
                printAST(eqExpNode->operands[i].get(), indent + 2);
                if (i < eqExpNode->operators.size()) {
                    cout << string(indent + 2, ' ') << tokenTypeMap[eqExpNode->operators[i]] << endl;
                }
            }
            break;
        }
        case NODE_LANDEXP: {
            auto landExpNode = static_cast<LandExpNode*>(node);
            cout << string(indent, ' ') << "LandExpNode" << endl;
            for (size_t i = 0; i < landExpNode->operands.size(); ++i) {
                printAST(landExpNode->operands[i].get(), indent + 2);
                if (i < landExpNode->operators.size()) {
                    cout << string(indent + 2, ' ') << tokenTypeMap[landExpNode->operators[i]] << endl;
                }
            }
            break;
        }
        case NODE_LOREXP: {
            auto lorExpNode = static_cast<LorExpNode*>(node);
            cout << string(indent, ' ') << "LorExpNode" << endl;
            for (size_t i = 0; i < lorExpNode->operands.size(); ++i) {
                printAST(lorExpNode->operands[i].get(), indent + 2);
                if (i < lorExpNode->operators.size()) {
                    cout << string(indent + 2, ' ') << tokenTypeMap[lorExpNode->operators[i]] << endl;
                }
            }
            break;
        }
        case NODE_NUMBER: {
            auto numberNode = static_cast<NumberNode*>(node);
            cout << string(indent, ' ') << "NumberNode: " << numberNode->value << endl;
            break;
        }
        case NODE_CHARACTER: {
            auto characterNode = static_cast<CharacterNode*>(node);
            cout << string(indent, ' ') << "CharacterNode: " << characterNode->value << endl;
            break;
        }
        case NODE_CONSTEXP: {
            auto constExpNode = static_cast<ConstExpNode*>(node);
            cout << string(indent, ' ') << "ConstExpNode" << endl;
            break;
        }
        case NODE_RETURNSTMT: {
            auto returnStmtNode = static_cast<ReturnStmtNode*>(node);
            cout << string(indent, ' ') << "ReturnStmtNode" << endl;
            if (returnStmtNode->exp) {
                printAST(returnStmtNode->exp.get(), indent + 2);
            }
            break;
        }
        case NODE_BREAKSTMT: {
            cout << string(indent, ' ') << "BreakStmtNode" << endl;
            break;
        }
        case NODE_CONTINUESTMT: {
            cout << string(indent, ' ') << "ContinueStmtNode" << endl;
            break;
        }
        case NODE_PRINTFSTMT: {
            auto printfStmtNode = static_cast<PrintfStmtNode*>(node);
            cout << string(indent, ' ') << "PrintfStmtNode: " << printfStmtNode->format << endl;
            for (auto& arg : printfStmtNode->args) {
                printAST(arg.get(), indent + 2);
            }
            break;
        }
        case NODE_EMPTYSTMT: {
            cout << string(indent, ' ') << "EmptyStmtNode" << endl;
            break;
        }
        case NODE_ASSIGNSTMT: {
            auto assignStmtNode = static_cast<AssignStmtNode*>(node);
            cout << string(indent, ' ') << "AssignStmtNode" << endl;
            printAST(assignStmtNode->lval.get(), indent + 2);
            printAST(assignStmtNode->exp.get(), indent + 2);
            break;
        }
        case NODE_EXPSTMT: {
            auto expStmtNode = static_cast<ExpStmtNode*>(node);
            cout << string(indent, ' ') << "ExpStmtNode" << endl;
            printAST(expStmtNode->exp.get(), indent + 2);
            break;
        }
        case NODE_IFSTMT: {
            auto ifStmtNode = static_cast<IfStmtNode*>(node);
            cout << string(indent, ' ') << "IfStmtNode" << endl;
            printAST(ifStmtNode->ifcond.get(), indent + 2);
            printAST(ifStmtNode->thenStmt.get(), indent + 2);
            if (ifStmtNode->elseStmt) {
                printAST(ifStmtNode->elseStmt.get(), indent + 2);
            }
            break;
        }
        case NODE_FOR: {
            auto forNode = static_cast<ForNode*>(node);
            cout << string(indent, ' ') << "ForNode" << endl;
            cout <<"init"<<endl;
            if (forNode->init) {
                printAST(forNode->init.get(), indent + 2);
            }
            cout <<"forcond"<<endl;
            if (forNode->forcond) {
                printAST(forNode->forcond.get(), indent + 2);
            }
            cout <<"step"<<endl;
            if (forNode->step) {
                printAST(forNode->step.get(), indent + 2);
            }
            cout<<"body"<<endl;
            printAST(forNode->body.get(), indent + 2);
            break;
        }
        case NODE_SmallFor: {
            auto smallforNode = static_cast<SmallforstmtNode*>(node);
            printAST(smallforNode->lval.get(), indent + 2);
            printAST(smallforNode->exp.get(), indent + 2);
        }
        // 其他节点类型
    }
}