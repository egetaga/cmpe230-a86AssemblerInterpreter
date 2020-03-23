//
// Created by mafevke on 20.03.2020.
//
#include <vector>
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

bool mov(string destination, string source) {
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
            registers[destination].first = (int)source.at(1);
        }
        // case where source is the contents of another registers
        else if (registers.find(source) != registers.end()) {
            if (registers[destination].second < registers[source].second) {
                cout << "Overflow";
                return false;
            }
            else {
                registers[destination].first = registers[source].first;
            }
        }
        // case where the source is the contents of a memory offset [asda]
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
                    int content = fetchValue(val);
                    if (registers[destination].second == 255) {
                        cout << "Overflow";
                        return false;
                    }
                    else {
                        registers[destination].first = content;
                    }
                }
                else {
                    cout << "Incorrect memory address";
                    return false;
                }
            }
        }
        // case where the source is the contents of a memory offset pointed by a register
        else if (source.at(0) == '[' && source.at(3) == ']') {
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
    if (destination.front() == '[' && destination.back() == ']') {
        string val = destination.substr(1, destination.length()-2);
        int value;
        if (!decimal(val, value)) {
            cout << "Incorrect memory address";
            return false;
        }
        // if source is an immediate value
        int constant;
        if (decimal(source, constant)) {
            if (255 < constant) {
                cout << "Overflow";
                return false;
            }
            else {
                memory[value] = constant;
            }
        }
        else if (source.length() == 3 && source.front() == 39 && source.back() == 39) {
            memory[value] = (int)source.at(1);
        }
        // if source is the contents of a register
        else if (registers.find(source) != registers.end()) {
            if (255 < registers[source].second) {
                cout << "Overflow";
            }
            else {
                memory[value] = registers[source].first;
            }
        }
        else {
            cout << "Incorrect operand for move operation";
            return false;
        }
    }
    return true;
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