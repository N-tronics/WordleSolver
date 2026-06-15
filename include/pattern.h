#pragma once
#include <bits/stdc++.h>
using namespace std;

constexpr int WORD_LEN = 5;
constexpr int GREEN = 2, YELLOW = 1, GREY = 0;
constexpr int GUESS_SET_SIZE = 12972;
constexpr int CANDIDATE_SET_SIZE = 2315;
constexpr uint8_t CORRECT_GUESS = 242;

using pattern = vector<int>;

class PatternEngine {
    public:
        static vector<string> guessSet, candidateSetWords;
        static vector<uint8_t> patternMatrix;

        static void init();
        static pattern getPattern(string &guess, string &ans);
        static uint8_t computePattern(const std::string &guess,
                                      const std::string &ans);
        static uint8_t encodePattern(pattern &p);
        static uint8_t pm(int guess, int cand);
        static pattern decodePattern(uint8_t x);

        static void buildMatrix();
        static void loadPatternMatrix(string fname);
        static void precomputeMatrix(string fname);

        static int getWordGuessIdx(string word);
};
