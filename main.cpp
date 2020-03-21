#include <vector>
#include <list>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
using namespace std;

int ax=0, bx=0, cx=0, dx=0, al=0, ah=0, bl=0, bh=0, cl=0, ch=0, dl=0, dh=0, di=0, sp=0xfffe, si=0, bp=0; //registers

int zf=0, cf=0, af=0, sf=0, df=0; //flags

vector<string> tokens;

vector<int> memory(2<<15, -1);
// initializes memory and labels
unordered_map<string, int> labels; // maps from label to memory address
unordered_map<string, pair<int, string>> variables; // maps from variable name to a pair of it's memory address and string value
unordered_map<string, int&> generalRegisters;

int main() {

    unordered_map<string, int&> registers= { {"ax",ax} , {"bx", bx}, {"cx", cx}, {"dx",dx}, {"al",al},{"ah",ah}, {"bl",bl},{"bh",bh},{"cl",cl},{"ch",ch},{"dl",dl},{"dh",dl}}; //contains ax, bx... al, ah...
    unordered_map<string, int&> generalRegisters = registers; // bad style

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

    string instructions = "NOP,NOT,JZ,JNZ,JE,JNE,JA,JAE,JB,JBE,JNAE,JNB,JNBE,JNC,JC,PUSH,POP,INT,MOV,ADD,SUB,MUL,DIV,XOR,OR,AND,RCL,RCR,SHL,SHR";
    string directives ="DW,DB";
    // need to add lowercase versions
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
    for (int i=0; i<memory.size(); i++) {
        cout << memory[i] << " ";
        if(memory[i] == -1) break;
    }









}