#ifndef PCODE_INTERPRETER_H
#define PCODE_INTERPRETER_H

#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <stack>
#include <string>
#include <fstream>
#include <string>
#include <memory>
using namespace std;

enum Opcode {
    DEF_VAR,
    PUSH,
    STORE,
    LOAD,
    ADD,
    SUB,
    MUL,
    DiV,
    GT,
    LT,
    EQ,
    JUMP_IF_FALSE,
    JUMP,
    JUMP_IF_FALSE_SHORT,
    JUMP_IF_TRUE_SHORT,
    PRINT,
    CALL,
    RETURN,
    END_FUNC,
    LOAD_PARAM,
    POP_VAR,
    /*补充*/
    GETINT,
    GETCHAR,
    ZHENG,
    FU,
    FEI,
    LABEL,
    FUNC_DEF,
    STORE_arraysize,
    STORE_arrayelement,
    LOAD_arrayelement,
    STORE_arrayindex,
    MoD,
    NE,
    GE,
    LE,
    AnD,
    O_R,
    RETURN_NuLL,
    CFarraySize,
    LOAD_ARRPARAM,
    FUNCBLOCKNOW
};

struct Instruction {
    Opcode opcode;
    vector<string> operands;
    int index; //数组相关
};

class PCodeInterpreter {
public:
    void run(const string& filename,const string& input,const string& result);

private:
    ofstream   outputfile;
    ifstream   inputfile;
    vector<Instruction> instructions;
    size_t programCounter;
    //unordered_map<string, stack<vector<int>>> variables;//变量和数组合一
    unordered_map<string, stack<shared_ptr<vector<int>>>> variables;
    unordered_map<string, stack<bool>> varischar; 
    unordered_map<string, stack<bool>> varisarray;
    std::stack<int> numstack;
    stack<vector<int>> tmpintvecs;//传参临时数组
    stack<string> vecsname;//实参的名字
    unordered_map<string,stack<shared_ptr<vector<int>>>> sarray2namemap;//形参实参数组对照修改:实参
    unordered_map<string,stack<shared_ptr<vector<int>>>> xarray2namemap;//形参实参数组对照修改:形参

    std::stack<int> callStack;  // 用于保存函数调用时的返回地址
    vector<int> paramStack;  // 用于保存函数参数
    int arraysize;
    int arrayindex;
    unordered_map<string, int> functable;


    vector<Instruction> parsePCodeFile(const string& filename);
    void execute();
};

#endif // PCODE_INTERPRETER_H