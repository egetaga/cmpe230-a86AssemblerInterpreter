#include  <vector>
#include <list>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <cctype>
#include <bits/stdc++.h>


using namespace std;

/* Here we are defining necessary data structures to properly represent the processor*/
vector<string> tokens; // A vector into which each individual token of the source program is parsed
vector<int> memory(2<<15, -1); // A vector representing the 64KB memory of HYP86, value -1 means represents an unused byte
unordered_map<string, int> labels; // A map that links labels appearing in the source program to the memory location they point to
unordered_map<string, pair<int, string>> variables; // A map that links the names of the variables declared to a pair of their memory address and type
unordered_map<string, pair<int, int>> registers; // A map that links the names of the registers to a pair of their value and size in integers
unordered_map<string, bool> flags; // A map that links the names of the flags to their boolean value
unordered_set<string> instructions = {"NOP","NOT","JZ","JNZ","JE","JNE","JA","JAE","JB","JBE","JNAE","JNB","JNBE","JNC","JC","PUSH","POP","INT","MOV","ADD","SUB","MUL"
        ,"DIV","XOR","OR","AND","RCL","RCR","SHL","SHR", "JMP", "CMP"}; // A set containing the string values of all possible instructions supported by the processor
unordered_set<string> directives ={"DW","DB"}; // A set containing the string values of all possible directives supported by the processor
int instructionLim=-1;
/* Function declarations */
bool initializeTokens(ifstream& inFile); // Reads the input file and tokenizes it
bool decimal(string st, int& a); // Takes a string and writes it's decimal value to a parameter integer, returns false if the string is an invalid number
int fetchValue(string varName); // Takes a variable name as a string and returns it's value from the memory
void updateRegisters(string changedRegister); // Updates the values of registers depending on the register last changed
bool mov(int instructionNum); // Executes mov command
bool checkSyntax(string& instruction, int& i); // Does some basic syntax checking during tokenization
bool push(int instructionNum); // Executes push command
bool pop(int instructionNum); // Executes pop command
vector<int> lineNumber; // Stores the line number of each token
bool sub(int instructionNum); // Executes sub instruction
bool andf(int instructionNum); // Executes and instruction
void toUpperCase(string& token); // Turns a string uppercase
unsigned int arithmeticUnit(int a,int b, string  operation, char type); // Performs arithmetic operations on two operands
unsigned int arithmeticUnit(unsigned int a, string operation, char type); // Performs arithmetic operations on single operand
bool twoOperandArithmetic(string& operation, int instructionNum);
bool singleOperandArithmetic(string& operation, int instructionNum);
bool generalJump(string& op, int& index) ;
bool execute(int memoryIndexLimit);
bool interrupt(int instructionNum, int& finish);
bool stringFixer(string& op1, string& op2);
int main() {
    /* Here we are initializing our registers and flags */
    // 8 bit registers
    registers["AH"] = make_pair(0, 255);
    registers["AL"] = make_pair(0, 255);
    registers["BH"] = make_pair(0, 255);
    registers["BL"] = make_pair(0, 255);
    registers["CH"] = make_pair(0, 255);
    registers["CL"] = make_pair(0, 255);
    registers["DH"] = make_pair(0, 255);
    registers["DL"] = make_pair(0, 255);
    // 16 bit registers
    registers["AX"] = make_pair(0, 65535);
    registers["BX"] = make_pair(0, 65535);
    registers["CX"] = make_pair(0, 65535);
    registers["DX"] = make_pair(0, 65535);
    registers["DI"] = make_pair(0, 65535);
    registers["SP"] = make_pair(0xFFFE, 65535);
    registers["SI"] = make_pair(0, 65535);
    registers["BP"] = make_pair(0, 65535);
    // flags
    flags["ZF"] = 0; flags["AF"] = 0; flags["CF"] = 0; flags["SF"] = 0; flags["0F"] = 0;
    // Following code parses the input program and loads it into memory using the initializeTokens function
    ifstream inFile;
    inFile.open("../test.txt");
    initializeTokens(inFile);
    execute(instructionLim);

    //   Following code prints out the state of the processor to the standard output
    cout<<endl;
    cout<<"------------------------------------------------------------------"<<endl;
    cout<<"------------------------------------------------------------------"<<endl;
    cout<<"------------------------------------------------------------------"<<endl;
    cout<<"------------------------------------------------------------------"<<endl;
    cout<<"------------------------------------------------------------------"<<endl;

    for(auto iter= tokens.begin(); iter!=tokens.end(); iter++) {
        cout<<*iter<< " " << endl;
    }
    cout << endl;


    cout << "Memory "<< instructionLim<<endl;
    for (int i=0; i<instructionLim; i++) {
        cout << memory[i] << " ";
    }

    cout<<endl;
    for(int i=0; i<lineNumber.size(); i++) {
        cout << lineNumber[i] << " ";
    }
    cout<<endl;

    cout << "\n";
    cout << "\n";
    cout << "Registers\n";
    for (auto it = registers.begin(); it !=registers.end(); it++) {
        cout << it->first << " " << it->second.first << " " << it->second.second << "\n";
    }
    cout << "\n";
    cout << "Variables\n";
    for (auto it = variables.cbegin(); it !=variables.cend(); it++) {
        if(it->second.second=="DB")
        cout << it->first << " " << it->second.first << " " << it->second.second <<" value: "<< memory[it->second.first]<<"\n";
        if(it->second.second=="DW")
            cout << it->first << " " << it->second.first << " " << it->second.second <<" value: "<< memory[it->second.first] +memory[it->second.first+1]*256<<"\n";
    }
    cout << "\n";
    cout <<"Labels\n";
    for (auto it = labels.begin(); it !=labels.end(); it++) {
        cout << it->first << " " << it->second << "\n";
    }
    cout << "\nFlags\n";
    cout << "ZF = " << flags["ZF"] << " AF = " << flags["AF"]<<" CF = "<< flags["CF"] <<" SF = " << flags["SF"] << " OF = " << flags["OF"];
}

bool initializeTokens(ifstream& inFile) { // Function to read the input program, write the tokens to the tokens vector and initialize memory
    // reads every token into the tokens vector
    string token;
    vector<string> temptokens;
    vector<int> templineNumber;
    if (inFile.is_open()) {
        string line;
        int lineNum{1};
        while (getline(inFile, line)) {
            string curToken = "";
            while (line.front() == ' ') line.erase(0,1);
            while (line.back() == ' ') line.pop_back();
            for (int i=0; i<line.length(); i++) {
                char current = line[i];
                if (current == ' ' || current == ',') {
                    if (line[i-1] == 39 && line[i+1] == 39) curToken += current;
                    else if (curToken != "") {
                        if (curToken.front() != 39 || curToken.back() != 39) toUpperCase(curToken);
                        temptokens.push_back(curToken);
                        templineNumber.push_back(lineNum);
                        curToken = "";
                    }
                }
                else {
                    curToken += current;
                }
            }
            if (curToken != "") {
                if (curToken.front() != 39 || curToken.back() != 39) toUpperCase(curToken);
                temptokens.push_back(curToken);
                templineNumber.push_back(lineNum);
                curToken = "";
            }
            lineNum++;
        }
        int index = 0;
        while (index < temptokens.size()) {
            string curToken = temptokens[index];
            if (curToken == "B" || curToken == "W") {
                if (temptokens[index+1].front() == '[') {
                    while (curToken.back()!=']') {
                        index++;
                        curToken.append(temptokens[index]);
                    }
                }
            }
            else if (curToken.find('[') != string::npos) {
                while (curToken.back()!=']') {
                    index++;
                    curToken.append(temptokens[index]);
                }
            }
            tokens.push_back(curToken);
            lineNumber.push_back(templineNumber[index]);
            index++;
        }


    }
    //the world's most ineffective code
    for(int i=0; i<tokens.size(); i++) {
        token= tokens[i];
        if(token=="INC") {
            tokens[i]="ADD";
            tokens.insert(tokens.begin() + (i+2), "1");
            lineNumber.insert(lineNumber.begin()+(i+2), lineNumber[i+1] );
        }
        else if(token=="DEC") {
            tokens[i]="SUB";
            tokens.insert(tokens.begin()+(i+2), "1");
            lineNumber.insert(lineNumber.begin()+ (i+2), lineNumber[i+1]);
        }
    }
    // loads the program into memory
    int curPos = 0;
    for (int i = 0; i < tokens.size(); i++) {
        string curToken = tokens[i];
        if(i!=0&&lineNumber[i-1]!=lineNumber[i]&&instructions.find(curToken)==instructions.end()&&curToken!="CODE"&&curToken.back()!=':') {
            if(!((i+1<tokens.size()&&tokens[i+1]=="DW")||tokens[i+1]=="DB" ))
            cout<<"Error at line "<<lineNumber[i]<<" Unknown menomonic: "<< curToken<<endl;
        }
        else if(i!=0&& lineNumber[i-1]==lineNumber[i]&& (instructions.find(curToken)!=instructions.end()||curToken.back()==':')) {
            cout<<"Error at line "<<lineNumber[i]<<" Invalid mnemonic: "<< curToken<<endl;
        }
        if(i==0&& (tokens[i]!="CODE"||tokens[i+1]!="SEGMENT")) {  cout<<"Error at line "<<lineNumber[i]<<" Unknown menomonic: "<< curToken<<endl;
        return false; }

        if((i==tokens.size()-2)&&(tokens.at(tokens.size()-2)!="CODE"||tokens.at(tokens.size()-1)!="ENDS")) {
            cout<<"Error at line "<<lineNumber[i]<<" Unknown menomonic: "<< curToken<<endl;
        return false;}

        // if the token is an instruction, encode it to memory using 6 bytes, but first check whether it is syntatically true or not
        if (instructions.find(curToken) != instructions.end()) {

            if(instructionLim==-1&&curToken=="INT"&& (i+1)<tokens.size()&&tokens[i+1]=="20H") instructionLim= (curPos+6);

            int posInTokens= i;
            if(!checkSyntax(curToken, i)) return false;
            for (int j = 0; j < 6; j++) {
                memory[curPos] = posInTokens;
                curPos++;
            }
        }
            // if the token is a label encode the memory location it refers to
        else if (curToken.back() == ':') {
            curToken.pop_back();
            labels[curToken] = curPos;
            continue;
        }
            // if the token is a directive initialize the variable with proper type
        else if (directives.find(curToken) != directives.end()) {
            string varName = tokens[i - 1];
            string varValue = tokens[i + 1];
            int value;
            variables[varName] = make_pair(curPos, curToken);
            // if variable value is a number
            if (decimal(varValue, value)) {
                if (curToken == "DB") {
                    if (value > 255) {
                        cout <<"Error at line number"<<lineNumber[i]<< " : Overflow";
                        return false;
                    } else {
                        memory[curPos++] = value;
                    }
                }
                if (curToken == "DW") {
                    if (value > 65535) {
                        cout <<"Error at line number"<<lineNumber[i]<< " : Overflow";
                        return false;
                    } else {
                        memory[curPos++] = ((unsigned short)value) & 0xff;
                        memory[curPos++] = (((unsigned short)value) >> 8u) & 0xff;
                    }
                }
            }
                // if variable value is a character
            else if (varValue.length() == 3 && varValue.front() == 39 && varValue.back() == 39) {
                value = varValue.at(1);
                if (curToken == "DB") {
                    if (value > 255) {
                        cout <<"Error at line number"<<lineNumber[i]<< " : Overflow";
                        return false;
                    } else {
                        memory[curPos++] = value;
                    }
                }
                if (curToken == "DW") {
                    if (value > 65535) {
                        cout <<"Error at line number"<<lineNumber[i]<< " : Overflow";
                        return false;
                    } else {
                        memory[curPos++] = value & 0xff;
                        memory[curPos++] = (value >> 8) & 0xff;
                    }
                }
            }
            else {
                cout <<"Error at line number "<<lineNumber[i]<< " : Invalid variable initialization. Must be a character or a number.";
                return false;
            }
        }
    }
    if(instructionLim==-1) {
        cout<<" Program must end with INT 20H in HYP86 "<<endl;
        return false;
    }
    return true;

}

