#include <iostream>
#include <fstream>
#include <vector>
#include "lexer.h"
#include "parser.h"
#include "semantic_analyzer.h"
#include "symbol_table.h"
#include "shared.h"
#include "pcode_interpreter.h"

using namespace std;

int main() {
    // 读取输入文件
    ifstream inputFile("testfile.txt");
    if (!inputFile.is_open()) {
        cerr << "Error: Could not open testfile.txt" << endl;
        return 1;
    }
    //去掉注释
    processFile("testfile.txt", "testfile2.txt");
    // 词法分析
    Lexer lexer("testfile2.txt", "lexer.txt", "lexer_error.txt");
    lexer.analyze();
    vector<Token> tokens = lexer.getTokens();

    // 语法分析
    Parser parser(tokens, "parser.txt", "parser_error.txt");
    unique_ptr<ASTNode> ast = parser.parse();

    
    // 语义分析
    SemanticAnalyzer semanticAnalyzer(ast);
    semanticAnalyzer.analyze("symbol.txt", "symbol_error.txt", "P_code.txt");

    // 合并错误信息并输出到 error.txt
    MergeErrors();

    deduplicateLines("error2.txt","error.txt");

    PCodeInterpreter interpreter;
    interpreter.run("P_code.txt","pcoderesult.txt");

    //cout<<"program have been finished"<<endl;
    return 0;
}


