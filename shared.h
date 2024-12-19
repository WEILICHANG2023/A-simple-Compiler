#ifndef SHARED_H
#define SHARED_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include "lexer.h"

using namespace std;

// 提取行号
int extractLineNumber(const string& line);

// 比较函数，用于排序
bool compareLines(const string& line1, const string& line2);

// 合并错误
void MergeErrors();

// 打印Token
void PrintTokens(const vector<Token>& tokens);

std::string replaceCommentsWithSpaces(const std::string& input);

void processFile(const std::string& inputFileName, const std::string& outputFileName);

void deduplicateLines(const std::string& inputFileName, const std::string& outputFileName);

#endif // SHARED_H