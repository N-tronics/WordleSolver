#pragma once
#include <bits/stdc++.h>
#include <pattern.h>
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
        int candidateSetSize;
        int prev_guess;
        int bestGuessIdx();

    private:
        void loadCSIFile(string candidateSetIndicesFile);
        vector<uint16_t> csiData;
};
