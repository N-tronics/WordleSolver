#pragma once
#include <bits/stdc++.h>
#include <pattern.h>
#include <unordered_map>
using namespace std;
using ProgressFn = function<void(int, int)>;

// Used to hash a vector
// Evaluated states are stored in mmCache with the key as the
// has of the candidate set.
struct VectorHash {
        size_t operator()(const vector<uint16_t> &v) const noexcept {
            size_t seed = v.size();
            for (uint16_t x : v)
                seed ^= (size_t)x + 0x9e3779b9u + (seed << 6) + (seed >> 2);
            return seed;
        }
};

class Solver {
    public:
        Solver(string guessSetFile, string candidateSetFile,
               string candidateSetIndicesFile, string patMatrixFile,
               string mmCacheFile);

        // Evaulates the entropy of a given guess against a given candidate set
        // p = probability of a particular pattern
        // E = -sum(plog(p))
        double entropyScore(int guess, span<const pair<int, int>> candSet);
        // Filters the current candidateSet according to a given guess and
        // a pattern
        void filterWords(const int guess, pattern &p);
        // Resets the candidate set to the original 2315 words
        void reset();

        // =====================================================================
        // Entropy Solver

        // Wrapper around bestGuessIdxEntropy to return the word instead of the
        // idx
        string bestGuessEntropy();
        // Returns the guess idx of the best guess according to entropy.
        int bestGuessIdxEntropy();

        // =====================================================================
        // Minimax Solver

        // Sorts the best (ENTROPY_SORT_CAP) words according to their entropy
        // scores
        void sortByEntropy(vector<int> &guessOrder,
                           span<const pair<int, int>> candidates,
                           bool fullSearch);
        // Recursive minimax function to find the minimum cost
        int minimax(span<const pair<int, int>> candidates, int alpha,
                    int *bestGuessOut = nullptr,
                    const ProgressFn &onProgress = nullptr);
        // Populates mmCache to ensuret the actual search during gameplay is
        // fast
        void precomputeMinimax();
        // Returns the guess idx of the best word according to the minimax
        // solver
        int bestGuessIdxMinimax();
        // Wrapper around the bestGuessIdxMinimax to return the word instead of
        // the idx
        string bestGuessMinimax();

        // =====================================================================

        // Current candidate set. It contains all possible candidates,
        // But the first candidateSetSize elements are the current valid
        // canidates.
        vector<pair<int, int>> candidateSet;
        int candidateSetSize;

    private:
        // loads the candidateSetIndicesFile
        void loadCSIFile(string candidateSetIndicesFile);
        // indices for the initial 2315 candidates
        vector<uint16_t> csiData;

        // saves and loads precomputed mmCache values
        void saveMMCache(const string &fname) const;
        void loadMMCache(const string &fname);

        // Minimax search control variables
        // If the current candidate set size is below this threshold, the search
        // is only going to evaluate the remaining candidates. Above this
        // threshold, the search evaulates the best ENTROPY_SORT_CAP elements
        static constexpr int CANDIDATE_ONLY_THRESHOLD = 15;
        // Maximum depth for the minimax search
        static constexpr int MINIMAX_MAX_DEPTH = 30;
        // If the candidate set size is above CANDIDATE_ONLY_THRESHOLD, the
        // entropy sort only sorts the best ENTROPY_SORT_CAP guesses based on
        // their entropy scores
        static constexpr int ENTROPY_SORT_CAP = 300;
        // The minimax cache that stores the results of previously computed
        // states
        unordered_map<vector<uint16_t>, pair<int, int>, VectorHash> mmCache;
};
