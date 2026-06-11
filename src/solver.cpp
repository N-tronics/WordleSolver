#include <bits/stdc++.h>
#include <pattern.h>
#include <solver.h>
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

    loadCSIFile(candidateSetIndicesFile);
    reset();

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

    int new_size = 0;
    for (int i = 0; i < candidateSetSize; i++)
        if (PatternEngine::pm(prev_guess, candidateSet[i].first) == target)
            swap(candidateSet[new_size++], candidateSet[i]);
    candidateSetSize = new_size;
}

double Solver::entropyScore(int guess) {
    int buckets[243]{};
    const uint8_t *row =
        PatternEngine::patternMatrix.data() + guess * CANDIDATE_SET_SIZE;
    for (int i = 0; i < candidateSetSize; i++)
        buckets[row[candidateSet[i].first]]++;

    double total = candidateSetSize;
    double entropy = 0;
    for (int b = 0; b < 243; b++) {
        if (!buckets[b])
            continue;
        double prob = buckets[b] / total;
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
    int n = candidateSetSize;
    if (n <= 2)
        return candidateSet[0].second;

    static bool isCand[GUESS_SET_SIZE];
    std::memset(isCand, 0, sizeof(bool) * GUESS_SET_SIZE);
    for (int i = 0; i < n; i++)
        isCand[candidateSet[i].second] = true;

    double ma = -1e18;
    int maxidx = -1;
    for (int i = 0; i < GUESS_SET_SIZE; i++) {
        double entropy = entropyScore(i) + (isCand[i] ? 0.001 : 0.0);
        if (entropy > ma) {
            ma = entropy;
            maxidx = i;
        }
    }
    return maxidx;
}

void Solver::reset() {
    for (int i = 0; i < CANDIDATE_SET_SIZE; i++) {
        candidateSet[i].first = i;
        candidateSet[i].second = csiData[i];
    }
    candidateSetSize = CANDIDATE_SET_SIZE;
    prev_guess = -1;
}

void Solver::loadCSIFile(string candidateSetIndicesFile) {
    ifstream csifile(candidateSetIndicesFile, ios::binary);
    if (!csifile)
        throw std::runtime_error("Could not open " + candidateSetIndicesFile);

    candidateSet.resize(CANDIDATE_SET_SIZE);
    csiData.resize(CANDIDATE_SET_SIZE);
    csifile.read(reinterpret_cast<char *>(csiData.data()),
                 CANDIDATE_SET_SIZE * sizeof(uint16_t));

    for (int i = 0; i < CANDIDATE_SET_SIZE; i++) {
        candidateSet[i].first = i;
        candidateSet[i].second = csiData[i];
    }
}
