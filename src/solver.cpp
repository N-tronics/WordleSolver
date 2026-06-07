#include "solver.h"
#include "pattern.h"
#include <bits/stdc++.h>
#include <cstdint>
#include <filesystem>
#include <stdexcept>
#include <utility>
using namespace std;

Solver::Solver(string guessSetFile, string candidateSetFile,
               string candidateSetIndicesFile, string patMatrixFile) {
    if (!filesystem::exists(guessSetFile))
        throw runtime_error("File '" + guessSetFile + "' does not exist");
    if (!filesystem::exists(candidateSetFile))
        throw runtime_error("File '" + candidateSetFile + "' does not exist");
    if (!filesystem::exists(candidateSetIndicesFile))
        throw runtime_error("File '" + candidateSetIndicesFile +
                            "' does not exist");

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

    candidateSet.resize(CANDIDATE_SET_SIZE);
    ifstream csifile(candidateSetIndicesFile);
    i = 0;
    while (getline(csifile, word))
        candidateSet[i++] = make_pair(i, stoi(word));

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

    prev_guess = -1;
}

void Solver::filterWords(pattern &p) {
    if (prev_guess == -1)
        return;
    uint8_t target = PatternEngine::encodePattern(p);
    vector<pair<int, int>> new_rem;
    for (auto i : candidateSet)
        if (PatternEngine::patternMatrix[prev_guess][i.first] == target)
            new_rem.push_back(i);
    candidateSet = new_rem;
}

double Solver::entropyScore(int guess) {
    unordered_map<int, int> buckets;
    for (auto i : candidateSet)
        buckets[PatternEngine::patternMatrix[guess][i.first]]++;

    double total = candidateSet.size();
    double entropy = 0;
    for (auto [b, count] : buckets) {
        double prob = count / total;
        entropy += prob * log2(prob);
    }
    return -entropy;
}

string Solver::bestGuess() {
    prev_guess = bestGuessIdx();
    if (prev_guess == -1)
        throw runtime_error("Cannot compute best guess");
    return PatternEngine::guessSet[prev_guess];
}

int Solver::bestGuessIdx() {
    if (candidateSet.size() == 1)
        return candidateSet[0].second;

    double ma = -1e8;
    int maxidx = -1;
    for (int i = 0; i < GUESS_SET_SIZE; i++) {
        double x = entropyScore(i);
        if (x > ma) {
            ma = x;
            maxidx = i;
        }
    }
    return maxidx;
}

void Solver::reset() {
    // TODO: FIX
    vector<int> new_rem;
    for (int i = 0; i < CANDIDATE_SET_SIZE; i++)
        new_rem.push_back(i);
    // candidateSet = new_rem;
    prev_guess = -1;
}
