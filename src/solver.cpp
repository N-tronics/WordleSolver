#include "solver.h"
#include "pattern.h"
#include <bits/stdc++.h>
#include <cstdint>
using namespace std;

Solver::Solver(string guessSetFile, string candidateSetFile,
               string candidateSetIdxFile, string patMatrixFile) {
    PatternEngine::init();
    // Load word sets
    ifstream gsfile(guessSetFile);
    string word;
    int i = 0;
    while (getline(gsfile, word))
        PatternEngine::guessSet[i++] = word;

    ifstream csfile(candidateSetFile);
    i = 0;
    while (getline(csfile, word))
        PatternEngine::candidateSetWords[i++] = word;

    ifstream csifile(candidateSetIdxFile);
    i = 0;
    candidateSet.resize(CANDIDATE_SET_SIZE);
    while (getline(csifile, word))
        candidateSet[i++] = stoi(word);

    cout << "Word sets have been loaded" << endl;

    // Load the pattern matrix
    if (filesystem::exists(patMatrixFile)) {
        cout << "Loading existing pattern matrix ... " << flush;
        PatternEngine::loadPatternMatrix(patMatrixFile);
    } else {
        cout << "No existing pattern matrix found. Precomputing ... " << flush;
        PatternEngine::precomputeMatrix(patMatrixFile);
    }
    cout << "done" << endl;
}

void Solver::filterWords(pattern &p) {
    uint8_t target = PatternEngine::encodePattern(p);
    vector<int> new_rem;
    for (auto i : candidateSet)
        if (PatternEngine::patternMatrix[prev_guess][i] == target)
            new_rem.push_back(i);
    candidateSet = new_rem;
}

double Solver::entropyScore(int guess) {
    unordered_map<int, int> buckets;
    for (int i : candidateSet)
        buckets[PatternEngine::patternMatrix[guess][i]]++;

    double total = candidateSet.size();
    double entropy = 0;
    for (auto [b, count] : buckets) {
        double prob = count / total;
        entropy += prob * log2(prob);
    }
    return -entropy;
}

string Solver::bestGuess() {
    double ma = -1e8;
    int maxidx = -1;
    for (int i = 0; i < GUESS_SET_SIZE; i++) {
        double x = entropyScore(i);
        if (x > ma) {
            ma = x;
            maxidx = i;
        }
    }
    prev_guess = maxidx;
    return PatternEngine::guessSet[maxidx];
}

void Solver::reset() {
    vector<int> new_rem;
    for (int i = 0; i < CANDIDATE_SET_SIZE; i++)
        new_rem.push_back(i);
    candidateSet = new_rem;
    prev_guess = -1;
}