bool execute(int memoryIndexLimit) {
    int memoryIndex=0;

    while(memoryIndex<=(memoryIndexLimit)) {

        int tokenIndex= memory[memoryIndex];
        string instruction= tokens[tokenIndex];

        if(instruction=="MOV"){
            if(!mov(tokenIndex)) return false;
        }
        else if(instruction=="ADD") {
            if(!twoOperandArithmetic(instruction, tokenIndex)) return false;
        }
        else if(instruction=="SUB"||instruction=="CMP"||instruction=="AND"||instruction=="OR"||instruction=="XOR"){
            if(!twoOperandArithmetic(instruction, tokenIndex)) return false;

        }
        else if(instruction=="RCL"||instruction=="RCR"||instruction=="SHL"||instruction=="SHR") {
            if(!twoOperandArithmetic(instruction, tokenIndex)) return false;
        }
        else if(instruction=="MUL"||instruction=="DIV"||instruction=="NOT") {
            if(!singleOperandArithmetic(instruction, tokenIndex)) return false;
        }
        else if(instruction=="PUSH") {
            if(!push(tokenIndex)) return false;

        }
        else if(instruction=="POP") {
            if(!pop(tokenIndex)) return false;
        }
        else if(instruction=="JNA"||instruction=="JZ"||instruction=="JNZ"||instruction=="JE"||instruction=="JNE"||instruction=="JA"||instruction=="JAE"||instruction=="JB"||instruction=="JBE"||instruction=="JMP"
       ||instruction=="JNAE"||instruction=="JNB"||instruction=="JNBE"||instruction=="JNC"||instruction=="JC"	) {
            int oldMem= memoryIndex;
            if(!generalJump(instruction, memoryIndex)) return false;
            if(memoryIndex!=oldMem) continue;
        }
        else if(instruction=="INT") {
            string next= tokens[tokenIndex+1];
            if(next!="20H"&&next!="21H") {
                cout<<"Error at line "<< lineNumber[tokenIndex]<<"Only 20H and 21H is defined in this project "<<endl;
                return false;
            }
            int a=-1;
            if(!interrupt(tokenIndex,a)) return false;
            if(a==0) return true;

        }
        else if (instruction == "NOP") {

        }
        else return false;

    memoryIndex+=6;

    }


return true;



}


bool decimal(string st, int& a) {
    if ((st.front()!='-'&&(st.front() < '0' || st.front() > '9'))||(st.front()=='-'&&(st.at(1)<'0'||st.at(1)>'9'))) return false;
    if(st.back()=='H') {
        a= stoi(st, nullptr, 16);
        return true;
    }
    if(st.back()=='B') {
        a=  stoi(st, nullptr, 2);

        return true;
    }
    if(st.back()=='D'|| (('0'<=st.back())&&(st.back()<='9'))) {

        a=  stoi(st);
        return true;
    }
    return false;
}

int fetchValue(string varName) {
    int address = variables[varName].first;
    if (variables[varName].second == "DB") {
        return memory[address];
    }
    else {
        return memory[address] + memory[address+1]*256;
    }
}

void updateRegisters(string changedRegister) {
    if (changedRegister == "AX") {
        registers["AL"].first = registers["AX"].first & 0xff;
        registers["AH"].first = (registers["AX"].first >> 8) & 0xff;
    }
    else if (changedRegister == "BX") {
        registers["BL"].first = registers["BX"].first & 0xff;
        registers["BH"].first = (registers["BX"].first >> 8) & 0xff;
    }
    else if (changedRegister == "CX") {
        registers["CL"].first = registers["CX"].first & 0xff;
        registers["CH"].first = (registers["CX"].first >> 8) & 0xff;
    }
    else if (changedRegister == "DX") {
        registers["DL"].first = registers["DX"].first & 0xff;
        registers["DH"].first = (registers["DX"].first >> 8) & 0xff;
    }
    else {
        registers["AX"].first = registers["AL"].first + registers["AH"].first*256;
        registers["BX"].first = registers["BL"].first + registers["BH"].first*256;
        registers["CX"].first = registers["CL"].first + registers["CH"].first*256;
        registers["DX"].first = registers["DL"].first + registers["DH"].first*256;
    }
}

bool mov(int instructionNum) {
    int op1= instructionNum+1;
    int op2= instructionNum+2;
    string destination = tokens[op1];
    string source = tokens[op2];
    stringFixer(destination, source);
    // cases where destination is a register
    if (registers.find(destination) != registers.end()) {
        // case where source is an immediate value
        int value;
        if (decimal(source, value)) {
            if(value<0) {
                if(value< -registers[destination].second) {
                    cout<<"Overflow"<<endl;
                    return false; }
                value+=(registers[destination].second+1);
            }
            if (registers[destination].second < value) {
                cout << "Overflow";
                return false;
            }
            else {
                registers[destination].first = value;
            }
        }
        else if (source.length() == 3 && source.front() == 39 && source.back() == 39) {
            registers[destination].first = source.at(1);
        }
        else if (source == "OFFSET") {
            string variable= tokens[op2+1];
            if(variables.find(variable)==variables.end()) {
                cout<<"Error at line: "<< lineNumber[instructionNum]<<" Offset must direct to memory address ";
                return false;
            }
            value = variables[variable].first;
            if (registers[destination].second < value) {
                cout <<"Error at line:"<<lineNumber[instructionNum] << " Overflow";
                return false;
            }
            else {
                registers[destination].first = value;
            }
        }
        // case where source is the contents of another register
        else if (registers.find(source) != registers.end()) {
            if (registers[destination].second != registers[source].second) {
                cout <<"Error at line:"<<lineNumber[instructionNum] << " Byte Word combination not allowed";
                return false;
            }
            else {
                registers[destination].first = registers[source].first;
            }
        }
        // case where the source is the contents of a memory offset pointed by a register
        else if (source.size() == 5 && source.at(1) == '[' && source.at(4) == ']'&&registers.find(source.substr(2,2))!=registers.end()) {
            string registerName = source.substr(2,2);
            if (registerName == "SI" || registerName == "DI" || registerName == "BX" || registerName == "BP") {
                char type = source.front();
                if(type!='B'&&type!='W') {
                    cout <<"Error at line:"<<lineNumber[instructionNum] << " Type can be only B or W";
                    return false;
                }
                if (type == 'B') {
                    if(registers[destination].second!=255) {
                        cout <<"Error at line:"<<lineNumber[instructionNum] << " Byte Word combination not allowed";
                        return false;
                    }
                    registers[destination].first = memory[registers[registerName].first];
                }
                if (type == 'W') {
                    if (registers[destination].second == 255) {
                        cout <<"Error at line:"<<lineNumber[instructionNum] << " Byte Word combination not allowed";
                        return false;
                    }
                    else {
                        registers[destination].first = memory[registers[registerName].first]+ memory[registers[registerName].first + 1] * 256;
                    }
                }
            }
            else if (!decimal(registerName, value)) {
                cout <<"Error at line:"<<lineNumber[instructionNum] << " Incorrect register name for addressing. Only following registers could be used for indexing in A86: SI, DI, BX, BP";
                return false;
            }
        }
        // case where the source is the contents of a memory offset
        else if ((source.size()>1)&&(source.at(1) == '[' && source.back() == ']')) {
            char type = source.front();
            if (type == 'B') {
                if(registers[destination].second!=255) {
                    cout <<"Error at line:"<<lineNumber[instructionNum] << " Byte Word combination not allowed";
                    return false;
                }
                string val = source.substr(2, source.length()-3);
                int value;
                if (decimal(val, value)) {

                    if(value<0||value>(65535)) {
                        cout <<"Error at line: "<<lineNumber[instructionNum] << " Index out of range ";
                        return false;
                    }
                    registers[destination].first = memory[value];
                }
                else {
                    cout <<"Error at line:"<<lineNumber[instructionNum] << "Incorrect memory address";
                    return false;
                }
            }
            else  if (type == 'W') {
                string val = source.substr(2, source.length()-3);
                int value;
                if (decimal(val, value)) {
                    if(value<0||value>(65535)) {
                        cout <<"Error at line: "<<lineNumber[instructionNum] << " Index out of range ";
                        return false;
                    }
                    if (registers[destination].second == 255) {
                        cout <<"Error at line:"<<lineNumber[instructionNum] << " Byte Word combination not allowed";
                        return false;
                    }
                    else {
                        registers[destination].first = memory[value] + memory[value+1]*256;
                    }
                }
                else {
                    cout <<"Error at line:"<<lineNumber[instructionNum] << " Incorrect memory address";
                    return false;
                }
            }
        }
        // case where the source is a variable
        else if ((variables.find(source) != variables.end())||(source=="B")||(source=="W")) {
            if(source=="B"||source=="W") {
                string  newSource = tokens[instructionNum+3];
                if(variables.find(newSource)==variables.end()) {
                    cout <<"Error at line:"<<lineNumber[instructionNum] << " after B OR W You should put valid variable ";
                    return false;
                }
                if ((variables[newSource].second=="DB"&&source=="W")||(variables[newSource].second=="DW"&&source=="B")) {
                    cout <<"Error at line:"<<lineNumber[instructionNum] << " after B, your variable type should be B, after W your variable type should be W";
                    return false;
                }
                source=newSource;
            }
            string type = variables[source].second;
            if (type == "DB") {
                if (registers[destination].second != 255) {
                    cout <<"Error at line:"<<lineNumber[instructionNum] << " Byte Word combination not allowed";
                    return false;
                }
                registers[destination].first = memory[variables[source].first];
            }
            if (type == "DW") {
                if (registers[destination].second == 255) {
                    cout <<"Error at line:"<<lineNumber[instructionNum] << " Byte Word combination not allowed";
                    return false;
                }
                registers[destination].first = memory[variables[source].first]+memory[variables[source].first+1] * 256;
            }
        }
        else {
            cout <<"Error at line:"<<lineNumber[instructionNum] << "Incorrect operand for MOV operation";
            return false;
        }
        updateRegisters(destination);
    }
    //TODO ADD SOURCE OFFSETS
    // cases where destination is a memory location
    else if ((destination.size()>1)&& destination.at(1) == '[' && destination.back() == ']') {
        if(destination.size()<4) {
            cout <<"Error at line:"<<lineNumber[instructionNum] << " Incorrect operand for MOV operation";
        }
        string val = destination.substr(2, destination.length()-3);
        int value;
        char type = destination.front();
        if (!decimal(val, value)) {
            if (val == "SI" || val == "DI" || val == "BX" || val == "BP") {
                value = registers[val].first;
            }
            else {
                cout <<"Error at line:"<<lineNumber[instructionNum] << "Incorrect register name for addressing. Only following registers could be used for indexing in A86: SI, DI, BX, BP";
                return false;
            }
        }
        // if source is an immediate value
        int constant;
        if (decimal(source, constant)) {
            if (type == 'B') {
                if (constant > 255 || constant < -255) {
                    cout <<"Error at line:"<<lineNumber[instructionNum] << "Overflow";
                    return false;
                }
                else {
                    memory[value] = (int)(char)constant;
                }
            }
            if (type == 'W') {
                if (constant > 65535 || constant < -65535) {
                    cout <<"Error at line:"<<lineNumber[instructionNum] << "Overflow";
                    return false;
                }
                else {
                    constant = (unsigned short int)constant;
                    memory[value] = constant & 0xff;
                    memory[value+1] = (constant >> 8) & 0xff;
                }
            }
        }
        else if (source.length() == 3 && source.front() == 39 && source.back() == 39) {
            constant = source.at(1);
            if (type == 'B' ) {
                memory[value] = constant;
            }
            if (type == 'W') {
                memory[value] = constant & 0xff;
                memory[value+1] = (constant >> 8) & 0xff;
            }
        }
        // if source is the contents of a register
        else if (registers.find(source) != registers.end()) {
            constant = registers[source].first;
            if (type == 'B' ) {
                if (constant > 255) {
                    cout <<"Error at line:"<<lineNumber[instructionNum] << "Overflow";
                    return false;
                }
                else {
                    memory[value] = constant;
                }
            }
            if (type == 'W') {
                memory[value] = constant & 0xff;
                memory[value+1] = (constant >> 8) & 0xff;
            }
        }

        else if (source == "OFFSET") {
            string variable= tokens[op2+1];
            if(variables.find(variable)==variables.end()) {
                cout<<"Error at line: "<< lineNumber[instructionNum]<<" Offset must direct to memory address ";
                return false;
            }
           int address = variables[variable].first;
            if ((type=='B'&&value>255)||value>65535) {
                cout <<"Error at line:"<<lineNumber[instructionNum] << " Overflow";
                return false;
            }
            else {
                if(type=='B')
                memory[value] = address;

                else if(type=='W') {
                 memory[value]= address & 0xff ;
                 memory[value+1]= (address>>8) & 0xff ;
                }

            }
        }
        else if ((source.size()==5) && source.at(1) == '[' && source.at(4) == ']'&&registers.find(source.substr(2,2))!=registers.end()) {
            cout<<"Error at line number"<<lineNumber[instructionNum]<<" Memory memory operations not allowed"<<endl;
            return false;
        }
        // case where the source is the contents of a memory offset
        else if ((source.size()>1)&&(source.at(1) == '[' && source.back() == ']')) {
            cout<<"Error at line number"<<lineNumber[instructionNum]<<" Memory memory operations not allowed"<<endl;
            return false;
        }

        //if source is variable
        else if ((variables.find(source) != variables.end())||(source=="B")||(source=="W")) {
            cout<<"Error at line number"<<lineNumber[instructionNum]<<" Memory memory operations not allowed"<<endl;
            return false;
        }
        else {
            cout <<"Error at line:"<<lineNumber[instructionNum] << "Incorrect operand for mov operation";
            return false;
        }
    }
    // case where the destination is a variable
    else if(destination=="B"||destination=="W"||variables.find(destination)!=variables.end()) {
        if(destination=="B"||destination=="W") {
            string type= destination;
            destination= source;
            source= tokens[op2+1];
            if(variables.find(destination)==variables.end()) {
                cout <<"Error at line:"<<lineNumber[instructionNum] << " Incorrect operand for mov operation, after B or W, there should be a variable";
            }
            if ((variables[destination].second=="DB"&&type=="W")||(variables[destination].second=="DW"&&destination=="B")) {
                cout <<"Error at line:"<<lineNumber[instructionNum] << " after B, your variable type should be, after W your variable type should be W";
                return false;
            }
            instructionNum++; //Why, otherwise we can't decide for source variable, whether we are supposed to shift or not
        }
        char type= variables[destination].second.back();
        int constant;
        int value= variables[destination].first;
        // if source is an immediate value
        if (decimal(source, constant)) {
            if (type == 'B') {
                if (constant > 255 || constant < -255) {
                    cout <<"Error at line:"<<lineNumber[instructionNum] << "Overflow";
                    return false;
                }
                else {
                    memory[value] = (int)(char)constant;  //are you sure this does work?
                }
            }
            if (type == 'W') {
                if (constant > 65535 || constant < -65535) {
                    cout <<"Error at line:"<<lineNumber[instructionNum] << "Overflow";
                    return false;
                }
                else {
                    constant = (unsigned short int)constant;
                    memory[value] = constant & 0xff;
                    memory[value+1] = (constant >> 8) & 0xff;
                }
            }
        }
        else if (source.length() == 3 && source.front() == 39 && source.back() == 39) {
            constant = source.at(1);
            if (type == 'B' ) {
                memory[value] = constant;
            }
            if (type == 'W') {
                memory[value] = constant & 0xff;
                memory[value+1] = (constant >> 8) & 0xff;
            }
        }
        //if the source is the contents of the register
        else if (registers.find(source) != registers.end()) {
            constant = registers[source].first;
            if (type == 'B' ) {
                if (constant > 255) {
                    cout <<"Error at line:"<<lineNumber[instructionNum] << "Overflow";
                    return false;
                }
                else {
                    memory[value] = constant;
                }
            }
            if (type == 'W') {
                memory[value] = constant & 0xff;
                memory[value+1] = (constant >> 8) & 0xff;
            }
        }
        //TODO check again, added recently
        else if (source == "OFFSET") {
            string variable= tokens[op2+1];
            if(variables.find(variable)==variables.end()) {
                cout<<"Error at line: "<< lineNumber[instructionNum]<<" Offset must direct to memory address ";
                return false;
            }
            int address = variables[variable].first;
            if ((type=='B'&&value>255)||value>65535) {
                cout <<"Error at line:"<<lineNumber[instructionNum] << " Overflow";
                return false;
            }
            else {
                if(type=='B')
                    memory[value] = address;

                else if(type=='W') {
                    memory[value]= address & 0xff ;
                    memory[value+1]= (address>>8) & 0xff ;
                }

            }
        }

        // case where the source is the contents of a memory offset pointed by a register
        else if ((source.size()==5) && source.at(1) == '[' && source.at(4) == ']'&& registers.find(source.substr(2,2))!=registers.end()) {
            cout <<"Error at line:"<<lineNumber[instructionNum] << " Memory memory operation is not possible in HYP86   "<<endl;
            return false;
        }
        //case where the source contains a memory address
        else if ((source.size()>1)&&(source.at(1) == '[' && source.back() == ']')) {
            cout <<"Error at line:"<<lineNumber[instructionNum] << " Memory memory operation is not possible in HYP86   "<<endl;
            return false;

        }
        else if ((variables.find(source) != variables.end())||(source=="B")||(source=="W")) {
            cout <<"Error at line:"<<lineNumber[instructionNum] << " Memory memory operation is not possible in HYP86   "<<endl;
            return false;
        }
        // to do: add cases where the source is offset and destination is a variable or memory location
        else {
            cout <<"Error at line:"<<lineNumber[instructionNum] << "Incorrect operand for mov operation";
            return false;
        }
    }
    return true;
}

