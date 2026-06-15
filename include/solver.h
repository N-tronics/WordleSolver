#pragma once
#include <bits/stdc++.h>
#include <pattern.h>
#include <unordered_map>
using namespace std;
using ProgressFn = function<void(int, int)>;

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

        double entropyScore(int guess, span<const pair<int, int>> candSet);
        void filterWords(const int guess, pattern &p);
        void reset();

        string bestGuessEntropy();
        int bestGuessIdxEntropy();

        void sortByEntropy(vector<int> &guessOrder,
                           span<const pair<int, int>> candidates,
                           bool fullSearch);
        int minimax(span<const pair<int, int>> candidates, int alpha,
                    int *bestGuessOut = nullptr,
                    const ProgressFn &onProgress = nullptr);
        void precomputeMinimax();
        int bestGuessIdxMinimax();
        string bestGuessMinimax();

        vector<pair<int, int>> candidateSet;
        int candidateSetSize;

    private:
        void loadCSIFile(string candidateSetIndicesFile);
        vector<uint16_t> csiData;

        void saveMMCache(const string &fname) const;
        void loadMMCache(const string &fname);

        static constexpr int CANDIDATE_ONLY_THRESHOLD = 15;
        static constexpr int MINIMAX_MAX_DEPTH = 30;
        static constexpr int ENTROPY_SORT_CAP = 150;
        unordered_map<vector<uint16_t>, pair<int, int>, VectorHash> mmCache;
};
