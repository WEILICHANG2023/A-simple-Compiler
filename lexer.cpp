#include "lexer.h"
#include "ast.h"
#include "parser.h"

// 单词类别码映射
unordered_map<string, TokenType> tokenMap = {
    {"const", CONSTTK}, {"int", INTTK}, {"char", CHARTK}, {"void", VOIDTK},
    {"main", MAINTK}, {"if", IFTK}, {"else", ELSETK}, {"for", FORTK},
    {"break", BREAKTK}, {"continue", CONTINUETK}, {"return", RETURNTK},
    {"+", PLUS}, {"-", MINU}, {"*", MULT}, {"/", DIV}, {"%", MOD},
    {"<", LSS}, {"<=", LEQ}, {">", GRE}, {">=", GEQ}, {"==", EQL},
    {"!=", NEQ}, {"=", ASSIGN}, {";", SEMICN}, {",", COMMA},
    {"(", LPARENT}, {")", RPARENT}, {"[", LBRACK}, {"]", RBRACK},
    {"{", LBRACE}, {"}", RBRACE}, {"&&", AND}, {"||", OR}, {"!", NOT},
    {"getint", GETINTTK}, {"getchar", GETCHARTK}, {"printf", PRINTFTK}
};

// 单词类别码字符串映射
unordered_map<TokenType, string> tokenTypeMap = {
    {CONSTTK, "CONSTTK"}, {INTTK, "INTTK"}, {CHARTK, "CHARTK"}, {VOIDTK, "VOIDTK"},
    {MAINTK, "MAINTK"}, {IFTK, "IFTK"}, {ELSETK, "ELSETK"}, {FORTK, "FORTK"},
    {BREAKTK, "BREAKTK"}, {CONTINUETK, "CONTINUETK"}, {RETURNTK, "RETURNTK"},
    {PLUS, "PLUS"}, {MINU, "MINU"}, {MULT, "MULT"}, {DIV, "DIV"}, {MOD, "MOD"},
    {LSS, "LSS"}, {LEQ, "LEQ"}, {GRE, "GRE"}, {GEQ, "GEQ"}, {EQL, "EQL"},
    {NEQ, "NEQ"}, {ASSIGN, "ASSIGN"}, {SEMICN, "SEMICN"}, {COMMA, "COMMA"},
    {LPARENT, "LPARENT"}, {RPARENT, "RPARENT"}, {LBRACK, "LBRACK"}, {RBRACK, "RBRACK"},
    {LBRACE, "LBRACE"}, {RBRACE, "RBRACE"}, {AND, "AND"}, {OR, "OR"}, {NOT, "NOT"},
    {IDENFR, "IDENFR"}, {INTCON, "INTCON"}, {STRCON, "STRCON"}, {CHRCON, "CHRCON"},
    {GETINTTK, "GETINTTK"}, {GETCHARTK, "GETCHARTK"}, {PRINTFTK, "PRINTFTK"},
    {UNKNOWN, "UNKNOWN"}
};

// 错误类别码映射
unordered_map<ErrorType, string> errorMap = {
    {ERROR_UNKNOWN_TOKEN, "ERROR_UNKNOWN_TOKEN"},
    {ERROR_UNEXPECTED_CHAR, "ERROR_UNEXPECTED_CHAR"}
};

Lexer::Lexer(const string& inputFile, const string& lexerOutputFile, const string& errorOutputFile)
    : inputFile(inputFile), lexerOutputFile(lexerOutputFile), errorOutputFile(errorOutputFile) {}