bool checkSyntax(string& instruction, int& i) { //offsets are not considered here, need to improve on those
    if (instruction == "NOP") return true;
    int lineNominator=i;
    //I assumed, tokens are not "," or sth similar. Controls the tokens with 2 operands
    if (instruction == "MOV" || instruction == "ADD" || instruction == "AND" || instruction == "CMP" ||
        instruction == "XOR" || instruction == "SHL" ||
        instruction == "SHR" || instruction == "RCL" || instruction == "RCR" || instruction == "OR" || instruction =="SUB") {
        int k = i + 1;
        int l = i + 2;
        if (l >= tokens.size()) {
            cout <<"Line:"<<lineNumber[lineNominator]<< " Invalid operand to " << instruction << endl;
            return false;
        }
        string op1 = tokens[k];
        string op2 = tokens[l];
        if(op1=="OFFSET") {
            cout <<"Line:"<<lineNumber[lineNominator]<< " Invalid operand to " << instruction << endl;
            return false;
        }
        if(op1=="B"||op1=="W") {
            op1=op2;
            if ((++l) >= tokens.size()) {
                cout <<"Line:"<<lineNumber[lineNominator]<< " Invalid operand to " << instruction << endl;
                return false;
            }
            op2= tokens[l];
            i++;
        }
        if(op2=="OFFSET"||op2=="B"||op2=="W")  {
            if ((++l) >= tokens.size()) {
                cout <<"Line:"<<lineNumber[lineNominator]<< " Invalid operand to " << instruction << endl;
                return false;
            }
            op2= tokens[l];
            i++;
        }
        if(op2=="B"||op2=="W")  {
            if ((++l) >= tokens.size()) {
                cout <<"Line:"<<lineNumber[lineNominator]<< " Invalid operand to " << instruction << endl;
                return false;
            }
            op2= tokens[l];
            i++;
        }

        if ((instructions.find(op1) != instructions.end()) || instructions.find(op2) != instructions.end() ||
            op1[op1.length() - 1] == ':' || (op2.length()>=1&&op2[op2.length() - 1] == ':')||op1=="DB"||op2=="DW"||op1=="OFFSET"||op2=="OFFSET"||op1=="B"||op1=="W") {
            cout <<"Line:"<<lineNumber[lineNominator]<< " Invalid operand to " << instruction << endl;
            return false;
        }
        i += 2;
    }
    //controls INT specifically
  else  if (instruction == "INT") {
        if (((i + 1) >= tokens.size()) || (tokens[i + 1] != "20H" && tokens[i + 1] != "21H")) {
            cout <<"Line:"<<lineNumber[lineNominator]<< " Invalid operand to " << instruction << endl;
            return false;
        }
        i++;
    }
    //controls instructions with one operand
 else  if(instruction=="PUSH"||instruction=="POP"||instruction=="NOT"||instruction=="MUL"||instruction=="DIV"||instruction=="JZ"||instruction=="JMP"||
    instruction=="JNZ"||instruction=="JE"||instruction=="JNE"||instruction=="JA"||instruction=="JAE"||instruction=="JB"||instruction=="JBE"||instruction=="JC"||
    instruction=="JNAE"||instruction=="JNB"||instruction=="JNBE"||instruction=="JNC") {

        if((i+1)>=tokens.size()||instructions.find(tokens[i+1])!=instructions.end()|| tokens[i+1].back()==':'||tokens[i+1]=="DB"||tokens[i+1]=="DW" ) {
            cout <<"Line:"<<lineNumber[lineNominator]<< " Invalid operand to " << instruction << endl;
            return false;
        }
        // controls the case when -instruction- -offset- -name-
        if((instruction=="PUSH"||instruction=="POP"||instruction=="NOT"||instruction=="MUL"||instruction=="DIV")&&tokens[i+1]=="OFFSET") {
            if((i+2)>=tokens.size()||instructions.find(tokens[i+2])!=instructions.end()|| tokens[i+2].back()==':'||tokens[i+2]=="DB"||tokens[i+2]=="DW") {
                cout <<"Line:"<<lineNumber[lineNominator]<< " Invalid operand to " << instruction << endl;
                return false;
            }
            i++;
        }
        i++;
 }
// What have we done here? We simply controlled the basic rules regarding the syntax of the assembly code. Any other rule needing the preprocessed code should be controlled in
// runtime
    if((i+1)<=(lineNumber.size()-1) &&(lineNumber[i]==lineNumber[i+1])) {
        cout <<"Line:"<<lineNumber[lineNominator]<< " Invalid operand to " << instruction << endl;
            return false;}
    return true;
}
void toUpperCase(string& token) {
 if(token.size()==3&&((token.at(0)=='"'||token.at(0)=='\'')&&(token.at(2)=='"'||token.at(2)=='\''))) return;
  for(char & i : token) i=toupper(i);
}

