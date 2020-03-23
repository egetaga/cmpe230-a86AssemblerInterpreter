//
// Created by mafevke on 20.03.2020.
//
/*#include <vector>
#include <list>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
using namespace std;

extern vector<string> tokens;
extern vector<int> memory;
extern unordered_map<string, int> labels; // maps from label to memory address
extern unordered_map<string, pair<int, string>> variables; // maps from variable name to a pair of it's memory address and string value
extern unordered_map<string, pair<int&, int>> registers;
extern unordered_set<string> instructions;
extern unordered_set<string> directives;

int fetchValue(string varName);
void updateRegisters(string changedRegister);
string addressDecoder(string token);
bool mov(string destination, string source);
bool decimal(string st, int& a);
bool checkSyntax(string& instruction, int i);
bool initializeTokens(ifstream& inFile);
// returns true if the operation is successful, returns false otherwise, which means there is a runtime error.

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

*/