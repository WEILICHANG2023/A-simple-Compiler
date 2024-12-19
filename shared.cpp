#include "shared.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <unordered_set>

// 提取行号
int extractLineNumber(const string& line) {
    size_t pos = line.find(' ');
    return stoi(line.substr(0, pos));
}

// 比较函数，用于排序
bool compareLines(const string& line1, const string& line2) {
    return extractLineNumber(line1) < extractLineNumber(line2);
}

// 合并错误
void MergeErrors() {
    vector<string> allErrors;

    // 读取 lexer_error.txt
    ifstream lexerErrorFile("lexer_error.txt");
    string line;
    while (getline(lexerErrorFile, line)) {
        allErrors.push_back(line);
    }
    lexerErrorFile.close();

    // 读取 parser_error.txt
    ifstream parserErrorFile("parser_error.txt");
    while (getline(parserErrorFile, line)) {
        allErrors.push_back(line);
    }
    parserErrorFile.close();

    // 读取 symbol_error.txt
    ifstream symbolErrorFile("symbol_error.txt");
    while (getline(symbolErrorFile, line)) {
        allErrors.push_back(line);
    }
    symbolErrorFile.close();

    // 按行号排序
    sort(allErrors.begin(), allErrors.end(), compareLines);

    // 输出到 error.txt
    ofstream outputFile("error2.txt");
    for (const auto& error : allErrors) {
        outputFile << error << endl;
    }
    outputFile.close();
}

// 打印Token
void PrintTokens(const vector<Token>& tokens) {
    for (const auto& token : tokens) {
        std::cout << tokenTypeMap[token.type] << " ";
        std::cout << token.value << " ";
        std::cout << token.lineNumber << std::endl;
    }
}

std::string replaceCommentsWithSpaces(const std::string& input) {
    std::ostringstream output;
    bool inComment = false;

    for (size_t i = 0; i < input.size(); ++i) {
        if (!inComment && input[i] == '/' && i + 1 < input.size() && input[i + 1] == '*') {
            inComment = true;
            ++i; // Skip the '*'
        } else if (inComment && input[i] == '*' && i + 1 < input.size() && input[i + 1] == '/') {
            inComment = false;
            ++i; // Skip the '/'
        } else if (inComment) {
            if (input[i] == '\n') {
                output << '\n'; // Preserve newlines
            } else {
                output << ' '; // Replace other characters with spaces
            }
        } else {
            output << input[i]; // Copy non-comment characters as is
        }
    }

    return output.str();
}

void processFile(const std::string& inputFileName, const std::string& outputFileName) {
    std::ifstream inputFile(inputFileName);
    if (!inputFile.is_open()) {
        std::cerr << "Error: Could not open input file " << inputFileName << std::endl;
        return;
    }

    std::stringstream buffer;
    buffer << inputFile.rdbuf();
    std::string input = buffer.str();
    inputFile.close();

    std::string output = replaceCommentsWithSpaces(input);

    std::ofstream outputFile(outputFileName);
    if (!outputFile.is_open()) {
        std::cerr << "Error: Could not open output file " << outputFileName << std::endl;
        return;
    }

    outputFile << output;
    outputFile.close();
}

void deduplicateLines(const std::string& inputFileName, const std::string& outputFileName) {
    std::ifstream inputFile(inputFileName);
    if (!inputFile.is_open()) {
        std::cerr << "Error: Could not open input file " << inputFileName << std::endl;
        return;
    }

    std::ofstream outputFile(outputFileName);
    if (!outputFile.is_open()) {
        std::cerr << "Error: Could not open output file " << outputFileName << std::endl;
        return;
    }

    std::unordered_set<std::string> seenLines;
    std::string line;

    while (std::getline(inputFile, line)) {
        if (seenLines.find(line) == seenLines.end()) {
            seenLines.insert(line);
            outputFile << line << std::endl;
        }
    }

    inputFile.close();
    outputFile.close();
}