bool push(int instructionNum) {
    string operand = tokens[instructionNum+1];
    // push the value of a register
    if (registers.find(operand)!= registers.end()) {
        if (registers[operand].second == 255) {
            cout << "Cannot push single byte values to the stack in A86";
            return false;
        }
        else {
            registers["SP"].first-=2;
            memory[registers["SP"].first] = registers[operand].first & 0xff;
            memory[registers["SP"].first+1]= (registers[operand].first >> 8) & 0xff;
            return true;
        }
    }
    // push an immediate value to the stack;
    int value;
    if (decimal(operand, value)) {
        if (value > 65535) {
            cout << "Overflow";
            return false;
        }
        registers["SP"].first-=2;
        memory[registers["SP"].first] = value & 0xff;
        memory[registers["SP"].first+1]= (value >> 8) & 0xff;
        return true;
    }
    else if (operand.front() == 39 && operand.back() == 39 && operand.length() == 3) {
        value = operand.at(1);
        registers["SP"].first-=2;
        memory[registers["SP"].first] = value & 0xff;
        memory[registers["SP"].first+1]= (value >> 8) & 0xff;
        return true;
    }
    else if (variables.find(operand) != variables.end()) {
        if (variables[operand].second == "DB") {
            cout << "Single byte variables cannot be pushed onto the stack";
            return false;
        }
        value = fetchValue(operand);
        registers["SP"].first-=2;
        memory[registers["SP"].first] = value & 0xff;
        memory[registers["SP"].first+1]= (value >> 8) & 0xff;
        return true;
    }
    // push an address of a variable to the stack
    else if (operand == "OFFSET") {
        string varName = tokens[instructionNum+2];
        if (variables.find(varName) == variables.end()) {
            cout << "Unknown Mnemonic";
            return false;
        }
        value = variables[varName].first;
        registers["SP"].first-=2;
        memory[registers["SP"].first] = value & 0xff;
        memory[registers["SP"].first+1]= (value >> 8) & 0xff;
        return true;
    }
    // push the value at a memory location to the stack
    else if (operand.at(1) == '[' && operand.back() == ']') {
        if (operand.front() == 'B') {
            cout << "Single byte values cannot be pushed onto the stack in A86.";
            return false;
        }
        else {
            string val = operand.substr(2,operand.length()-3);
            // number indexing
            if (decimal(val, value)) {
                registers["SP"].first-=2;
                memory[registers["SP"].first] = memory[value];
                memory[registers["SP"].first+1] = memory[value+1];
                return true;
            }
            // register indexing
            else if (registers.find(operand.substr(2,operand.length()-3)) != registers.end()) {
                if (val == "SI" || val == "DI" || val == "BX" || val == "BP") {
                    value = registers[val].first;
                    registers["SP"].first-=2;
                    memory[registers["SP"].first] = memory[value];
                    memory[registers["SP"].first+1] = memory[value+1];
                    return true;
                }
                else {
                    cout << "Incorrect register name for addressing. Only following registers could be used for indexing in A86: SI, DI, BX, BP";
                    return false;
                }
            }
        }
    }
    // push variable with type indicator to the stack
    else if (operand == "B") {
        cout << "Single byte values cannot be pushed onto the stack in A86.";
        return false;
    }
    else if (operand == "W") {
        string var = tokens[instructionNum+2];
        if (variables.find(var) == variables.end()) {
            cout << "Invalid mnemonic";
            return false;
        }
        else {
            value = fetchValue(var);
            registers["SP"].first-=2;
            memory[registers["SP"].first] = value & 0xff;
            memory[registers["SP"].first+1]= (value >> 8) & 0xff;
            return true;
        }
    }
    cout << "Invalid syntax " << "Line: " << lineNumber[instructionNum];
    return false;
}

bool pop(int instructionNum) {
    string operand = tokens[instructionNum+1];
    // pop into register
    if (registers.find(operand)!= registers.end()) {
        if (registers[operand].second == 255) {
            cout << "Expecting constant";
            return false;
        }
        else {
            registers[operand].first = memory[registers["SP"].first];
            registers[operand].first += (memory[registers["SP"].first+1]<<8);
            memory[registers["SP"].first] = -1;
            memory[registers["SP"].first+1] = -1;
            registers["SP"].first+=2;
            updateRegisters(operand);
            return true;
        }
    }
    // pop into a variable
    else if (variables.find(operand)!=variables.end()){
        if (variables[operand].second == "DB") {
            cout << "Cannot pop intro a byte variable";
            return false;
        }
        else {
            int address = variables[operand].first;
            memory[address] = memory[registers["SP"].first];
            memory[address+1] = memory[registers["SP"].first+1];
            memory[registers["SP"].first] = -1;
            memory[registers["SP"].first+1] = -1;
            registers["SP"].first+=2;
            return true;
        }
    }
    // pop into a memory location
    else if (operand.at(1) == '[' && operand.back() == ']') {
        int address;
        if (operand.front() == 'B') {
            cout << "Cannot pop into single byte values in A86.";
            return false;
        }
        else {
            string val = operand.substr(2,operand.length()-3);
            // number indexing
            if (decimal(val, address)) {
                memory[address] = memory[registers["SP"].first];
                memory[address+1] = memory[registers["SP"].first+1];
                memory[registers["SP"].first] = -1;
                memory[registers["SP"].first+1] = -1;
                registers["SP"].first+=2;
                return true;
            }
            // register indexing
            else if (registers.find(operand.substr(2,operand.length()-3)) != registers.end()) {
                if (val == "SI" || val == "DI" || val == "BX" || val == "BP") {
                    address = registers[val].first;
                    memory[address] = memory[registers["SP"].first];
                    memory[address+1] = memory[registers["SP"].first+1];
                    memory[registers["SP"].first] = -1;
                    memory[registers["SP"].first+1] = -1;
                    registers["SP"].first+=2;
                    return true;
                }
                else {
                    cout << "Incorrect register name for addressing. Only following registers could be used for indexing in A86: SI, DI, BX, BP";
                    return false;
                }
            }
        }
    }
    // pop into a variable with type indicator
    else if (operand == "B") {
        cout << "Line: " << lineNumber[instructionNum] << "Single byte values cannot be pushed onto the stack in A86.";
        return false;
    }
    else if (operand == "W") {
        string var = tokens[instructionNum+2];
        if (variables.find(var) == variables.end()) {
            cout << "Line: " << lineNumber[instructionNum] << "Invalid mnemonic";
            return false;
        }
        else {
            int address = variables[var].first;
            memory[address] = memory[registers["SP"].first];
            memory[address+1] = memory[registers["SP"].first+1];
            memory[registers["SP"].first] = -1;
            memory[registers["SP"].first+1] = -1;
            registers["SP"].first+=2;
            return true;
        }
    }
    cout << "Line: " << lineNumber[instructionNum] << " Invalid syntax";
    return false;
}

unsigned int arithmeticUnit(int op1, int op2, string  operation, char type) {
    if (type == 'B') {
        unsigned char firstValue = op1;
        unsigned char secondValue = op2;
        if (operation == "ADD") {
            unsigned char result = firstValue + secondValue;
            bool leftBit1 = (firstValue & (1<<7)) >> 7;
            bool leftBit2 = (secondValue & (1<<7)) >> 7;
            bool leftBitRes = (result & (1<<7)) >> 7;
            unsigned char lowNibble1 = firstValue & (0xF);
            unsigned char lowNibble2 = secondValue & (0xF);
            unsigned char lowNibbleRes = result & (0xF);
            flags["CF"] = result < firstValue && result < secondValue;
            flags["AF"] = lowNibbleRes < lowNibble1 && lowNibbleRes < lowNibble2;
            flags["OF"] = leftBit1 == leftBit2 && leftBit1 != leftBitRes;
            flags["SF"] = leftBitRes;
            flags["ZF"] = result == 0;
            return result;
        }
        if (operation == "SUB" || operation == "CMP") {
            unsigned char result = firstValue - secondValue;
            bool leftBit1 = (firstValue & (1<<7)) >> 7;
            bool leftBit2 = (secondValue & (1<<7)) >> 7;
            bool leftBitRes = (result & (1<<7)) >> 7;
            unsigned char lowNibble1 = firstValue & (0xF);
            unsigned char lowNibble2 = secondValue & (0xF);
            unsigned char lowNibbleRes = result & (0xF);
            flags["CF"] = firstValue < secondValue;
            flags["AF"] = lowNibble1 < lowNibble2;
            flags["OF"] = leftBit1 != leftBit2 && leftBit1 != leftBitRes;
            flags["SF"] = leftBitRes;
            flags["ZF"] = result == 0;
            return result;
        }
        if (operation == "AND") {
            unsigned char result = firstValue & secondValue;
            bool leftBit1 = (firstValue & (1<<7)) >> 7;
            bool leftBit2 = (secondValue & (1<<7)) >> 7;
            bool leftBitRes = (result & (1<<7)) >> 7;
            flags["CF"] = 0;
            flags["OF"] = 0;
            flags["SF"] = leftBitRes;
            flags["ZF"] = result == 0;
            return result;
        }
        if (operation == "OR") {
            unsigned char result = firstValue | secondValue;
            bool leftBit1 = (firstValue & (1<<7)) >> 7;
            bool leftBit2 = (secondValue & (1<<7)) >> 7;
            bool leftBitRes = (result & (1<<7)) >> 7;
            flags["CF"] = 0;
            flags["OF"] = 0;
            flags["SF"] = leftBitRes;
            flags["ZF"] = result == 0;
            return result;
        }
        if (operation == "XOR") {
            unsigned char result = firstValue ^ secondValue;
            bool leftBit1 = (firstValue & (1<<7)) >> 7;
            bool leftBit2 = (secondValue & (1<<7)) >> 7;
            bool leftBitRes = (result & (1<<7)) >> 7;
            flags["CF"] = 0;
            flags["OF"] = 0;
            flags["SF"] = leftBitRes;
            flags["ZF"] = result == 0;
            return result;
        }
        if (operation == "SHR") {
            unsigned char result = firstValue;
            if (secondValue == 0) return result;
            for (int i=0; i<secondValue; i++) {
                flags["CF"] = result & 1;
                result = result >> 1;
            }
            bool leftBit1 = (firstValue & (1<<7)) >> 7;
            bool leftBit2 = (secondValue & (1<<7)) >> 7;
            bool leftBitRes = (result & (1<<7)) >> 7;
            if (secondValue == 1) flags["OF"] = leftBit1;
            flags["SF"] = leftBitRes;
            flags["ZF"] = result == 0;
            return result;
        }
        if (operation == "SHL") {
            unsigned char result = firstValue;
            if (secondValue == 0) return result;
            for (int i=0; i<secondValue; i++) {
                flags["CF"] = (result & (1<<7)) >> 7;
                result = result << 1;
            }
            bool leftBit1 = (firstValue & (1<<7)) >> 7;
            bool leftBit2 = (secondValue & (1<<7)) >> 7;
            bool leftBitRes = (result & (1<<7)) >> 7;
            if (secondValue == 1) flags["OF"] = leftBitRes != flags["CF"];
            flags["SF"] = leftBitRes;
            flags["ZF"] = result == 0;
            return result;
        }
        if (operation == "RCR") {
            unsigned char result = firstValue;
            if (secondValue == 0) return result;
            for (int i=0; i<secondValue; i++) {
                int temp = flags["CF"];
                flags["CF"] = result & 1;
                result = result >> 1;
                unsigned char mask = 1 << 7;
                result = (result & ~mask) | ((temp << 7) & mask);
            }
            bool leftBit1 = (firstValue & (1<<7)) >> 7;
            bool leftBit2 = (secondValue & (1<<7)) >> 7;
            bool leftBitRes = (result & (1<<7)) >> 7;
            if (secondValue == 1) flags["OF"] = leftBitRes ^ ((result & (1<<6)) >> 6);
            flags["SF"] = leftBitRes;
            flags["ZF"] = result == 0;
            return result;
        }
        if (operation == "RCL") {
            unsigned char result = firstValue;
            if (secondValue == 0) return result;
            for (int i=0; i<secondValue; i++) {
                int temp = flags["CF"];
                flags["CF"] = (result & (1<<7)) >> 7;
                result = result << 1;
                unsigned char mask = 1;
                result = (result & ~mask) | ((temp) & mask);
            }
            bool leftBit1 = (firstValue & (1<<7)) >> 7;
            bool leftBit2 = (secondValue & (1<<7)) >> 7;
            bool leftBitRes = (result & (1<<7)) >> 7;
            if (secondValue == 1) flags["OF"] = leftBitRes ^ flags["CF"];
            flags["SF"] = leftBitRes;
            flags["ZF"] = result == 0;
            return result;
        }
    }
    else if (type == 'W') {
        unsigned short int firstValue = op1;
        unsigned short int secondValue = op2;
        if (operation == "ADD") {
            unsigned short int result = firstValue + secondValue;
            bool leftBit1 = (firstValue & (1<<15)) >> 15;
            bool leftBit2 = (secondValue & (1<<15)) >> 15;
            bool leftBitRes = (result & (1<<15)) >> 15;
            unsigned char lowNibble1 = firstValue & (0xF);
            unsigned char lowNibble2 = secondValue & (0xF);
            unsigned char lowNibbleRes = result & (0xF);
            flags["CF"] = result < leftBit1 && result < leftBit2;
            flags["AF"] = lowNibbleRes < lowNibble1 && lowNibbleRes < lowNibble2;
            flags["OF"] = leftBit1 == leftBit2 && leftBit1 != leftBitRes;
            flags["SF"] = leftBitRes;
            flags["ZF"] = result == 0;
            return result;
        }
        if (operation == "SUB" || operation == "CMP") {
            unsigned short int result = firstValue - secondValue;
            bool leftBit1 = (firstValue & (1<<15)) >> 15;
            bool leftBit2 = (secondValue & (1<<15)) >> 15;
            bool leftBitRes = (result & (1<<15)) >> 15;
            unsigned char lowNibble1 = firstValue & (0xF);
            unsigned char lowNibble2 = secondValue & (0xF);
            unsigned char lowNibbleRes = result & (0xF);
            flags["CF"] = firstValue < secondValue;
            flags["AF"] = lowNibble1 < lowNibble2;
            flags["OF"] = leftBit1 != leftBit2 && leftBit1 != leftBitRes;
            flags["SF"] = leftBitRes;
            flags["ZF"] = result == 0;
            return result;
        }
        if (operation == "AND") {
            unsigned short int result = firstValue & secondValue;
            bool leftBit1 = (firstValue & (1<<15)) >> 15;
            bool leftBit2 = (secondValue & (1<<15)) >> 15;
            bool leftBitRes = (result & (1<<15)) >> 15;
            flags["CF"] = 0;
            flags["OF"] = 0;
            flags["SF"] = leftBitRes;
            flags["ZF"] = result == 0;
            return result;
        }
        if (operation == "OR") {
            unsigned short int result = firstValue | secondValue;
            bool leftBit1 = (firstValue & (1<<15)) >> 15;
            bool leftBit2 = (secondValue & (1<<15)) >> 15;
            bool leftBitRes = (result & (1<<15)) >> 15;
            flags["CF"] = 0;
            flags["OF"] = 0;
            flags["SF"] = leftBitRes;
            flags["ZF"] = result == 0;
            return result;
        }
        if (operation == "XOR") {
            unsigned short int result = firstValue ^ secondValue;
            bool leftBit1 = (firstValue & (1<<15)) >> 15;
            bool leftBit2 = (secondValue & (1<<15)) >> 15;
            bool leftBitRes = (result & (1<<15)) >> 15;
            flags["CF"] = 0;
            flags["OF"] = 0;
            flags["SF"] = leftBitRes;
            flags["ZF"] = result == 0;
            return result;
        }
        if (operation == "SHR") {
            unsigned short int result = firstValue;
            if (secondValue == 0) return result;
            for (int i=0; i<secondValue; i++) {
                flags["CF"] = result & 1;
                result = result >> 1;
            }
            bool leftBit1 = (firstValue & (1<<15)) >> 15;
            bool leftBit2 = (secondValue & (1<<15)) >> 15;
            bool leftBitRes = (result & (1<<15)) >> 15;
            if (secondValue == 1) flags["OF"] = leftBit1;
            flags["SF"] = leftBitRes;
            flags["ZF"] = result == 0;
            return result;
        }
        if (operation == "SHL") {
            unsigned short int result = firstValue;
            if (secondValue == 0) return result;
            for (int i=0; i<secondValue; i++) {
                flags["CF"] = (result & (1<<15)) >> 15;
                result = result << 1;
            }
            bool leftBit1 = (firstValue & (1<<15)) >> 15;
            bool leftBit2 = (secondValue & (1<<15)) >> 15;
            bool leftBitRes = (result & (1<<15)) >> 15;
            if (secondValue == 1) flags["OF"] = leftBitRes != flags["CF"];
            flags["SF"] = leftBitRes;
            flags["ZF"] = result == 0;
            return result;
        }
        if (operation == "RCR") {
            unsigned short int result = firstValue;
            if (secondValue == 0) return result;
            for (int i=0; i<secondValue; i++) {
                int temp = flags["CF"];
                flags["CF"] = result & 1;
                result = result >> 1;
                unsigned short int mask = 1 << 15;
                result = (result & ~mask) | ((temp << 15) & mask);
            }
            bool leftBit1 = (firstValue & (1<<15)) >> 15;
            bool leftBit2 = (secondValue & (1<<15)) >> 15;
            bool leftBitRes = (result & (1<<15)) >> 15;
            if (secondValue == 1) flags["OF"] = leftBitRes ^ ((result & (1<<14)) >> 6);
            flags["SF"] = leftBitRes;
            flags["ZF"] = result == 0;
            return result;
        }
        if (operation == "RCL") {
            unsigned short int result = firstValue;
            if (secondValue == 0) return result;
            for (int i=0; i<secondValue; i++) {
                int temp = flags["CF"];
                flags["CF"] = (result & (1<<15)) >> 15;
                result = result << 1;
                unsigned short int mask = 1;
                result = (result & ~mask) | ((temp) & mask);
            }
            bool leftBit1 = (firstValue & (1<<15)) >> 15;
            bool leftBit2 = (secondValue & (1<<15)) >> 15;
            bool leftBitRes = (result & (1<<15)) >> 15;
            if (secondValue == 1) flags["OF"] = leftBitRes ^ flags["CF"];
            flags["SF"] = leftBitRes;
            flags["ZF"] = result == 0;
            return result;
        }
    }
    return INT_MAX;
}