void Lexer::analyze() {
    openFiles();
    char ch;
    skipWhitespace();
    while (input.get(ch)) {
        if (ch == '\n') {
            lineNumber++;
        }
        if (isspace(ch)) {
            skipWhitespace();
            continue;
        }
        if (isalpha(ch) || ch == '_') {
            input.unget();
            string identifier = readIdentifier();
            TokenType type = getTokenType(identifier);
            if (type == UNKNOWN) {
                processError(ERROR_UNKNOWN_TOKEN);
            } else {
                processToken(identifier, type);
            }
        } else if (isdigit(ch)) {
            input.unget();
            string number = readNumber();
            processToken(number, INTCON);
        } else if (ch == '"') {
            string str = readString();
            processToken(str, STRCON);
        } else if (ch == '\'') {
            string chr = readChar();
            processToken(chr, CHRCON);
        } else {
            string token;
            token += ch;
            char nextCh;
            input.get(nextCh);

            // 检查是否构成多字符运算符
            if (ch == '&' ) {
                if(nextCh == '&') {
                    token += nextCh;
                    processToken(token, AND);
                } else {
                    token += '&';
                    processToken(token, AND);
                    processError("a");
                    input.unget();
                }
            } else if (ch == '|') {
                if(nextCh == '|') {
                    token += nextCh;
                    processToken(token, OR);
                } else {
                    token += '|';
                    processToken(token, OR);
                    processError("a");
                    input.unget();
                }
            } else if (ch == '<' && nextCh == '=') {
                token += nextCh;
                processToken(token, LEQ);
            } else if (ch == '>' && nextCh == '=') {
                token += nextCh;
                processToken(token, GEQ);
            } else if (ch == '!' && nextCh == '=') {
                token += nextCh;
                processToken(token, NEQ);
            } else if (ch == '=' && nextCh == '=') {
                token += nextCh;
                processToken(token, EQL);
            } else {
                input.unget(); // 回退一个字符
                if (tokenMap.find(token) != tokenMap.end()) {
                    processToken(token, tokenMap[token]);
                } else {
                    processError(ERROR_UNEXPECTED_CHAR);
                }
            }
        }
    }
    closeFiles();
}

void Lexer::openFiles() {
    input.open(inputFile);
    lexerOutput.open(lexerOutputFile);
    errorOutput.open(errorOutputFile);
}

void Lexer::closeFiles() {
    input.close();
    lexerOutput.close();
    errorOutput.close();
}

void Lexer::processToken(const string& token, TokenType type) {
    tokens.push_back({type, token, lineNumber});
    lexerOutput << tokenTypeMap[type] << " " << token << " " << lineNumber << endl;
}

void Lexer::processError(ErrorType error) {
    errors.push_back({lineNumber, errorMap[error]});
}

void Lexer::processError(string error) {
    errorOutput << lineNumber << " " << error << endl;
}

void Lexer::skipWhitespace() {
    char ch;
    while (input.get(ch)) {
        if (isspace(ch)) {
            if (ch == '\n') {
                lineNumber++;
            }
        } else {
            input.unget(); // 不是空白字符或注释，回退一个字符
            break;
        }
    }
}

string Lexer::readIdentifier() {
    string identifier;
    char ch;
    while (input.get(ch) && (isalnum(ch) || ch == '_')) {
        identifier += ch;
    }
    if (input) input.unget();
    return identifier;
}

string Lexer::readNumber() {
    string number;
    char ch;
    while (input.get(ch) && isdigit(ch)) {
        number += ch;
    }
    if (input) input.unget();
    return number;
}

string Lexer::readString() {
    string str;
    char ch;
    str += '"';
    while (input.get(ch) && ch != '"') {
        str += ch;
    }
    if (ch == '"') {
        str += ch; // 保留结尾的双引号
    }
    return str;
}

string Lexer::readChar() {
    string chr;
    char ch;
    chr += '\'';
    input.get(ch); // 读取字符
    if (ch == '\\') {
        // 处理转义字符
        chr += ch;
        input.get(ch);
    }
    chr += ch;
    input.get(ch); // 跳过结尾的单引号
    chr += '\'';
    return chr;
}

TokenType Lexer::getTokenType(const string& token) {
    if (tokenMap.find(token) != tokenMap.end()) {
        return tokenMap[token];
    } else if (isdigit(token[0])) {
        return INTCON;
    } else if (isalpha(token[0]) || token[0] == '_') {
        return IDENFR;
    } else {
        return UNKNOWN;
    }
}
