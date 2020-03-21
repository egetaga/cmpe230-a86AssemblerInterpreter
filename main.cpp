#include <vector>
#include <list>
#include <iostream>
#include <fstream>
#include <unordered_map>

using namespace std;

vector<string> tokens;
vector<int> memory(2<<15, -1);
// initializes memory and labels
unordered_map<string, int> labels; // maps from label to memory address
unordered_map<string, pair<int, string>> variables; // maps from variable name to a pair of it's memory address and string value
string instructions = "NOP,NOT,JZ,JNZ,JE,JNE,JA,JAE,JB,JBE,JNAE,JNB,JNBE,JNC,JC,PUSH,POP,INT,MOV,ADD,SUB,MUL,DIV,XOR,OR,AND,RCL,RCR,SHL,SHR";
string directives ="DW,DB";
// need to add lowercase versions
unordered_map<string, pair<int, int>> registers; // maps the name of the register to a pair of it's value and size in bits


// flags
int ZF = 0,AF = 0,CF = 0,SF = 0,OF= 0;

string addressDecoder(string token);
bool decimal(string st, int& a);

int main() {
    // initiate registers and flags
    // 8 bit registers
    registers["AH"] = make_pair(0,8);
    registers["AL"] = make_pair(0,8);
    registers["BH"] = make_pair(0,8);
    registers["BL"] = make_pair(0,8);
    registers["CH"] = make_pair(0,8);
    registers["CL"] = make_pair(0,8);
    registers["DH"] = make_pair(0,8);
    registers["DL"] = make_pair(0,8);
    // 16 bit registers
    registers["AX"] = make_pair(0,16);
    registers["BX"] = make_pair(0,16);
    registers["CX"] = make_pair(0,16);
    registers["DX"] = make_pair(0,16);
    registers["DI"] = make_pair(0,16);
    registers["SP"] = make_pair((int)0xFFFE,16);
    registers["SI"] = make_pair(0,16);
    registers["BP"] = make_pair(0,16);
    // parses the input program and places the tokens into a vector
    ifstream inFile;
    inFile.open("../test.txt");
    string token;
    if (inFile.is_open()) {
        string line;
        while (getline(inFile, line)) {
            size_t prev = 0, pos;
            while ((pos = line.find_first_of(" ,", prev)) != string::npos)
            {
                if (pos > prev)
                    tokens.push_back(line.substr(prev, pos-prev));
                prev = pos+1;
            }
            if (prev < line.length())
                tokens.push_back(line.substr(prev, string::npos));
        }
    }
    // loads the program into memory
    int curPos = 0;
    for (int i=0; i<tokens.size(); i++) {
        string curToken = tokens[i];
        // if the token is an instruction, encode it to memory using 6 bytes
        if (instructions.find(curToken) != string::npos) {
            for (int j=0; j<6; j++) {
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
            // if the token is a directive encode the variable in memory and put it's address and string value to the variable map
        else if (directives.find(curToken) != string::npos) {
            variables[tokens[i - 1]] = make_pair(curPos, tokens[i + 1]);
            if (tokens[i + 1] == "OFFSET") {
                string offset = tokens[i+1] + " " + tokens[i+2];
                variables[tokens[i - 1]] = make_pair(curPos, offset);
            }
            memory[curPos] = i;
            curPos++;
            if (curToken == "DW") {
                memory[curPos] = i;
                curPos++;
            }
        }
    }
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
    for (auto it = variables.begin(); it !=variables.end(); it++) {
        cout << it->first << " " << it->second.first <<" " << it->second.second << "\n";
    }
    cout << "\n";
    cout <<"Labels\n";
    for (auto it = labels.begin(); it !=labels.end(); it++) {
        cout << it->first << " " << it->second << "\n";
    }
    cout << "\nFlags\n";
    cout << "ZF = " << ZF << " AF = " << AF <<" CF = "<< CF <<" SF = " << SF << " OF = " << OF;
 }

string addressDecoder(string token) {
    // register addressing: if operand is the contents of a register return the name of the registers in a string
    if (registers.find(token) != registers.end()) {
        return token;
    }
    // register indirect: if the operand is the contents of the memory address pointed to by a register, return the token corresponding to that memory address
    else if (token.front() == '[' && token.back() == ']' && registers.find(token.substr(1,2)) != registers.end()) {
        return tokens[memory[registers[token.substr(1,2)].first]];
    }
    // memory addressing: if the operand is the content of a memory location return the token corresponding to that memory location
    else if (token.at(1) == '[' && token.back() == ']') {
        int address;
        token.pop_back();
        decimal(token.substr(1), address);
        return tokens[memory[address]];
    }
    // immediate addressing
    else return token;
    // stack addressing will be handled implicitly during execution
}

bool decimal(string st, int& a) {
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

void mov(string destination, string source) {
    // from register to register
    if (registers.find(destination) != registers.end() && registers.find(source) != registers.end()) {
        if (registers[source].second > registers[destination].second) {
            // error message and exit here
        }
        else {
            registers[destination] = registers[source];
        }
    }
}
