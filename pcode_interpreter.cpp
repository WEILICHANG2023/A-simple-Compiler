#include "pcode_interpreter.h"
#include <limits>
using namespace std;

int funcblock_stacknum = 0;
/*处理同名数组的深度*/
int SameArrDeep = 0;

void PCodeInterpreter::run(const std::string& filename,const std::string& result) {
    instructions = parsePCodeFile(filename);
    programCounter = 0;
    outputfile.open(result);
    execute();
    outputfile.close();
}


/*文件预处理 */
std::vector<Instruction> PCodeInterpreter::parsePCodeFile(const std::string& filename) {
    std::vector<Instruction> instructions;
    std::ifstream file(filename);
    std::string line;

    while (std::getline(file, line)) {
        Instruction instr;
        size_t pos = line.find(' ');
        std::string opcodeStr = line.substr(0, pos);
        /*识别pcode*/
        if (opcodeStr == "DEF_VAR") {
            instr.opcode = DEF_VAR;
        } else if (opcodeStr == "PUSH") {
            instr.opcode = PUSH;
        } else if (opcodeStr == "STORE") {
            instr.opcode = STORE;
        } else if (opcodeStr == "LOAD") {
            instr.opcode = LOAD;
        } else if (opcodeStr == "ADD") {
            instr.opcode = ADD;
        } else if (opcodeStr == "SUB") {
            instr.opcode = SUB;
        } else if (opcodeStr == "MULT") {
            instr.opcode = MUL;
        } else if (opcodeStr == "DIV") {
            instr.opcode = DiV;
        } else if (opcodeStr == "MOD") {
            instr.opcode = MoD;
        } else if (opcodeStr == "GT") {
            instr.opcode = GT;
        } else if (opcodeStr == "LT") {
            instr.opcode = LT;
        } else if (opcodeStr == "GE") {
            instr.opcode = GE;
        } else if (opcodeStr == "LE") {
            instr.opcode = LE;
        } else if (opcodeStr == "EQ") {
            instr.opcode = EQ;
        } else if (opcodeStr == "NE") {
            instr.opcode = NE;
        } else if (opcodeStr == "AND") {
            instr.opcode = AnD;
        } else if (opcodeStr == "OR") {
            instr.opcode = O_R;
        } else if (opcodeStr == "JUMP_IF_FALSE") {
            instr.opcode = JUMP_IF_FALSE;
        } else if (opcodeStr == "JUMP_IF_FALSE_SHORT") {
            instr.opcode = JUMP_IF_FALSE_SHORT;
        } else if (opcodeStr == "JUMP_IF_TRUE_SHORT") {
            instr.opcode = JUMP_IF_TRUE_SHORT;
        } else if (opcodeStr == "JUMP") {
            instr.opcode = JUMP;
        } else if (opcodeStr == "PRINT") {
            instr.opcode = PRINT;
        } else if (opcodeStr == "CALL") {
            instr.opcode = CALL;
        } else if (opcodeStr == "RETURN") {
            instr.opcode = RETURN;
        } else if (opcodeStr == "RETURN_NULL") {
            instr.opcode = RETURN_NuLL;
        } else if (opcodeStr == "END_FUNC") {
            instr.opcode = END_FUNC;
        } else if (opcodeStr == "LOAD_PARAM") {
            instr.opcode = LOAD_PARAM;
        } else if (opcodeStr == "POP_VAR") {
            instr.opcode = POP_VAR;     /*补充 */
        } else if (opcodeStr == "GETINT") {
            instr.opcode = GETINT;
        } else if (opcodeStr == "GETCHAR") {
            instr.opcode = GETCHAR;
        } else if (opcodeStr == "ZHENG") {
            instr.opcode = ZHENG;
        } else if (opcodeStr == "FU") {
            instr.opcode = FU;
        } else if (opcodeStr == "FEI") {
            instr.opcode = FEI;
        } else if (opcodeStr == "LABEL") {
            instr.opcode = LABEL;
        } else if (opcodeStr == "FUNC_DEF") {
            instr.opcode = FUNC_DEF;
        } else if (opcodeStr == "STORE_arraysize") {
            instr.opcode = STORE_arraysize;
        } else if (opcodeStr == "STORE_arrayelement") {
            instr.opcode = STORE_arrayelement;
        } else if (opcodeStr == "STORE_arrayindex") {
            instr.opcode = STORE_arrayindex;
        } else if (opcodeStr == "LOAD_arrayelement") {
            instr.opcode = LOAD_arrayelement;
        } else if (opcodeStr == "STORE_funcf_arraysize") {
            instr.opcode = CFarraySize;
        } else if (opcodeStr == "LOAD_ARRPARAM") {
            instr.opcode = LOAD_ARRPARAM;
        } else if (opcodeStr == "FUNCBLOCKNOW") {
            instr.opcode = FUNCBLOCKNOW;
        } else {
            cout<<"=============="<<endl;
            cout<<opcodeStr<<endl;
            cout<<"=============="<<endl;
            continue; // Skip unknown opcodes
        }

        /*识别操作数*/
        while (pos != std::string::npos) {
            if(line.find('"')==string::npos){
                size_t nextPos = line.find(' ', pos + 1);
                std::string operand = line.substr(pos + 1, nextPos - pos - 1);
                instr.operands.push_back(operand);
                pos = nextPos;
            } else {
                size_t firstQuotePos = line.find('"');
                size_t secondQuotePos = line.find('"', firstQuotePos + 1);
                string format = line.substr(firstQuotePos + 1, secondQuotePos - firstQuotePos - 1);
                instr.operands.push_back(format);
                break;
            }
        }

        /*指令压栈*/
        instructions.push_back(instr);
    }

    return instructions;
}

