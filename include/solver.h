#pragma once
#include "pattern.h"
#include <bits/stdc++.h>
using namespace std;

class Solver {
    public:
        Solver(string guessSetFile, string candidateSetFile,
               string candidateSetIndicesFile, string patMatrixFile);
        string bestGuess();
        void filterWords(pattern &p);
        void reset();
        double entropyScore(int guess);

        vector<pair<int, int>> candidateSet;
        int prev_guess;

    private:
        int bestGuessIdx();
};
