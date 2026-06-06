#pragma once
#include<bits/stdc++.h>
#include "pattern.h"
using namespace std;

class solver{
    public:
    solver(string);
    string bestGuess();
    void filterWords(pattern& p);
    void reset(); 

    private:
    vector<vector<int>> matrix;
    vector<string> all;
    vector<int> remaining;
    int prev_guess;
    void buildMatrix();
    double entropyScore(int x);
};