#ifndef LEXER_H
#define LEXER_H

#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>


using namespace std;

// 定义单词类别码
enum TokenType {
    CONSTTK, INTTK, CHARTK, VOIDTK, MAINTK, IFTK, ELSETK,
    FORTK, BREAKTK, CONTINUETK, RETURNTK, PLUS, MINU, MULT,
    DIV, MOD, LSS, LEQ, GRE, GEQ, EQL, NEQ, ASSIGN, SEMICN,
    COMMA, LPARENT, RPARENT, LBRACK, RBRACK, LBRACE, RBRACE,
    IDENFR, INTCON, STRCON, CHRCON, GETINTTK, GETCHARTK, PRINTFTK,
    AND, OR, NOT, UNKNOWN
};

// 定义错误类别码
enum ErrorType {
    ERROR_UNKNOWN_TOKEN,
    ERROR_UNEXPECTED_CHAR,
    // 其他错误类型
};

// 单词类别码映射
extern unordered_map<string, TokenType> tokenMap;

// 单词类别码字符串映射
extern unordered_map<TokenType, string> tokenTypeMap;

// 错误类别码映射
extern unordered_map<ErrorType, string> errorMap;

struct Token {
    TokenType type;
    string value;
    int lineNumber;
};

class Lexer {
public:
    Lexer(const string& inputFile, const string& lexerOutputFile, const string& errorOutputFile);
    void analyze();
    vector<Token> getTokens() const { return tokens; }
    vector<pair<int, string>> getErrors() const { return errors; }

private:
    string inputFile;
    string lexerOutputFile;
    string errorOutputFile;
    ifstream input;
    ofstream lexerOutput;
    ofstream errorOutput;
    int lineNumber = 1;
    vector<Token> tokens;
    vector<pair<int, string>> errors;

    void openFiles();
    void closeFiles();
    void processToken(const string& token, TokenType type);
    void processError(ErrorType error);
    void processError(string error);
    void skipWhitespace();
    string readIdentifier();
    string readNumber();
    string readString();
    string readChar();
    TokenType getTokenType(const string& token);
};

#endif // LEXER_H