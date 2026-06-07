#pragma once
#include <bits/stdc++.h>
using namespace std;

constexpr int WORD_LEN = 5;
constexpr int GREEN = 2, YELLOW = 1, GREY = 0;
constexpr int GUESS_SET_SIZE = 12972;
constexpr int CANDIDATE_SET_SIZE = 2315;

using pattern = vector<int>;

class PatternEngine {
    public:
        static vector<string> guessSet, candidateSetWords;
        static vector<vector<uint8_t>> patternMatrix;

        static void init();
        static pattern getPattern(string &guess, string &ans);
        static uint8_t encodePattern(pattern &p);
        static pattern decodePattern(uint8_t x);

        static void buildMatrix();
        static void loadPatternMatrix(string fname);
        static void precomputeMatrix(string fname);
};
