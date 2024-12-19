#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <unordered_map>
#include <vector>
#include <string>

using namespace std;

const int smb_size =150;

struct SymbolEntry {
    string name;
    string type;
    int order;
    bool isConst = false; // 默认值为 false
    bool isFunction = false; // 默认值为 false
    bool isArray = false; // 默认值为 false
    vector<string> paramTypes = {}; // 函数参数类型
    int scopeLevel = 0; // 默认值为 0
};

class SymbolTable {
public:
  
    void enterScope(int num) {
        currentScopeLevel = num;
    }

    void exitScope(int num) {
        currentScopeLevel = num;
    }

    void addSymbol(const SymbolEntry& entry) {
        SymbolEntry newEntry = entry; // 创建一个新的 SymbolEntry 对象
        newEntry.scopeLevel = currentScopeLevel; // 设置作用域序号
        newEntry.order = declorder++;
        scopeStack[currentScopeLevel][newEntry.name] = newEntry; // 添加到符号表
        lastAddedSymbol = newEntry; // 记录最后一个添加的符号
    }

    void insertparamtypes(const SymbolEntry& entry){
        auto thentry = lookup(entry.name);
        thentry->paramTypes = entry.paramTypes;
    }

    bool Isrepeated(const string& name) {//b
        for (auto it : scopeStack[currentScopeLevel]) {
            if (it.first == name) {
                return true;
            }
        }
        return false;
    }

    bool Isundefined(const string& name,int funclevel) {//c
        //cout<<"funclevel: "<<funclevel<<endl;
        for (auto it : scopeStack[1]) {
            if (it.first == name) {
                //cout<<"c: "<<it.first<<" "<<it.second.scopeLevel<<endl;
                return false;
            }
        }
        for (int level = funclevel; level <= currentScopeLevel; ++level) {
            for (auto it : scopeStack[level]) {
                if (it.first == name) {
                    //cout<<"c: "<<it.first<<" "<<it.second.scopeLevel<<endl;
                    return false;
                }
            }
        }
        return true;
    }

    SymbolEntry* lookup(const string& name) {
        for (int level = currentScopeLevel; level >=0; --level) {
            for (auto& entry : scopeStack[level]) {
                if (entry.first == name) {
                    return &entry.second;
                }
            }
        }
        return nullptr;
    }

    int getCurrentLevel() const {
        return currentScopeLevel;
    }

    const SymbolEntry& getLastAddedSymbol() const {
        return lastAddedSymbol;
    }

    void dumpSymbolTable(const string& filename) const;

private:
    unordered_map<string, SymbolEntry> scopeStack[smb_size];
    int currentScopeLevel = 0;
    SymbolEntry lastAddedSymbol; // 记录最后一个添加的符号
    int declorder = 0;
};

#endif // SYMBOL_TABLE_H