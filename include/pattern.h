#pragma once
#include <bits/stdc++.h>
using namespace std;

// Game constants
constexpr int WORD_LEN = 5;
constexpr int GREEN = 2, YELLOW = 1, GREY = 0;
constexpr int GUESS_SET_SIZE = 12972;
constexpr int CANDIDATE_SET_SIZE = 2315;
constexpr uint8_t CORRECT_GUESS = 242;

using pattern = vector<int>;

class PatternEngine {
    public:
        // guessSet contains the 12972 possible guesses
        // candidateSetWords contains the 2315 possible answers
        static vector<string> guessSet, candidateSetWords;
        // GUESS_SET_SIZE * CANDIDATE_SET_SIZE flattened matrix of patterns
        // patternMatrix[i * CANDIDATE_SET_SIZE + j] =
        //     computePattern(guessSet[i], candidateSetWords[j])
        // Flattened matrix allows to have faster reads and writes to the file
        static vector<uint8_t> patternMatrix;

        // Initializes the vectors
        static void init();
        // Computes the pattern for a guess and an answer
        static uint8_t computePattern(const std::string &guess,
                                      const std::string &ans);
        // Converts between vector and uint8_t representations of a pattern
        static uint8_t encodePattern(pattern &p);
        static pattern decodePattern(uint8_t x);
        // Indexes the patternMatrix
        static uint8_t pm(int guess, int cand);
        // Pattern matrix utility functions
        static void buildMatrix();
        static void loadPatternMatrix(string fname);
        static void precomputeMatrix(string fname);

        // Returns the guess idx of a given word
        static int getWordGuessIdx(string word);
};
