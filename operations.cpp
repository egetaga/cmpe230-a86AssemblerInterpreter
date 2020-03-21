//
// Created by mafevke on 20.03.2020.
//
#include <vector>
#include <list>
#include <iostream>
#include <fstream>
#include <unordered_map>
using namespace std;

extern vector<string> tokens;
extern vector<int> memory;
extern unordered_map<string, int> labels; // maps from label to memory address
extern unordered_map<string, pair<int, string>> variables; // maps from variable name to a pair of it's memory address and string value
extern unordered_map<string, int&> generalRegisters;

// returns true if the operation is successful, returns false otherwise, which means there is a runtime error.

bool decimal1(string st, int& a) {
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