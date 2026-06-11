#include "pattern.h"
#include <cstdint>
#include <stdexcept>

vector<string> PatternEngine::guessSet;
vector<string> PatternEngine::candidateSetWords;
vector<uint8_t> PatternEngine::patternMatrix;

void PatternEngine::init() {
    guessSet.resize(GUESS_SET_SIZE);
    candidateSetWords.resize(CANDIDATE_SET_SIZE);
    patternMatrix.resize(GUESS_SET_SIZE * CANDIDATE_SET_SIZE);
}

uint8_t computePattern(const std::string &guess, const std::string &ans) {
    int8_t p[WORD_LEN]{};
    int8_t freq[26]{};

    for (int i = 0; i < WORD_LEN; i++) {
        freq[ans[i] - 'a']++;
        if (guess[i] == ans[i]) {
            p[i] = GREEN;
            freq[ans[i] - 'a']--;
        }
    }
    // Mark yellows
    uint8_t code = 0;
    for (int i = 0; i < WORD_LEN; i++) {
        if (p[i] != GREEN) {
            int c = guess[i] - 'a';
            p[i] = (freq[c] > 0) ? (freq[c]--, YELLOW) : GREY;
        }
    }
    // Encode base-3 inline
    for (int i = 0; i < WORD_LEN; i++)
        code = code * 3 + p[i];
    return code;
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

uint8_t PatternEngine::pm(int guess, int cand) {
    return patternMatrix[guess * CANDIDATE_SET_SIZE + cand];
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
    ifstream file(fname, ios::binary);
    if (!file)
        throw std::runtime_error("Cannot open " + fname);
    file.read(reinterpret_cast<char *>(patternMatrix.data()),
              GUESS_SET_SIZE * CANDIDATE_SET_SIZE);
}

void PatternEngine::precomputeMatrix(string fname) {
    buildMatrix();
    ofstream file(fname, ios::binary);
    if (!file)
        throw std::runtime_error("Cannot open " + fname);
    file.write(reinterpret_cast<const char *>(patternMatrix.data()),
               GUESS_SET_SIZE * CANDIDATE_SET_SIZE);
}

void PatternEngine::buildMatrix() {
    for (int i = 0; i < GUESS_SET_SIZE; i++)
        for (int j = 0; j < CANDIDATE_SET_SIZE; j++)
            patternMatrix[i * CANDIDATE_SET_SIZE + j] =
                computePattern(guessSet[i], candidateSetWords[j]);
}
