#include <vector>
#include <list>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <unordered_set>

using namespace std;

/* Here we are defining necessary data structures to properly represent the processor*/
vector<string> tokens; // A vector into which each individual token of the source program is parsed
vector<int> memory(2<<15, -1); // A vector representing the 64KB memory of HYP86, value -1 means represents an unused byte
unordered_map<string, int> labels; // A map that links labels appearing in the source program to the memory location they point to
unordered_map<string, pair<int, string>> variables; // A map that links the names of the variables declared to a pair of their memory address and type
unordered_map<string, pair<int, int>> registers; // A map that links the names of the registers to a pair of their value and size in integers
unordered_map<string, bool> flags; // A map that links the names of the flags to their boolean value
unordered_set<string> instructions = {"NOP","NOT","JZ","JNZ","JE","JNE","JA","JAE","JB","JBE","JNAE","JNB","JNBE","JNC","JC","PUSH","POP","INT","MOV","ADD","SUB","MUL"
        ,"DIV","XOR","OR","AND","RCL","RCR","SHL","SHR"}; // A set containing the string values of all possible instructions supported by the processor
unordered_set<string> directives ={"DW","DB"}; // A set containing the string values of all possible directives supported by the processor

/* Function declarations */
bool initializeTokens(ifstream& inFile);
bool decimal(string st, int& a);
int fetchValue(string varName);
void updateRegisters(string changedRegister);
bool mov(int op1, int op2);

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

    /* Following code parses the input program and loads it into memory using the initializeTokens function */
    ifstream inFile;
    inFile.open("../test.txt");
    initializeTokens(inFile);

    mov(1,2);

    /* Following code prints out the state of the processor to the standard output */
    cout << "Memory\n";
    for (int i=0; i<40; i++) {
        cout << memory[i] << " ";
    }
    cout << "\n";
    cout << "\n";
    cout << "Registers\n";
    for (auto it = registers.begin(); it !=registers.end(); it++) {
        cout << it->first << " " << it->second.first << " " << it->second.second << "\n";
    }
    cout << "\n";
    cout << "Variables\n";
    for (auto it = variables.cbegin(); it !=variables.cend(); it++) {
        cout << it->first << " " << it->second.first << " " << it->second.second <<  "\n";
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
    if (inFile.is_open()) {
        string line;
        while (getline(inFile, line)) {
            size_t prev = 0, pos;
            while ((pos = line.find_first_of(" ,", prev)) != string::npos) {
                if (pos > prev)
                    tokens.push_back(line.substr(prev, pos - prev));
                prev = pos + 1;
            }
            if (prev < line.length())
                tokens.push_back(line.substr(prev, string::npos));
        }
    }
    // loads the program into memory
    int curPos = 0;
    for (int i = 0; i < tokens.size(); i++) {
        string curToken = tokens[i];
        // if the token is an instruction, encode it to memory using 6 bytes, but first check whether it is syntatically true or not
        if (instructions.find(curToken) != instructions.end()) { ;
            for (int j = 0; j < 6; j++) {
                memory[curPos] = i;
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
            if (decimal(varValue, value)) {
                if (curToken == "DB") {
                    if (value > 255) {
                        cout << "Overflow";
                        return false;
                    } else {
                        memory[curPos++] = value;
                    }
                }
                if (curToken == "DW") {
                    if (value > 65535) {
                        cout << "Overflow";
                        return false;
                    } else {
                        memory[curPos++] = value & 0xff;
                        memory[curPos++] = (value >> 8) & 0xff;
                    }
                }
            }
            else {
                value = varValue.at(1);
                if (curToken == "DB") {
                    memory[curPos++] = value;
                }
                else if (curToken == "DW") {
                    if (value > 65535) {
                        return false;
                    }
                    else {
                        memory[curPos++] = value & 0xff;
                        memory[curPos++] = (value >> 8) & 0xff;
                    }
                }
            }
        }
    }
    return true;
}


bool decimal(string st, int& a) {
    if (st.front() < '0' || st.front() > '9') return false;
    if(st.back()=='h') {
        a= stoi(st, nullptr, 16);
        return true;
    }
    if(st.back()=='b') {
        a= stoi(st, nullptr, 2);

        return true;
    }
    if(st.back()=='d'|| (('0'<=st.back())&&(st.back()<='9'))) {

        a=stoi(st);
        return true;
    }

    cout<<"The number specification is invalid!";
    return false;
}

int fetchValue(string varName) {
    int address = variables[varName].first;
    if (variables[varName].second == "DB") {
        return memory[address];
    }
    else {
        int value = memory[address];
        value += memory[address+1]<<8;
        return value;
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
        registers["AX"].first = registers["AL"].first + registers["AH"].first<<8;
        registers["BX"].first = registers["AL"].first + registers["AH"].first<<8;
        registers["CX"].first = registers["AL"].first + registers["AH"].first<<8;
        registers["DX"].first = registers["AL"].first + registers["AH"].first<<8;
    }
}

bool mov(int op1, int op2) {
    string destination = tokens[op1];
    string source = tokens[op2];
    // cases where destination is a register
    if (registers.find(destination) != registers.end()) {
        // case where source is an immediate value
        int value;
        if (decimal(source, value)) {
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
            value = variables[tokens[op2+1]].first;
            if (registers[destination].second < value) {
                cout << "Overflow";
                return false;
            }
            else {
                registers[destination].first = value;
            }
        }
        // case where source is the contents of another register
        else if (registers.find(source) != registers.end()) {
            if (registers[destination].second < registers[source].second) {
                cout << "Overflow";
                return false;
            }
            else {
                registers[destination].first = registers[source].first;
            }
        }
        // case where the source is the contents of a memory offset
        else if (source.at(1) == '[' && source.back() == ']') {
            char type = source.front();
            if (type == 'b' || type == 'B') {
                string val = source.substr(2, source.length()-3);
                int value;
                if (decimal(val, value)) {
                    registers[destination].first = memory[value];
                }
                else {
                    cout << "Incorrect memory address";
                    return false;
                }
            }
            if (type == 'w' || type == 'W') {
                string val = source.substr(2, source.length()-3);
                int value;
                if (decimal(val, value)) {
                    if (registers[destination].second == 255) {
                        cout << "Overflow";
                        return false;
                    }
                    else {
                        registers[destination].first = memory[value];
                    }
                }
                else {
                    cout << "Incorrect memory address";
                    return false;
                }
            }
        }
        // case where the source is the contents of a memory offset pointed by a register
        else if (source.length() == 3 && source.at(0) == '[' && source.at(3) == ']') {
            string registerName = source.substr(1,2);
            if (registers.find(registerName) != registers.end()) {
                registers[destination].first = memory[registers[registerName].first];
            }
            else {
                cout << "Incorrect register name";
                return false;
            }
        }
        else {
            cout << "Incorrect operand for MOV operation";
            return false;
        }
        updateRegisters(destination);
    }
    // cases where destination is a memory location
    if (destination.at(1) == '[' && destination.back() == ']') {
        string val = destination.substr(2, destination.length()-3);
        int value;
        if (!decimal(val, value)) {
            cout << "Incorrect memory address";
            return false;
        }
        // if source is an immediate value
        int constant;
        if (decimal(source, constant)) {
            if (destination.front() == 'b' || destination.front() == 'B' ) {
                if (constant > 255) {
                    cout << "Overflow";
                    return false;
                }
                else {
                    memory[value] = constant;
                }
            }
            if (destination.front() == 'w' || destination.front() == 'W') {
                if (constant > 65535) {
                    cout << "Overflow";
                    return false;
                }
                else {
                    memory[value] = constant & 0xff;
                    memory[value+1] = (constant >> 8) & 0xff;
                }
            }
        }
        else if (source.length() == 3 && source.front() == 39 && source.back() == 39) {
            constant = source.at(1);
            if (destination.front() == 'b' || destination.front() == 'B' ) {
                memory[value] = constant;
            }
            if (destination.front() == 'w' || destination.front() == 'W') {
                    memory[value] = constant & 0xff;
                    memory[value+1] = (constant >> 8) & 0xff;
            }
        }
        // if source is the contents of a register
        else if (registers.find(source) != registers.end()) {
            constant = registers[source].first;
            if (destination.front() == 'b' || destination.front() == 'B' ) {
                if (constant > 255) {
                    cout << "Overflow";
                    return false;
                }
                else {
                    memory[value] = constant;
                }
            }
            if (destination.front() == 'w' || destination.front() == 'W') {
                if (constant > 65535) {
                    cout << "Overflow";
                    return false;
                }
                else {
                    memory[value] = constant & 0xff;
                    memory[value+1] = (constant >> 8) & 0xff;
                }
            }
        }
        else {
            cout << "Incorrect operand for move operation";
            return false;
        }
    } 
    return true;
}

/*
bool checkSyntax(string& instruction, int i) { //offsets are not considered here, need to improve on those
    if (instruction == "NOP") return true;

    //I assumed, tokens are not "," or sth similar. Controls the tokens with 2 operands
    if (instruction == "MOV" || instruction == "ADD" || instruction == "AND" || instruction == "CMP" ||
        instruction == "XOR" || instruction == "SHL" ||
        instruction == "SHR" || instruction == "RCL" || instruction == "RCR" || instruction == "OR" || instruction =="SUB") {
        int k = i + 1;
        int l = i + 2;
        if (l >= tokens.size()) {
            cout << "Invalid operand to " << instruction << endl;
            return false;
        }
        string op1 = tokens[k];
        string op2 = tokens[l];

        // For all instructions except CMP, first operand cannot be in the form offset var1, hence we add that condition as well.
        if ((instructions.find(op1) != instructions.end()) || instructions.find(op2) != instructions.end() ||
            op1[op1.length() - 1] == ':' || op2[op2.length() - 1] == ':'||op1=="DB"||op2=="DW"||(instruction!="CMP"&&op1=="OFFSET" )) {
            cout << "Invalid operand to " << instruction << endl;
            return false;
        }
        // if instruction is cmp, we need to check that condition and increment i by 1, since "offset" takes place as well in tokens
        if(op1=="OFFSET"&&instruction=="CMP") {
            op1=op2;
            l++;
            if (l >= tokens.size()) {
                cout << "Invalid operand to " << instruction << endl;
                return false;
            }
            op2=tokens[l];
            if ((instructions.find(op1) != instructions.end()) || instructions.find(op2) != instructions.end() ||
                op1[op1.length() - 1] == ':' || op2[op2.length() - 1] == ':'||op1=="DB"||op2=="DW"||(instruction!="CMP"&&op1=="OFFSET" )) {
                cout << "Invalid operand to " << instruction << endl;
                return false;
            }
            i++;
        }
        //check the case when op2==offset. It is possible for all instructions with 2 operands since the second operand may be immediate value, that is, the value returned by offset
        if(op2=="OFFSET") {
           l++;
            if (l >= tokens.size()) {
                cout << "Invalid operand to " << instruction << endl;
                return false;
            }
            op2=tokens[l];
            if (instructions.find(op2) != instructions.end() ||op2.back() == ':'||op2=="DW") {
                cout << "Invalid operand to " << instruction << endl;
                return false;
            }
            i++;
        }


        i += 2;
    }
    //controls INT specifically
  else  if (instruction == "INT") {
        if (((i + 1) >= tokens.size()) || (tokens[i + 1] != "20h" && tokens[i + 1] != "21h")) {
            cout << "Invalid operand to " << instruction << endl;
            return false;
        }
        i++;
    }
    //controls instructions with one operand
 else  if(instruction=="PUSH"||instruction=="POP"||instruction=="NOT"||instruction=="MUL"||instruction=="DIV"||instruction=="JZ"||
    instruction=="JNZ"||instruction=="JE"||instruction=="JNE"||instruction=="JA"||instruction=="JAE"||instruction=="JB"||instruction=="JBE"||
    instruction=="JNAE"||instruction=="JNB"||instruction=="JNBE"||instruction=="JNC") {

        if((i+1)>=tokens.size()||instructions.find(tokens[i+1])!=instructions.end()|| tokens[i+1].back()==':'||tokens[i+1]=="DB"||tokens[i+1]=="DW" ) {
            cout << "Invalid operand to " << instruction << endl;
            return false;
        }
        // controls the case when -instruction- -offset- -name-
        if((instruction=="PUSH"||instruction=="POP"||instruction=="NOT"||instruction=="MUL"||instruction=="DIV")&&tokens[i+1]=="OFFSET") {
            if((i+2)>=tokens.size()||instructions.find(tokens[i+2])!=instructions.end()|| tokens[i+2].back()==':'||tokens[i+2]=="DB"||tokens[i+2]=="DW") {
                cout << "Invalid operand to " << instruction << endl;
                return false;
            }
            i++;
        }
        i++;
    }
// What have we done here? We simply controlled the basic rules regarding the syntax of the assembly code. Any other rule needing the preprocessed code should be controlled in
// runtime
    return true;
}







*/