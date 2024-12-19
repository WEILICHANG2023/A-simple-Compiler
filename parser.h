#ifndef PARSER_H
#define PARSER_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <memory>
#include "lexer.h"
#include "ast.h"
using namespace std;


class Parser {
public:
    Parser(const vector<Token>& tokens, const string& parserOutputFile, const string& errorOutputFile);
    unique_ptr<ASTNode> parse();

private:
    vector<Token> tokens;
    string parserOutputFile;
    string errorOutputFile;
    ofstream parserOutput;
    ofstream errorOutput;
    int currentIndex = 0;

    void openFiles();
    void closeFiles();
    void processToken(TokenType type, const string& value);
    void processError(int lineNumber, const string& error);
    unique_ptr<ASTNode> compUnit();
    unique_ptr<ASTNode> decl();
    unique_ptr<ASTNode> constDecl();
    unique_ptr<ASTNode> constDef(string type,int line);
    unique_ptr<ASTNode> constInitVal(unique_ptr<ConstDefNode> &&constDefNode);
    unique_ptr<ASTNode> varDecl();
    unique_ptr<ASTNode> varDef(string type,int line);
    unique_ptr<ASTNode> initVal(unique_ptr<VarDefNode> &&varDefNode);
    unique_ptr<ASTNode> funcDef(string type,int line);
    unique_ptr<ASTNode> mainFuncDef();
    unique_ptr<ASTNode> funcFParams();
    unique_ptr<ASTNode> funcFParam();
    unique_ptr<ASTNode> block(bool blockisfor);
    unique_ptr<ASTNode> stmt(bool blockisfor = false);
    unique_ptr<ASTNode> exp();
    unique_ptr<ASTNode> cond();
    unique_ptr<ASTNode> lVal();
    unique_ptr<ASTNode> primaryExp();
    unique_ptr<ASTNode> unaryExp();
    unique_ptr<ASTNode> funcRParams();
    unique_ptr<ASTNode> mulExp();
    unique_ptr<ASTNode> addExp();
    unique_ptr<ASTNode> reExp();
    unique_ptr<ASTNode> eqExp();
    unique_ptr<ASTNode> laExp();
    unique_ptr<ASTNode> loExp();
    unique_ptr<ASTNode> number();
    unique_ptr<ASTNode> strcon(unique_ptr<ConstDefNode> && constDefNode);
    unique_ptr<ASTNode> strcon(unique_ptr<VarDefNode> && varDefNode);
    unique_ptr<ASTNode> str2char(char ch);
    unique_ptr<ASTNode> character();
    unique_ptr<ASTNode> constExp();
    unique_ptr<ASTNode> forStmt();


    Token currentToken() const;
    Token lookAhead(int offset) const;
    void match(TokenType expectedType);
};

void printAST(ASTNode* node, int indent = 0);

#endif // PARSER_H