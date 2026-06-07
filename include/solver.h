#pragma once
#include "pattern.h"
#include <bits/stdc++.h>
using namespace std;

class Solver {
    public:
        Solver(string guessSetFile, string candidateSetFile,
               string candidateSetIdxFile, string patMatrixFile);
        string bestGuess();
        void filterWords(pattern &p);
        void reset();
        double entropyScore(int guess);

        vector<int> candidateSet;
        int prev_guess;

    private:
};
