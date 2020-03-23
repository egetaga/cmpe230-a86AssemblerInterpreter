#include <vector>
#include <list>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
using namespace std;

vector<string> tokens;
vector<int> memory(2<<15, -1);
// initializes memory and labels

unordered_map<string, int> labels; // maps from label to memory address
unordered_map<string, pair<int, string>> variables; // maps from variable name to it's address and data type
unordered_map<string, pair<int&, int>> registers; // maps the name of the register to a pair of it's value and size in int

int ax=0, bx=0, cx=0, dx=0, al=0, ah=0, bl=0, bh=0, cl=0, ch=0, dl=0, dh=0, di=0, sp=0xfffe, si=0, bp=0; //registers
// flags
int ZF = 0,AF = 0,CF = 0,SF = 0,OF= 0;

unordered_set<string> instructions = {"NOP","NOT","JZ","JNZ","JE","JNE","JA","JAE","JB","JBE","JNAE","JNB","JNBE","JNC","JC","PUSH","POP","INT","MOV","ADD","SUB","MUL"
                                ,"DIV","XOR","OR","AND","RCL","RCR","SHL","SHR"};
unordered_set<string> directives ={"DW","DB"};

// need to add lowercase versions
int fetchValue(string varName);
string addressDecoder(string token);
bool mov(string destination, string source);
bool decimal(string st, int& a);
bool checkSyntax(string& instruction, int i);
bool initializeTokens(ifstream& inFile);
int main() {
    // initiate registers and flags
    // 8 bit registers
    registers["AH"] = make_pair(ah, 255);
    registers["AL"] = make_pair(al, 255);
    registers["BH"] = make_pair(bh, 255);
    registers["BL"] = make_pair(bl, 255);
    registers["CH"] = make_pair(ch, 255);
    registers["CL"] = make_pair(cl, 255);
    registers["DH"] = make_pair(dh, 255);
    registers["DL"] = make_pair(dl, 255);
    // 16 bit registers
    registers["AX"] = make_pair(ax, 65535);
    registers["BX"] = make_pair(bx, 65535);
    registers["CX"] = make_pair(cx, 65535);
    registers["DX"] = make_pair(dx, 65535);
    registers["DI"] = make_pair(di, 65535);
    registers["SP"] = make_pair(sp, 65535);
    registers["SI"] = make_pair(si, 65535);
    registers["BP"] = make_pair(bp, 65535);
    // parses the input program and places the tokens into a vector
    ifstream inFile;
    inFile.open("../test.txt");
    initializeTokens(inFile);
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
    cout << "ZF = " << ZF << " AF = " << AF <<" CF = "<< CF <<" SF = " << SF << " OF = " << OF;
    cout << fetchValue("MYVAR");
}




bool initializeTokens(ifstream& inFile) {
    string token;
    if (inFile.is_open()) {
        string line;
        while (getline(inFile, line)) {
            size_t prev = 0, pos;
            while ((pos = line.find_first_of(" ,", prev)) != string::npos) {
                if (pos > prev)
                    tokens.push_back(line.substr(prev, pos - prev));
                    tokensProcessed.push_back(false);
                prev = pos + 1;
            }
            if (prev < line.length())
                tokens.push_back(line.substr(prev, string::npos));
                tokensProcessed.push_back(false);
        }
    }
    // loads the program into memory
    int curPos = 0;
    for (int i = 0; i < tokens.size(); i++) {
        string curToken = tokens[i];
        // if the token is an instruction, encode it to memory using 6 bytes, but first check whether it is syntatically true or not
        if (instructions.find(curToken) != instructions.end()) {
            if(!checkSyntax(curToken,i)) return false;
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
        else if (directives.find(curToken) != directives.end()) {
            string varName= "NULL";
            if (i>0) varName = tokens[i-1];
            string varValue = tokens[i+1];
            int value; // turn to unsigned int
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
                    }
                    else {
                        memory[curPos++] = value & 0xff;
                        memory[curPos++] = (value >> 8) & 0xff;
                    }
                }
            }
            else {
                value = (int) curToken.at(1);
                if (curToken == "DB") {
                    if (value > 255) {
                        cout << "Overflow";
                    } else {
                        memory[curPos++] = value;
                    }
                }
                if (curToken == "DW") {
                    if (value > 65535) {
                        cout << "Overflow";
                    }
                    else {
                        memory[curPos++] = value & 0xff;
                        memory[curPos++] = (value >> 8) & 0xff;
                    }
                }
            }
        }
    }
    //need to add variable parts here...


    return true;
}

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