unsigned int arithmeticUnit(unsigned int a, string operation, char type) {
    if (operation == "MUL") {
        if (type == 'B') {
            unsigned char val1 = registers["AL"].first;
            unsigned char val2 = a;
            unsigned short int result = val1 * val2;
            flags["CF"] = flags["OF"] = ((result >> 8) & 0xFF) != 0;
            return result;
        }
        if (type == 'W') {
            unsigned short int val1 = registers["AX"].first;
            unsigned short int val2 = a;
            unsigned int result = val1 * val2;
            flags["CF"] = flags["OF"] = ((result >> 16) & 0xFFFF) != 0;
            return result;
        }
    }
    if (operation == "NOT") {
        if (type == 'B') {
            return (unsigned char) ~a;
        }
        else return (unsigned short int) ~a;
    }
    return 0;
}
bool twoOperandArithmetic(string& operation, int instructionNum) {
    int op1= instructionNum+1;
    int op2= instructionNum+2;
    string destination = tokens[op1];
    string source = tokens[op2];
    stringFixer(destination, source);
    // cases where destination is a register
    if (registers.find(destination) != registers.end()) {
        // case where source is an immediate value
        int val1 = registers[destination].first;
        char optype;
        if (registers[destination].second == 255) {
            optype = 'B';
        }
        else {
            optype = 'W';
        }
        int value;
        if (decimal(source, value)) {
            if(value<0) {
                if(value< -registers[destination].second) {
                    cout<<"Line: " << lineNumber[instructionNum] << "Overflow"<<endl;
                    return false; }
                value+= (registers[destination].second+1);
            }
            if (registers[destination].second < value) {
                cout <<"Line: " << lineNumber[instructionNum] << "Overflow";
                return false;
            }
            else {
                if((operation=="RCL"||operation=="RCR"||operation=="SHL"||operation=="SHR")&&value>=32){
                    cout <<"Error at line: " << lineNumber[instructionNum] << " "<<operation<<" it should be either cl or a constant less than 32.";
                    return false;
                }

                if(operation!="CMP")   //CMP
                    registers[destination].first = arithmeticUnit(val1, value, operation, optype);
                else  arithmeticUnit(val1, value, operation, optype);
            }
        }
        else if (source.length() == 3 && source.front() == 39 && source.back() == 39) {
            if((operation=="RCL"||operation=="RCR"||operation=="SHL"||operation=="SHR")){
                cout <<"Error at line: " << lineNumber[instructionNum] << " "<<operation<<" :BAD SHIFT OPERAND- it should be either cl or a constant less than 32.";
                return false;
            }
            value = source.at(1);
            if(operation!="CMP") registers[destination].first = arithmeticUnit(val1, value, operation, optype);
            else arithmeticUnit(val1, value, operation, optype);
        }
        else if (source == "OFFSET") {

            if((operation=="RCL"||operation=="RCR"||operation=="SHL"||operation=="SHR")){
                cout <<"Error at line: " << lineNumber[instructionNum] << " "<<operation<<" :BAD SHIFT OPERAND- it should be either cl or a constant less than 32.";
                return false;
            }

            string variable= tokens[op2+1];
            if(variables.find(variable)==variables.end()) {
                cout<<"Error at line: "<< lineNumber[instructionNum]<<" Offset must direct to memory address ";
                return false;
            }
            value = variables[variable].first;
            if (registers[destination].second < value) {
                cout <<"Error at line:"<<lineNumber[instructionNum] << " Overflow";
                return false;
            }
            else {
                if(operation!="CMP") registers[destination].first = arithmeticUnit(val1, value, operation, optype);
                else arithmeticUnit(val1, value, operation, optype);
            }
        }
        // case where source is the contents of another register
        else if (registers.find(source) != registers.end()) {
            if((operation=="RCL"||operation=="RCR"||operation=="SHL"||operation=="SHR")&&source!="CL"){
                cout <<"Error at line: " << lineNumber[instructionNum] << " "<<operation<<" :BAD SHIFT OPERAND- it should be either cl or a constant less than 32.";
                return false;
            }

            if (registers[destination].second != registers[source].second) {
                cout <<"Error at line:"<<lineNumber[instructionNum] << " Byte Word combination not allowed";
                return false;
            }
            else {
                value = registers[source].first;
                if(operation!="CMP") registers[destination].first = arithmeticUnit(val1, value, operation, optype);
                else  arithmeticUnit(val1, value, operation, optype);
            }
        }
        // case where the source is the contents of a memory offset pointed by a register
        else if (source.size() == 5 && source.at(1) == '[' && source.at(4) == ']'&&registers.find(source.substr(2,2))!=registers.end()) {
            if((operation=="RCL"||operation=="RCR"||operation=="SHL"||operation=="SHR")){
                cout <<"Error at line: " << lineNumber[instructionNum] << " "<<operation<<" :BAD SHIFT OPERAND- it should be either cl or a constant less than 32.";
                return false;
            }
            string registerName = source.substr(2,2);
            if (registerName == "SI" || registerName == "DI" || registerName == "BX" || registerName == "BP") {
                char type = source.front();
                if(type!='B'&&type!='W') {
                    cout <<"Error at line:"<<lineNumber[instructionNum] << " Type can be only B or W";
                    return false;
                }
                if (type == 'B') {
                    if(registers[destination].second!=255) {
                        cout <<"Error at line:"<<lineNumber[instructionNum] << " Byte Word combination not allowed";
                        return false;
                    }
                    value = memory[registers[registerName].first];
                    if(operation!="CMP")
                        registers[destination].first = arithmeticUnit(val1, value, operation, optype);
                    else arithmeticUnit(val1, value, operation, optype);
                }
                if (type == 'W') {
                    if (registers[destination].second == 255) {
                        cout <<"Error at line:"<<lineNumber[instructionNum] << " Byte Word combination not allowed";
                        return false;
                    }
                    else {
                        value = memory[registers[registerName].first]+ memory[registers[registerName].first + 1] * 256 ;
                        if(operation!="CMP")  registers[destination].first = arithmeticUnit(val1, value, operation, optype);
                        else  arithmeticUnit(val1, value, operation, optype);
                    }
                }
            }
            else if (!decimal(registerName,value)){
                cout <<"Error at line:"<<lineNumber[instructionNum] << "Incorrect register name for addressing. Only following registers could be used for indexing in A86: SI, DI, BX, BP";
                return false;
            }
        }
        // case where the source is the contents of a memory offset
        else if ((source.size()>1)&&(source.at(1) == '[' && source.back() == ']')) {
            if((operation=="RCL"||operation=="RCR"||operation=="SHL"||operation=="SHR")){
                cout <<"Error at line: " << lineNumber[instructionNum] << " "<<operation<<" :BAD SHIFT OPERAND- it should be either cl or a constant less than 32.";
                return false;
            }
            char type = source.front();
            if (type == 'B') {
                if(registers[destination].second!=255) {
                    cout <<"Error at line:"<<lineNumber[instructionNum] << " Byte Word combination not allowed";
                    return false;
                }
                string val = source.substr(2, source.length()-3);
                int address;
                if (decimal(val, address)) {
                    value = memory[address];
                    if(operation!="CMP") registers[destination].first = arithmeticUnit(val1, value, operation,optype);
                    else arithmeticUnit(val1, value, operation,optype);
                }
                else {
                    cout <<"Error at line:"<<lineNumber[instructionNum] << "Incorrect memory address";
                    return false;
                }
            }
            else  if (type == 'W') {
                string val = source.substr(2, source.length()-3);
                int address;
                if (decimal(val, address)) {
                    if (registers[destination].second == 255) {
                        cout <<"Error at line:"<<lineNumber[instructionNum] << " Byte Word combination not allowed";
                        return false;
                    }
                    else {
                        value = memory[address]+ memory[address+1]*256;
                        if(operation!="CMP") registers[destination].first = arithmeticUnit(val1, value, operation, optype);
                        else arithmeticUnit(val1, value, operation, optype);
                    }
                }
                else {
                    cout <<"Error at line:"<<lineNumber[instructionNum] << "Incorrect memory address";
                    return false;
                }
            }
        }
        //case where the source is a variable
        else if ((variables.find(source) != variables.end())||(source=="B")||(source=="W")) {
            if((operation=="RCL"||operation=="RCR"||operation=="SHL"||operation=="SHR")){
                cout <<"Error at line: " << lineNumber[instructionNum] << " "<<operation<<" :BAD SHIFT OPERAND- it should be either cl or a constant less than 32.";
                return false;
            }
            if(source=="B"||source=="W") {
                string  newSource= tokens[instructionNum+3];
                if(variables.find(newSource)==variables.end()) {
                    cout <<"Error at line:"<<lineNumber[instructionNum] << "after B OR W You should put valid variable ";
                    return false;
                }
                if ((variables[newSource].second=="DB"&&source=="W")||(variables[newSource].second=="DW"&&source=="B")) {
                    cout <<"Error at line:"<<lineNumber[instructionNum] << " after B, your variable type should be, after W your variable type should be W";
                    return false;
                }
                source=newSource;
            }
            string type = variables[source].second;
            if (type == "DB") {
                if (registers[destination].second != 255) {
                    cout <<"Error at line:"<<lineNumber[instructionNum] << " Byte Word combination not allowed";
                    return false;
                }
                value = memory[variables[source].first];
                if(operation!="CMP") registers[destination].first = arithmeticUnit(val1, value, operation, optype);
                else arithmeticUnit(val1, value, operation, optype);
            }
            if (type == "DW") {
                if (registers[destination].second == 255) {
                    cout <<"Error at line:"<<lineNumber[instructionNum] << " Byte Word combination not allowed";
                    return false;
                }
                value = memory[variables[source].first]+memory[variables[source].first+1] * 256;
                if(operation!="CMP") registers[destination].first = arithmeticUnit(val1, value, operation, optype);
                else arithmeticUnit(val1, value, operation, optype);
            }
        }
        else {
            cout <<"Error at line:"<<lineNumber[instructionNum] << "Incorrect operand for "<<operation <<" operation";
            return false;
        }
        if(operation!="CMP")
            updateRegisters(destination);

    }
    // cases where destination is a memory location
    else if ((destination.size()>1)&& destination.at(1) == '[' && destination.back() == ']') {
        int val1;
        char optype;
        if(destination.size()<4) {
            cout <<"Error at line:"<<lineNumber[instructionNum] << "Incorrect operand for "<<operation <<" operation";
        }
        string val = destination.substr(2, destination.length()-3);
        int address;
        optype = destination.front();
        if (!decimal(val, address)) {
            if (val == "SI" || val == "DI" || val == "BX" || val == "BP") {
                address = registers[val].first;
            }
            else {
                cout <<"Error at line:"<<lineNumber[instructionNum] << "Incorrect register/number name for addressing. Only following registers could be used for indexing in A86: SI, DI, BX, BP";
                return false;
            }
        }
        if(address<0||address>(65535)) {
            cout <<"Error at line: "<<lineNumber[instructionNum] << " Index out of range ";
            return false;
        }
        if ( optype == 'B') {
            val1 = memory[address];
        }
        else if(optype=='W') {
            val1 = memory[address] + memory[address+1] * 256;
        }
        else  {
            cout << "Error at line:"<<lineNumber[instructionNum] << "Incorrect syntax.." << endl;
            return false;
        }
        // if source is an immediate value
        int value;
        if (decimal(source, value)) {
            if((operation=="RCL"||operation=="RCR"||operation=="SHL"||operation=="SHR")&&value>=32){
                cout <<"Error at line: " << lineNumber[instructionNum] << " "<<operation<<" :BAD SHIFT OPERAND- it should be either cl or a constant less than 32.";
                return false;
            }

            if (optype == 'B') {
                if (value > 255 || value < -255) {
                    cout <<"Error at line:"<<lineNumber[instructionNum] << "Overflow";
                    return false;
                }
                else {
                    value = (int)(char)value;
                    int result = arithmeticUnit(val1, value, operation, optype);
                    if(operation!="CMP")
                        memory[address] = result & 0xff;
                }
            }
            if (optype == 'W') {
                if ( value > 65535 || value < -65535) {
                    cout <<"Error at line:"<<lineNumber[instructionNum] << "Overflow";
                    return false;
                }
                else {
                    value = (unsigned short int)value;
                    int result = arithmeticUnit(val1, value, operation, optype);
                    if(operation!="CMP") {
                        memory[address] = result & 0xff;
                        memory[address+1] = (result >> 8) & 0xff;
                    }
                }

            }
        }
        else if (source.length() == 3 && source.front() == 39 && source.back() == 39) {
            if((operation=="RCL"||operation=="RCR"||operation=="SHL"||operation=="SHR")){
                cout <<"Error at line: " << lineNumber[instructionNum] << " "<<operation<<" :BAD SHIFT OPERAND- it should be either cl or a constant less than 32.";
                return false;
            }
            value = source.at(1);
            if (optype == 'B' ) {
                if(operation!="CMP")
                    memory[address] = arithmeticUnit(val1, value, operation, optype);
                else  arithmeticUnit(val1, value, operation, optype);
            }
            if (optype == 'W') {
                int result = arithmeticUnit(val1, value, operation, optype);
                if(operation!="CMP") {
                    memory[address] = result & 0xff;
                    memory[address+1] = (result >> 8) & 0xff;
                }
            }
        }
        // if source is the contents of a register
        else if (registers.find(source) != registers.end()) {

            if((operation=="RCL"||operation=="RCR"||operation=="SHL"||operation=="SHR")&&source!="CL"){
                cout <<"Error at line: " << lineNumber[instructionNum] << " "<<operation<<" :BAD SHIFT OPERAND- it should be either cl or a constant less than 32.";
                return false;
            }
            value = registers[source].first;
            if (optype == 'B' ) {
                if (value > 255) {
                    cout <<"Error at line:"<<lineNumber[instructionNum] << "Overflow";
                    return false;
                }
                else {
                    if(operation!="CMP") memory[address] =  arithmeticUnit(val1, value, operation, optype) ;
                    else arithmeticUnit(val1, value, operation, optype) ;
                }
            }
            if (optype == 'W') {
                int result = arithmeticUnit(val1, value, operation, optype);
                if(operation!="CMP") {
                    memory[address] = result & 0xff;
                    memory[address+1] = (result >> 8) & 0xff;
                }
            }
        }
        // case where the source is the contents of a memory offset pointed by a register
        else if ((source.size()==5) && source.at(1) == '[' && source.at(4) == ']'&&registers.find(source.substr(2, 2))!=registers.end()) {
            cout <<"Error at line:"<<lineNumber[instructionNum] << " MEMORY MEMORY OPERATIONS NOT ALLOWED ";
            return false;
        }
        // case where the source is the contents of a memory offset
        else if ((source.size()>1)&&(source.at(1) == '[' && source.back() == ']')) {
            cout <<"Error at line:"<<lineNumber[instructionNum] << " MEMORY MEMORY OPERATIONS NOT ALLOWED ";
            return false;
        }
        //if source is variable
        else if ((variables.find(source) != variables.end())||(source=="B")||(source=="W")) {
            cout <<"Error at line:"<<lineNumber[instructionNum] << " MEMORY MEMORY OPERATIONS NOT ALLOWED ";
            return false;
        }
        else {
            cout <<"Error at line:"<<lineNumber[instructionNum] << "Incorrect operand for" << operation;
            return false;
        }
    }
    // destination is a variable
    else if(destination=="B"||destination=="W"||variables.find(destination)!=variables.end()) {
        if(destination=="B"||destination=="W") {
            string type= destination;
            destination= source;
            source= tokens[op2+1];
            if(variables.find(destination)==variables.end()) {
                cout <<"Error at line:"<<lineNumber[instructionNum] << "Incorrect operand for operation "<<operation<< " after B or W, there should be a variable";
            }
            if ((variables[destination].second=="DB"&&type=="W")||(variables[destination].second=="DW"&&destination=="B")) {
                cout <<"Error at line:"<<lineNumber[instructionNum] << " after B, your variable type should be, after W your variable type should be W";
                return false;
            }
            instructionNum++; //Why, otherwise we can't decide for source variable, whether we are supposed to shift or not
        }
        char optype= variables[destination].second.back();
        int value;
        int address = variables[destination].first;
        int val1;
        if ( optype == 'B') {
            val1 = memory[address];
        }
        else {
            val1 = memory[address] + memory[address+1] * 256;
        }
        // if source is an immediate value
        if (decimal(source, value)) {

            if((operation=="RCL"||operation=="RCR"||operation=="SHL"||operation=="SHR")&&value>=32){
                cout <<"Error at line: " << lineNumber[instructionNum] << " "<<operation<<" :BAD SHIFT OPERAND- it should be either cl or a constant less than 32.";
                return false;
            }

            if (optype == 'B') {
                if (value > 255 || value < -255) {
                    cout <<"Error at line:"<<lineNumber[instructionNum] << "Overflow";
                    return false;
                }
                else {
                    value = (int)(char)value;  //are you sure this does work?
                    if(operation!="CMP") memory[address] = arithmeticUnit(val1, value, operation, optype);
                    else  arithmeticUnit(val1, value, operation, optype);
                }
            }
            if (optype == 'W') {
                if (value > 65535 || value< -65535) {
                    cout <<"Error at line:"<<lineNumber[instructionNum] << "Overflow";
                    return false;
                }
                else {
                    int result = arithmeticUnit(val1, value, operation, optype);
                    if(operation!="CMP")
                    {   memory[address] = result & 0xff;
                        memory[address+1] = (result >> 8) & 0xff;
                    }
                }
            }
        }
        else if (source.length() == 3 && source.front() == 39 && source.back() == 39) {
            if((operation=="RCL"||operation=="RCR"||operation=="SHL"||operation=="SHR")){
                cout <<"Error at line: " << lineNumber[instructionNum] << " "<<operation<<" :BAD SHIFT OPERAND- it should be either cl or a constant less than 32.";
                return false;
            }
            value = source.at(1);
            if (optype == 'B' ) {
                if(operation!="CMP") memory[address] = arithmeticUnit(val1, value, operation, optype);
                else arithmeticUnit(val1, value, operation, optype);
            }
            if (optype == 'W') {
                int result = arithmeticUnit(val1, value, operation, optype);
                if(operation!="CMP") {
                    memory[address] = result & 0xff;
                    memory[address+1] = (result >> 8) & 0xff;
                }
            }
        }
        //if the source is the contents of the register
        else if (registers.find(source) != registers.end()) {
            if((operation=="RCL"||operation=="RCR"||operation=="SHL"||operation=="SHR")&&source!="CL"){
                cout <<"Error at line: " << lineNumber[instructionNum] << " "<<operation<<" :BAD SHIFT OPERAND- it should be either cl or a constant less than 32.";
                return false;
            }
            value = registers[source].first;
            if (optype == 'B' ) {
                if (value > 255) {
                    cout <<"Error at line:"<<lineNumber[instructionNum] << "Overflow";
                    return false;
                }
                else {
                    if(operation!="CMP") memory[address] = arithmeticUnit(val1, value, operation, optype);
                    else arithmeticUnit(val1, value, operation, optype);
                }
            }
            if (optype == 'W') {
                int result = arithmeticUnit(val1, value, operation, optype);
                if(operation!="CMP") {
                    memory[address] = result & 0xff;
                    memory[address+1] = (result >> 8) & 0xff;
                }
            }
        }
        else if ((source.size()==5) && source.at(1) == '[' && source.at(4) == ']'&&registers.find(source.substr(2,2))!=registers.end()) {
            cout <<"Error at line:"<<lineNumber[instructionNum] << " MEMORY MEMORY OPERATIONS NOT ALLOWED ";
            return false;
        }
        //case where the source contains a memory address
        else if ((source.size()>1)&&(source.at(1) == '[' && source.back() == ']')) {
            cout <<"Error at line:"<<lineNumber[instructionNum] << " MEMORY MEMORY OPERATIONS NOT ALLOWED ";
            return false;
        }
        else if ((variables.find(source) != variables.end())||(source=="B")||(source=="W")) {
            cout <<"Error at line:"<<lineNumber[instructionNum] << " MEMORY MEMORY OPERATIONS NOT ALLOWED ";
            return false;
        }
        else {
            cout <<"Error at line:"<<lineNumber[instructionNum] << "Incorrect operand for "<< operation;
            return false;
        }
    }
    return true;
}
bool singleOperandArithmetic(string& operation, int instructionNum) {
    int opNum= instructionNum+1;
    string operand= tokens[opNum];
    if(operation=="DIV") {
        if (registers.find(operand) != registers.end()) {
            if(registers[operand].second==255) {
                unsigned  int divisor= registers[operand].first;
                if(divisor==0) {
                    cout <<"Error at line:"<<lineNumber[instructionNum] << "Divisor cannot be 0";
                    return false;
                }
                unsigned int quotient= registers["AX"].first/divisor;
                if(quotient>255) {
                    cout << "Error at line:" << lineNumber[instructionNum] << " Overflow in DIV operation, the result does not fit in 8 bits";
                    return false;
                }
                unsigned  int remainder= registers["AX"].first%divisor;
                registers["AL"].first=quotient;
                registers["AH"].first=remainder;
                updateRegisters("AL");
                updateRegisters("AH");
            }
            else if(registers[operand].second==65535) {
                unsigned int divisor= registers[operand].first;
                if(divisor==0) {
                    cout <<"Error at line:"<<lineNumber[instructionNum] << "Divisor cannot be 0";
                    return false;
                }
                unsigned int dividend= (( registers["DX"].first)*65536) + registers["AX"].first;
                unsigned int quotient= dividend/divisor;
                unsigned  int remainder= dividend%divisor;
                if(quotient>65535) {
                    cout << "Error at line:" << lineNumber[instructionNum] << " Overflow in DIV operation, the result does not fit in 8 bits";
                    return false;
                }
                registers["AX"].first= quotient;
                registers["DX"].first= remainder;
                updateRegisters("AX");
                updateRegisters("DX");
            }
            else {
                cout << "Error at line:" << lineNumber[instructionNum] << "Incorrect operand for DIV operation";
                return false;
            }
            return true;
        }
        else if(operand=="W"||operand=="B"||variables.find(operand)!=variables.end()) {
            if(operand=="W"||operand=="B") {
                string newOp= tokens[instructionNum+2];
                if(variables.find(newOp)==variables.end()|| variables[newOp].second.back()!=newOp.front() ) {
                    cout << "Error at line:" << lineNumber[instructionNum] << "Incorrect operand for DIV operation";
                    return false;
                }
                operand= newOp;
            }
            if(variables[operand].second.back()=='B') {
                unsigned  int divisor= memory[variables[operand].first];
                if(divisor==0) {
                    cout <<"Error at line:"<<lineNumber[instructionNum] << "Divisor cannot be 0";
                    return false;
                }
                unsigned int quotient= registers["AX"].first/divisor;
                if(quotient>255) {
                    cout << "Error at line:" << lineNumber[instructionNum] << "Overflow in DIV operation, the result does not fit in 8 bits";
                    return false;
                }
                unsigned  int remainder= registers["AX"].first%divisor;
                registers["AL"].first=quotient;
                registers["AH"].first=remainder;
                updateRegisters("AL");
                updateRegisters("AH");

            }
            else if(variables[operand].second.back()=='W') {
                unsigned int divisor= memory[variables[operand].first]+memory[variables[operand].first+1]*256;
                if(divisor==0) {
                    cout <<"Error at line:"<<lineNumber[instructionNum] << "Divisor cannot be 0";
                    return false;
                }
                unsigned int dividend= (( registers["DX"].first)*65536) + registers["AX"].first;
                unsigned int quotient= dividend/divisor;
                unsigned  int remainder= dividend%divisor;
                if(quotient>65535) {
                    cout << "Error at line:" << lineNumber[instructionNum] << "Overflow in DIV operation, the result does not fit in 8 bits";
                    return false;
                }
                registers["AX"].first= quotient;
                registers["DX"].first= remainder;
                updateRegisters("AX");
                updateRegisters("DX");

            }
            else {
                cout << "Error at line:" << lineNumber[instructionNum] << "Incorrect operand for DIV operation";
                return false;}

            return true;
        }
        else if((operand.size()==5) && operand.at(1) == '[' && operand.at(4) == ']'&& registers.find(operand.substr(2,2))!=registers.end() ) {
            char opType = operand.front();
            string registerName = operand.substr(2,2);
            if (!(registerName == "SI" || registerName == "DI" || registerName == "BX" || registerName == "BP")) {
                cout << "Error at line:" << lineNumber[instructionNum] << "Incorrect operand for DIV operation, Only certain registers can be used for indexation";
                return false;
            }
            if(opType=='B') {
                unsigned  int divisor= memory[registers[registerName].first];
                if(divisor==0) {
                    cout <<"Error at line:"<<lineNumber[instructionNum] << "Divisor cannot be 0";
                    return false;
                }
                unsigned int quotient= registers["AX"].first/divisor;
                if(quotient>255) {
                    cout << "Error at line:" << lineNumber[instructionNum] << "Overflow in DIV operation, the result does not fit in 8 bits";
                    return false;
                }
                unsigned  int remainder= registers["AX"].first%divisor;
                registers["AL"].first=quotient;
                registers["AH"].first=remainder;
                updateRegisters("AL");
                updateRegisters("AH");

            }
            else if(opType=='W') {
                unsigned int divisor= memory[registers[registerName].first]+ memory[registers[registerName].first+1]*256 ;
                if(divisor==0) {
                    cout <<"Error at line:"<<lineNumber[instructionNum] << "Divisor cannot be 0";
                    return false;
                }
                unsigned int dividend= ( registers["DX"].first)*65536 + registers["AX"].first;
                unsigned int quotient= dividend/divisor;
                unsigned  int remainder= dividend%divisor;
                if(quotient>65535) {
                    cout << "Error at line:" << lineNumber[instructionNum] << "Overflow in DIV operation, the result does not fit in 8 bits";
                    return false;
                }
                registers["AX"].first= quotient;
                registers["DX"].first= remainder;
                updateRegisters("AX");
                updateRegisters("DX");

            }
            else {
                cout << "Error at line:" << lineNumber[instructionNum] << "Incorrect operand for DIV operation";
                return false;
            }

            return true;
        }

        else if ((operand.size()>1)&&(operand.at(1) == '[' && operand.back() == ']')) {
            char opType = operand.front();
            if (opType == 'B') {
                string stIndex = operand.substr(2, operand.length()-3);
                int index;
                if (decimal(stIndex, index)) {
                    unsigned int divisor = memory[index];
                    if(divisor==0) {
                        cout <<"Error at line:"<<lineNumber[instructionNum] << "Divisor cannot be 0";
                        return false;
                    }
                    unsigned int quotient= registers["AX"].first/divisor;
                    if(quotient>255) {
                        cout << "Error at line:" << lineNumber[instructionNum] << "Overflow in DIV operation, the result does not fit in 8 bits";
                        return false;
                    }
                    unsigned  int remainder= registers["AX"].first%divisor;
                    registers["AL"].first=quotient;
                    registers["AH"].first=remainder;
                    updateRegisters("AL");
                    updateRegisters("AH");
                }
                else {
                    cout <<"Error at line:"<<lineNumber[instructionNum] << "Incorrect memory address";
                    return false;
                }
            }
            else  if (opType == 'W') {

                string stIndex = operand.substr(2, operand.length()-3);
                int index;
                if (decimal(stIndex, index)) {
                    unsigned int divisor = memory[index] + memory[index+1] * 256;

                    if(divisor==0) {
                        cout <<"Error at line:"<<lineNumber[instructionNum] << "Divisor cannot be 0";
                        return false;
                    }
                    unsigned int dividend= ( registers["DX"].first)*65536 + registers["AX"].first;
                    unsigned int quotient= dividend/divisor;
                    unsigned  int remainder= dividend%divisor;
                    if(quotient>65535) {
                        cout << "Error at line:" << lineNumber[instructionNum] << "Overflow in DIV operation, the result does not fit in 8 bits";
                        return false;
                    }
                    registers["AX"].first= quotient;
                    registers["DX"].first= remainder;
                    updateRegisters("AX");
                    updateRegisters("DX");


                }

                else {
                    cout <<"Error at line:"<<lineNumber[instructionNum] << "Incorrect memory address";
                    return false;
                }
            }
            else {
                cout << "Error at line:" << lineNumber[instructionNum] << "Incorrect operand for DIV operation";
                return false;
            }
            return true;
        }
        else {
            cout << "Error at line:" << lineNumber[instructionNum] << "Incorrect operand for DIV operation";
            return false;
        }

        return true;

    }
    else if(operation=="MUL"){
        if (registers.find(operand) != registers.end()) {
            if(registers[operand].second==255) {
                unsigned  int multiplier= registers[operand].first;
                unsigned int product= arithmeticUnit(multiplier, "MUL", 'B');
                registers["AX"].first=product;
                updateRegisters("AX");
            }
            else if(registers[operand].second==65535) {
                unsigned int multiplier= registers[operand].first;
                unsigned int product= arithmeticUnit(multiplier, "MUL", 'W');
                registers["AX"].first=product&(65535u);  //takes the lowest 16 bits of product
                registers["DX"].first= (product>>16u)&(65535u);
                updateRegisters("AX");
                updateRegisters("DX");
            }
            else {
                cout << "Error at line:" << lineNumber[instructionNum] << "Incorrect operand for MUL operation";
                return false;
            }

            return true;
        }
        else if(operand=="W"||operand=="B"||variables.find(operand)!=variables.end()) {
            if(operand=="W"||operand=="B") {
                string newOp= tokens[instructionNum+2];
                if(variables.find(newOp)==variables.end()|| variables[newOp].second.back()!=newOp.front() ) {
                    cout << "Error at line:" << lineNumber[instructionNum] << "Incorrect operand for MUL operation";
                    return false;
                }
                operand= newOp;
            }
            if(variables[operand].second.back()=='B') {
                unsigned  int multiplier= memory[variables[operand].first];;
                unsigned int product= arithmeticUnit(multiplier, "MUL", 'B');
                registers["AX"].first=product;
                updateRegisters("AX");

            }
            else if(variables[operand].second.back()=='W') {
                unsigned int multiplier= memory[variables[operand].first]+memory[variables[operand].first+1]*256;
                unsigned int product= arithmeticUnit(multiplier, "MUL", 'W');
                registers["AX"].first=product&(65535u);  //takes the lowest 16 bits of product
                registers["DX"].first= (product>>16u)&(65535u);
                updateRegisters("AX");
                updateRegisters("DX");
            }
            else {
                cout << "Error at line:" << lineNumber[instructionNum] << "Incorrect operand for MUL operation";
                return false;
            }
            return true;
        }
            //possible error here, check that in the inner part there is a register
        else if((operand.size()==5) && operand.at(1) == '[' && operand.at(4) == ']'&& registers.find(operand.substr(2,2))!=registers.end() ) {
            char opType = operand.front();
            string registerName = operand.substr(2,2);
            if (!(registerName == "SI" || registerName == "DI" || registerName == "BX" || registerName == "BP")) {
                cout << "Error at line:" << lineNumber[instructionNum] << "Incorrect operand for MUL operation, Only certain registers can be used for indexation";
                return false;
            }
            if(opType=='B') {
                unsigned  int multiplier= memory[registers[registerName].first];
                unsigned int product= arithmeticUnit(multiplier, "MUL", 'B');
                registers["AX"].first=product;
                updateRegisters("AX");
            }
            else if(opType=='W') {
                unsigned int multiplier= memory[registers[registerName].first]+ memory[registers[registerName].first+1]*256 ;;
                unsigned int product= arithmeticUnit(multiplier, "MUL", 'W');
                registers["AX"].first=product&(65535u);  //takes the lowest 16 bits of product
                registers["DX"].first= (product>>16u)&(65535u);
                updateRegisters("AX");
                updateRegisters("DX");
            }
            else {
                cout << "Error at line:" << lineNumber[instructionNum] << "Incorrect operand for MUL operation";
                return false;
            }
            return true;
        }

        else if ((operand.size()>1)&&(operand.at(1) == '[' && operand.back() == ']')) {
            char opType = operand.front();
            if (opType == 'B') {
                string stIndex = operand.substr(2, operand.length()-3);
                int index;
                if (decimal(stIndex, index)) {
                    unsigned  int multiplier= memory[index];
                    unsigned int product= arithmeticUnit(multiplier, "MUL", 'B');
                    registers["AX"].first=product;
                    updateRegisters("AX");

                }
                else {
                    cout <<"Error at line:"<<lineNumber[instructionNum] << "Incorrect memory address";
                    return false;
                }
            }
            else  if (opType == 'W') {
                string stIndex = operand.substr(2, operand.length()-3);
                int index;
                if (decimal(stIndex, index)) {
                    unsigned  int multiplier= memory[index] + memory[index+1] * 256;
                    unsigned int product= arithmeticUnit(multiplier, "MUL", 'W');
                    registers["AX"].first=product&(65535u);  //takes the lowest 16 bits of product
                    registers["DX"].first= (product>>16u)&(65535u);
                    updateRegisters("AX");
                    updateRegisters("DX");

                }

                else {
                    cout <<"Error at line:"<<lineNumber[instructionNum] << "Incorrect memory address";
                    return false;
                }
            }
            else {
                cout << "Error at line:" << lineNumber[instructionNum] << "Incorrect operand for MUL operation";
                return false;
            }
            return true;
        }
        else {
            cout << "Error at line:" << lineNumber[instructionNum] << "Incorrect operand for MUL operation";
            return false;
        }

        return true;

    }
    else if(operation=="NOT"){
        if (registers.find(operand) != registers.end()) {
            if(registers[operand].second==255) {
                unsigned  int opValue= registers[operand].first;
                unsigned int result= arithmeticUnit(opValue, "NOT", 'B');
                registers[operand].first=result;
                updateRegisters(operand);
            }
            else if(registers[operand].second==65535) {
                unsigned int opValue= registers[operand].first;
                unsigned int result= arithmeticUnit(opValue, "NOT", 'W');
                registers[operand].first= result;  //takes the lowest 16 bits of result
                updateRegisters(operand);
            }
            else {
                cout << "Error at line:" << lineNumber[instructionNum] << "Incorrect operand for NOT operation";
                return false;
            }

            return true;
        }
        else if(operand=="W"||operand=="B"||variables.find(operand)!=variables.end()) {
            if(operand=="W"||operand=="B") {
                string newOp= tokens[instructionNum+2];
                if(variables.find(newOp)==variables.end()|| variables[newOp].second.back()!=newOp.front() ) {
                    cout << "Error at line:" << lineNumber[instructionNum] << "Incorrect operand for NOT operation";
                    return false;
                }
                operand= newOp;
            }
            if(variables[operand].second.back()=='B') {
                unsigned  int opValue= memory[variables[operand].first];
                unsigned int result= arithmeticUnit(opValue, "NOT", 'B');
                memory[variables[operand].first]=result;
            }
            else if(variables[operand].second.back()=='W') {
                unsigned int opValue= memory[variables[operand].first]+memory[variables[operand].first+1]*256;
                unsigned int result= arithmeticUnit(opValue, "NOT", 'W');
                memory[variables[operand].first]=result&(255u);  //takes the lowest 8 bits of result
                memory[variables[operand].first+1]= (result>>(8u))&(255u);  // take the 8 higher bits
            }
            else {
                cout << "Error at line:" << lineNumber[instructionNum] << "Incorrect operand for NOT operation";
                return false;
            }
            return true;
        }
            //possible error here, check that in the inner part there is a register
        else if((operand.size()==5) && operand.at(1) == '[' && operand.at(4) == ']'&& registers.find(operand.substr(2,2))!=registers.end() ) {
            char opType = operand.front();
            string registerName = operand.substr(2,2);
            if (!(registerName == "SI" || registerName == "DI" || registerName == "BX" || registerName == "BP")) {
                cout << "Error at line:" << lineNumber[instructionNum] << "Incorrect operand for NOT operation, Only certain registers can be used for indexation";
                return false;
            }
            if(opType=='B') {
                unsigned  int opValue= memory[registers[registerName].first];
                unsigned int result= arithmeticUnit(opValue, "NOT", 'B');
                memory[registers[registerName].first]=result;
            }
            else if(opType=='W') {
                unsigned int opValue= memory[registers[registerName].first]+ memory[registers[registerName].first+1]*256;
                unsigned int result= arithmeticUnit(opValue, "NOT", 'W');
                memory[registers[registerName].first]=result&(255u);  //takes the lowest 8 bits of result
                memory[registers[registerName].first+1]= (result>>(8u))&(255u);  // take the 8 higher bits
            }
            else {
                cout << "Error at line:" << lineNumber[instructionNum] << "Incorrect operand for NOT operation";
                return false;
            }
            return true;
        }

        else if ((operand.size()>1)&&(operand.at(1) == '[' && operand.back() == ']')) {
            char opType = operand.front();
            if (opType == 'B') {
                string stIndex = operand.substr(2, operand.length()-3);
                int index;
                if (decimal(stIndex, index)) {
                    unsigned  int opValue= memory[index];
                    unsigned int result= arithmeticUnit(opValue, "NOT", 'B');
                    memory[index]=result;
                }
                else {
                    cout <<"Error at line:"<<lineNumber[instructionNum] << "Incorrect memory address";
                    return false;
                }
            }
            else  if (opType == 'W') {
                string stIndex = operand.substr(2, operand.length()-3);
                int index;
                if (decimal(stIndex, index)) {
                    unsigned int opValue=memory[index] + memory[index+1] * 256;
                    unsigned int result= arithmeticUnit(opValue, "NOT", 'W');
                    memory[index]=result&(255u);  //takes the lowest 8 bits of result
                    memory[index+1]= (result>>(8u))&(255u);  // takes the  higher 8 bits
                }
                else {
                    cout <<"Error at line:"<<lineNumber[instructionNum] << "Incorrect memory address";
                    return false;
                }
            }
            else {
                cout << "Error at line:" << lineNumber[instructionNum] << "Incorrect operand for NOT operation";
                return false;
            }
            return true;
        }
        else {
            cout << "Error at line:" << lineNumber[instructionNum] << "Incorrect operand for NOT operation";
            return false;
        }
        return true;
    }
    return true;
}
bool jmp(int& index) {
    //I assumed, this index is the index of
    int instructionNum= index;
    string token= tokens[memory[index]];
    string label= tokens[memory[index]+1];
    if(labels.find(label)==labels.end()) {
        cout << "Error at line:" << lineNumber[instructionNum] << "There is no label as "<<label;
        return false;
    }
    index= labels[label];
    return true;
}
bool generalJump(string& op, int& index)  {
    int instructionNum= index;
    if(op=="JMP"){
        if(!jmp(index)) return false;
    }
    else if(op=="JZ"||op=="JE"){
        if(flags["ZF"]) {
            if(!jmp(index)){
                return false;
            }  }
    }
    else if(op=="JNZ"||op=="JNE") {

        if(!flags["ZF"]) {
            if(!jmp(index)){
                return false;
            }  }
    }
    else if(op=="JA"||op=="JNBE"){
        if(!flags["ZF"]&&!flags["CF"]) {
            if(!jmp(index)){
                return false;
            }  }
    }
    else if(op=="JAE"||op=="JNB"||op=="JNC") {
        if(!flags["CF"]) {
            if(!jmp(index)){
                return false;
            }  }

    }
    else if(op=="JB"||op=="JNAE"||op=="JC"){
        if(flags["CF"]) {
            if(!jmp(index)){
                return false;
            }  }
    }
    else if(op=="JBE"||op=="JNA"){
        if(flags["ZF"]||flags["CF"]) {
            if(!jmp(index)){
                return false;
            }  }
    }
    else {
        cout << "Error at line:" << lineNumber[instructionNum] << "There is no instruction as "<<op;
        return false;
    }
    return true;
}

