//
// Created by Tolga on 20.03.2020.
//

#include <vector>
#include <list>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

int main() {
    ifstream testFile;
    testFile.open("../test.txt");
    vector<vector<string>> statements;
    if (testFile.is_open()) {
        string line;
        while (getline(testFile, line)) {
            vector<string> curStatement;
            size_t prev = 0, pos;
            while ((pos = line.find_first_of(" :,;", prev)) != string::npos)
            {
                if (pos > prev)
                    curStatement.push_back(line.substr(prev, pos-prev));
                prev = pos+1;
            }
            if (prev < line.length())
                curStatement.push_back(line.substr(prev, string::npos));
            statements.push_back(curStatement);
        }
    }
}