/*执行操作 */
void PCodeInterpreter::execute() {
    while (programCounter < instructions.size()) { //跳转改pc
        //cout<<"pc: "<<programCounter<<endl;
        const Instruction& instr = instructions[programCounter];
        switch (instr.opcode) {
            case DEF_VAR: {
                //vector<int> var = {0};
                shared_ptr<vector<int>> var = make_shared<vector<int>>(1);
                /*记录是int还是char*/
                if(instr.operands[0].find("Char")!=string::npos){
                    //cout<<instr.operands[1]<<" is char"<<endl;
                    varischar[instr.operands[1]].push(true);
                } else {
                    varischar[instr.operands[1]].push(false);
                }
                if(instr.operands[0].find("Array")!=string::npos){
                    varisarray[instr.operands[1]].push(true);
                } else {
                    varisarray[instr.operands[1]].push(false);
                }
                variables[instr.operands[1]].push(var);//同名压栈，修改最上层。
                break;
            }
            case PUSH:  {
                numstack.push(std::stoi(instr.operands[0]));
                break;
            }
            case STORE: {
                //vector<int>& var = variables[instr.operands[0]].top();
                shared_ptr<vector<int>> var = variables[instr.operands[0]].top();
                if(varischar[instr.operands[0]].top()){
                    if(varisarray[instr.operands[0]].top() == false){
                        (*var)[0] = numstack.top() % 128;
                        numstack.pop();
                    } else {
                        /*存数组*/
                        cout<<"char array store : ERROR INSRT"<<endl;
                    }
                } else{
                    if(varisarray[instr.operands[0]].top() == false){
                        if (var) { // 检查 shared_ptr 是否为空
                            cout<<numstack.top()<<endl;
                            cout<<(*var).size()<<endl;
                            (*var)[0] = numstack.top();
                        } else {
                            cout << "Error: shared_ptr is null for variable " << instr.operands[0] << endl;
                        }
                        numstack.pop();
                    } else {
                        /*存数组*/
                        cout<<"int array store: ERROR INSRT"<<endl;
                    }
                }
                break;
            }
            case LOAD:  {
                if(varisarray[instr.operands[0]].top() == false){
                    cout<<instr.operands[0]<<"is not array"<<endl;
                    shared_ptr<vector<int>> var = variables[instr.operands[0]].top();
                    numstack.push((*var)[0]);  
                } else {
                    /*数组传参,加载入临时数组*/
                    shared_ptr<vector<int>> &var = variables[instr.operands[0]].top();
                    cout<<"load array "+instr.operands[0]<<endl;
                    
                    tmpintvecs.push((*var));
                    vecsname.push(instr.operands[0]);//记录实参的名字
                    sarray2namemap[instr.operands[0]].push(variables[instr.operands[0]].top());
                }
                break;
            }
            case ADD: {
                int b = numstack.top(); numstack.pop();
                int a = numstack.top(); numstack.pop();
                numstack.push(a + b);
                break;
            }
            case SUB: {
                int b = numstack.top(); numstack.pop();
                int a = numstack.top(); numstack.pop();
                numstack.push(a - b);
                break;
            }
            case MUL: {
                int b = numstack.top(); numstack.pop();
                int a = numstack.top(); numstack.pop();
                numstack.push(a * b);
                break;
            }
            case DiV: {
                int b = numstack.top(); numstack.pop();
                //cout<<"b is "<<b<<endl;
                int a = numstack.top(); numstack.pop();
                //cout<<"a is "<<a<<endl;
                numstack.push(a / b);
                break;
            }
            case MoD: {
                int b = numstack.top(); numstack.pop();
                int a = numstack.top(); numstack.pop();
                numstack.push(a % b);
                break;
            }
            case GT: {
                int b = numstack.top(); numstack.pop();
                int a = numstack.top(); numstack.pop();
                numstack.push(a > b ? 1 : 0);
                break;
            }
            case LT: {
                int b = numstack.top(); numstack.pop();
                int a = numstack.top(); numstack.pop();
                numstack.push(a < b ? 1 : 0);
                break;
            }
            case GE: {
                int b = numstack.top(); numstack.pop();
                int a = numstack.top(); numstack.pop();
                numstack.push(a >= b ? 1 : 0);
                break;
            }
            case LE: {
                int b = numstack.top(); numstack.pop();
                int a = numstack.top(); numstack.pop();
                numstack.push(a <= b ? 1 : 0);
                break;
            }
            case EQ: {
                int b = numstack.top(); numstack.pop();
                int a = numstack.top(); numstack.pop();
                numstack.push(a == b ? 1 : 0);
                break;
            }
            case NE: {
                int b = numstack.top(); numstack.pop();
                int a = numstack.top(); numstack.pop();
                numstack.push(a != b ? 1 : 0);
                break;
            }
            case AnD: {
                int b = numstack.top(); numstack.pop();
                int a = numstack.top(); numstack.pop();
                numstack.push(a && b ? 1 : 0);
                break;
            }
            case O_R: {
                int b = numstack.top(); numstack.pop();
                int a = numstack.top(); numstack.pop();
                numstack.push(a || b ? 1 : 0);
                break;
            }
            case JUMP_IF_FALSE: {
                int condition = numstack.top(); numstack.pop();
                if (condition == 0) {
                    for(int i=0;i<instructions.size();i++){
                        if(instructions[i].opcode == LABEL && instructions[i].operands[0]==instr.operands[0]){
                            programCounter = i;
                            break;
                        }
                    }
                    continue;
                }
                break;
            }
            case JUMP_IF_FALSE_SHORT: {
                int condition = numstack.top(); 
                if (condition == 0) {
                    cout<<"and跳"<<endl;
                    for(int i=0;i<instructions.size();i++){
                        if(instructions[i].opcode == LABEL && instructions[i].operands[0]==instr.operands[0]){
                            programCounter = i;
                            break;
                        }
                    }
                    continue;
                }
                break;
            }
            case JUMP_IF_TRUE_SHORT: {
                int condition = numstack.top(); 
                if (condition == 1) {
                    cout<<"or跳"<<endl;
                    for(int i=0;i<instructions.size();i++){
                        if(instructions[i].opcode == LABEL && instructions[i].operands[0]==instr.operands[0]){
                            programCounter = i;
                            break;
                        }
                    }
                    continue;
                }
                break;
            }
            case JUMP:  {
                for(int i=0;i<instructions.size();i++){
                    if(instructions[i].opcode != LABEL)
                        continue;
                    if(instructions[i].operands[0]==instr.operands[0]){
                        programCounter = i;
                    }
                }
                continue;
            }
            case PRINT: {
                string format = instr.operands[0];
                size_t pos = 0;
                int placeholderCount = 0;
                // 统计占位符数量
                while ((pos = format.find("%", pos)) != string::npos) {
                    if (format[pos + 1] == 'd' || format[pos + 1] == 'c') {
                        placeholderCount++;
                    }
                    pos += 2;
                }
                // 从栈中弹出相应数量的元素
                std::vector<int> args;
                for (int i = 0; i < placeholderCount; ++i) {
                    args.push_back(numstack.top());
                    numstack.pop();
                }
                // 从后往前替换格式化字符串中的占位符
                pos = 0;
                for (auto it = args.rbegin(); it != args.rend(); ++it) {
                    if ((pos = format.find("%", pos)) != string::npos) {
                        if (format[pos + 1] == 'd'){
                            format.replace(pos, 2, to_string(*it));
                        } else if(format[pos + 1] == 'c'){
                            format.replace(pos, 2, string(1, static_cast<char>(*it)));
                        }
                    }
                }
                while ((pos = format.find("\\n")) != string::npos) {
                    format.replace(pos, 2, "\n");
                }
                    // 如果没有换行符，直接输出
                outputfile << format;
                break;
            }
            case CALL:  {
                callStack.push(programCounter);
                //cout<<"call is "<<callStack.top()<<endl;
                programCounter = functable[instr.operands[0]];
                //cout<<"来到第"<<programCounter+1<<"行"<<endl;
                /*做标记*/
                //stackelenum_call = numstack.size();
                continue;
            } 
            case FUNCBLOCKNOW: {
                cout<<"FUNCBLOCKNOW"<<endl;
                cout<<"and stack size is "<<numstack.size()<<endl;
                funcblock_stacknum = numstack.size();
                break;
            }
            case RETURN: {
                //cout<<"RETURN"<<endl;
                int returnValue = numstack.top();
                numstack.pop();

                cout<<"stack size "<<numstack.size()<<endl;
                int n = numstack.size() - funcblock_stacknum;
                if(n<0){
                    cout<<"ERROR: STACK SIZE < 0 "<<endl;
                }
                else{
                    while (n--)
                    {
                        numstack.pop();
                    }
                    
                }
                numstack.push(returnValue);
                
                if (!callStack.empty()) {
                    programCounter = callStack.top();
                    //cout<<"下一次pc:"<<programCounter+1<<endl;
                    callStack.pop();
                } else {
                    //cout<<"callstack is empty"<<endl;
                }
                break;
            }
            case RETURN_NuLL: {
                cout<<"stack size "<<numstack.size()<<endl;
                int n = numstack.size() - funcblock_stacknum;
                if(n<0){
                    cout<<"ERROR: STACK SIZE < 0 "<<endl;
                }
                else{
                    while (n--)
                    {
                        numstack.pop();
                    }
                }
                if (!callStack.empty()) {
                    programCounter = callStack.top();
                    //cout<<"回到第"<<programCounter+1<<"行"<<endl;
                    callStack.pop();
                } 
                break;
            }
            case END_FUNC:  {
                if (!callStack.empty()) {
                    programCounter = callStack.top();
                    //cout<<"回到第"<<programCounter+1<<"行"<<endl;
                    callStack.pop();
                } 
                break;
            }
            case LOAD_PARAM:    {
                /**/
                //vector<int>& var = variables[instr.operands[1]].top();
                shared_ptr<vector<int>>& var = variables[instr.operands[1]].top();
                if(varisarray[instr.operands[1]].top() == false){
                    int n = stoi(instr.operands[0]);
                    stack<int> tempStack;
                    for (int i = 0; i < n; ++i) {
                        tempStack.push(numstack.top());
                        numstack.pop();
                    }
                    // 目标元素
                    int targetElement = numstack.top();
                    numstack.pop();
                    // 将临时栈中的元素依次弹出并压回原栈
                    while (!tempStack.empty()) {
                        numstack.push(tempStack.top());
                        tempStack.pop();
                    }
                    (*var)[0] = targetElement;
                }else{
                    cout<<"ERR,arr is defined var"<<endl;
                }
                break;
            }
            case LOAD_ARRPARAM:    {
                /*标*/
                //vector<int>& var = variables[instr.operands[1]].top();
                shared_ptr<vector<int>>& var = variables[instr.operands[1]].top();
                if(varisarray[instr.operands[1]].top() == false){
                    cout<<"ERR,var is defined arr"<<endl;
                }else{
                    int n = stoi(instr.operands[0]);
                    //cout<<"装入数组"<<instr.operands[1]<<endl;
                    //cout<<"n is "<<n<<endl;
                    stack<vector<int>> tmparray;
                    for(int i = 0;i<n;i++){
                        if(!tmpintvecs.empty()){
                            tmparray.push(tmpintvecs.top());
                            tmpintvecs.pop();
                        }
                    }
                    (*var) = tmpintvecs.top();//要赋值的真正的数组？已经赋值？*tmpintvecs是实参
                    tmpintvecs.pop();
                    while(!tmparray.empty()){
                        tmpintvecs.push(tmparray.top());
                        tmparray.pop();
                    }
                    /*找数组的形参实参对应关系*/
                    stack<string> tmpname;
                    for(int i=0;i<n;i++){
                        tmpname.push(vecsname.top());
                        vecsname.pop();
                    }
                    string shicanname=vecsname.top();
                    vecsname.pop();
                    while(!tmpname.empty()){
                        vecsname.push(tmpname.top());
                        tmpname.pop();
                    }
                    shared_ptr<vector<int>> real_arr_ptr = sarray2namemap[shicanname].top();
                    var = real_arr_ptr;
                    xarray2namemap[instr.operands[1]].push(real_arr_ptr);
                    cout<<"形参："<<instr.operands[1]<<" 实参："<<shicanname<<endl;
                }
                break;
            }
            case POP_VAR:   {
                variables[instr.operands[0]].pop();
                varischar[instr.operands[0]].pop();
                varisarray[instr.operands[0]].pop();
                if(!sarray2namemap[instr.operands[0]].empty()){
                    sarray2namemap[instr.operands[0]].pop();
                }
                if(!xarray2namemap[instr.operands[0]].empty()){
                    xarray2namemap[instr.operands[0]].pop();
                }
                break;
            }   /*补充*/
            case GETINT: {
                std::string line;
                std::getline(std::cin, line); // 读取一整行输入
                int value = std::stoi(line);  // 将字符串转换为整数
                numstack.push(value);
                break;
            }
            case GETCHAR: {
                char value;
                cout << "getchar" << endl;
                value = getchar(); // 使用 getchar() 读取一个字符，包括空格和换行符
                cout << "getchar is " << value << endl;
                numstack.push(static_cast<int>(value) & 0xFF); // 截取低8位
                break;
            }
            case ZHENG:  {
                break;
            }
            case FU:  {
                int value = numstack.top();
                numstack.pop();
                numstack.push(-value);
                break;
            }
            case FEI:  {
                int value = numstack.top();
                numstack.pop();
                numstack.push(!value);
                break;
            }
            case LABEL:  {
                break;
            }
            case FUNC_DEF:  {
                functable[instr.operands[0]] = programCounter + 2;
                break;
            }
            case STORE_arraysize:  {
                arraysize = numstack.top();
                numstack.pop();
                shared_ptr<vector<int>>& var = variables[instr.operands[0]].top();
                (*var).resize(arraysize);
                for(auto& it: (*var)){
                    it = 0;
                }
                break;
            }
            case CFarraySize:  {
                /**/
                break;
            }
            case STORE_arrayindex:  {
                arrayindex = numstack.top();
                numstack.pop();
                break;
            }
            case STORE_arrayelement:  { /*参数数组的改*/
                shared_ptr<vector<int>>& var = variables[instr.operands[0]].top();
                int idx = stoi(instr.operands[1]);
                if(idx == -1){
                    idx = arrayindex;
                    arrayindex = 0;
                }
                if(varischar[instr.operands[0]].top()){
                    (*var)[idx] = numstack.top() % 128;
                } else{
                    (*var)[idx] = numstack.top();
                }
                cout<<"store_arrayelement is "<<(*var)[idx]<<endl;
                numstack.pop();
                break;
            }
            case LOAD_arrayelement:  {
                /*也应该从数组里加载 */
                cout<<"load arrayelement"<<endl;
                shared_ptr<vector<int>>& var = variables[instr.operands[0]].top();
                cout<<arrayindex<<endl;
                cout<<(*var).size()<<endl;
                cout<<"取"<<instr.operands[0]<<"["<<arrayindex<<"]"<<" is "<<endl;
                for(auto it:(*var)){
                    cout<<it<<endl;
                }
                numstack.push((*var)[arrayindex]);
                break;
            }
        }
        programCounter++;
    }
}