bool interrupt(int instructionNum, int& finish) {
    string operand = tokens[instructionNum+1];
    int value;
    if (decimal(operand, value)) {
        if (value == 0x20) {
            finish=0;
            return true;

        }
        else if (value == 0x21) {
            if (registers["AH"].first == 1) {
                char x;
                cin >> x;
                registers["AL"].first = x;
                updateRegisters("AL");
                return true;
            }
            if (registers["AH"].first == 2) {
                cout << (char) registers["DL"].first;
                registers["AL"].first=registers["DL"].first;
                return true;
            }
        }
    }
    return false;
}

bool stringFixer(string& op1, string& op2) {
    if (op1.front() == '[' && op1.back() == ']') {
        if (op2.front() == '[' && op2.back() == ']') {
            cout << "Incorrect operands";
            return false;
        }
        int val;
        if (variables.find(op2) != variables.end()) {
            op1.insert(0, 1,variables[op2].second.back());
        }
        else if (registers.find(op2) != registers.end()) {
            if (registers[op2].second == 255) {
                op1.insert(0, 1,'B');
            }
            else {
                op1.insert(0, 1,'W');
            }
        }
        else if (op2 == "OFFSET") {
            op1.insert(0, 1,'W');
        }
        else if (decimal(op2, val)) {
            if (val < 256 && val > -256) {
                cout << "Byte or Word?";
                return false;
            }
            op1.insert(0, 1, 'W');
        }
    }
    else if (op2.front() == '[' && op2.back() == ']') {
        int val;
        if (variables.find(op1) != variables.end()) {
            op2.insert(0, 1,variables[op1].second.back());
        }
        else if (registers.find(op1) != registers.end()) {
            if (registers[op1].second == 255) {
                op2.insert(0, 1,'B');
            }
            else {
                op2.insert(0, 1,'W');
            }
        }
    }
    return false;
}
