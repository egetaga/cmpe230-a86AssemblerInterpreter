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
extern unordered_set<string> generalRegisters;

// returns true if the operation is successful, returns false otherwise, which means there is a runtime error.
bool move(string op1, string op2, int num) {











}
