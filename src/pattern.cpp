#include "pattern.h"

vector<string> PatternEngine::guessSet;
vector<string> PatternEngine::candidateSetWords;
vector<vector<uint8_t>> PatternEngine::patternMatrix;

void PatternEngine::init() {
    guessSet.resize(GUESS_SET_SIZE);
    candidateSetWords.resize(CANDIDATE_SET_SIZE);
    patternMatrix.resize(GUESS_SET_SIZE);
    for (auto &guess : patternMatrix)
        guess.resize(CANDIDATE_SET_SIZE);
}

pattern PatternEngine::getPattern(string &guess, string &ans) {
    pattern p(WORD_LEN, 0);
    unordered_map<char, int> a;
    // holds remaining usable chars
    for (int i = 0; i < WORD_LEN; i++) {
        a[ans[i]]++;
        if (guess[i] == ans[i]) {
            p[i] = 2;
            a[ans[i]]--;
        }
    }
    for (int i = 0; i < WORD_LEN; i++) {
        if (p[i] == 2)
            continue;
        if (a.count(guess[i]) && a[guess[i]] > 0) {
            p[i] = 1;
            a[guess[i]]--;
        } else
            p[i] = 0;
    }
    return p;
}

uint8_t PatternEngine::encodePattern(pattern &p) {
    uint8_t ans = 0;
    for (auto i : p)
        ans = ans * 3 + i;
    return ans;
}

pattern PatternEngine::decodePattern(uint8_t x) {
    pattern ans(WORD_LEN);
    for (int i = WORD_LEN - 1; i >= 0; i--) {
        ans[i] = x % 3;
        x = x / 3;
    }
    return ans;
}

void PatternEngine::loadPatternMatrix(string fname) {
    ifstream file(fname, ios::binary | ios::in);
    for (int i = 0; i < GUESS_SET_SIZE; i++) {
        for (int j = 0; j < CANDIDATE_SET_SIZE; j++) {
            file.read(reinterpret_cast<char *>(&patternMatrix[i][j]),
                      sizeof(uint8_t));
        }
    }
    file.close();
}

void PatternEngine::precomputeMatrix(string fname) {
    PatternEngine::buildMatrix();
    ofstream file(fname, ios::binary | ios::out);
    for (int i = 0; i < GUESS_SET_SIZE; i++) {
        for (int j = 0; j < CANDIDATE_SET_SIZE; j++) {
            file.write(reinterpret_cast<const char *>(&patternMatrix[i][j]),
                       sizeof(uint8_t));
        }
    }
    file.close();
}

void PatternEngine::buildMatrix() {
    for (int i = 0; i < GUESS_SET_SIZE; i++) {
        for (int j = 0; j < CANDIDATE_SET_SIZE; j++) {
            pattern pat =
                PatternEngine::getPattern(guessSet[i], candidateSetWords[j]);
            patternMatrix[i][j] = PatternEngine::encodePattern(pat);
        }
    }
}
