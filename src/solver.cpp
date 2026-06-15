#include <algorithm>
#include <bits/stdc++.h>
#include <filesystem>
#include <functional>
#include <omp.h>
#include <pattern.h>
#include <solver.h>
#include <stdexcept>
using namespace std;

Solver::Solver(string guessSetFile, string candidateSetFile,
               string candidateSetIndicesFile, string patMatrixFile,
               string mmCacheFile) {
    // Ensures essential files are present
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

    // Load the minimax cache from file, otherwise precompute it
    if (filesystem::exists(mmCacheFile)) {
        cout << "Loading existing minimax cache ... " << flush;
        loadMMCache(mmCacheFile);
    } else {
        cout << "No existing minimax cache found. Precomputing ... \n" << flush;
        precomputeMinimax();
        saveMMCache(mmCacheFile);
    }
    cout << "done" << endl;
}

double Solver::entropyScore(int guess, span<const pair<int, int>> candSet) {
    // This function is called heavily so it must be optimized and not make too
    // many heavy dynamic allocations

    // We create buckets for each pattern 0 ~ 242 to hold the number of elements
    // following each pattern
    int buckets[243]{};
    // Store a pointer to the corresponding row in the patternMatrix to avoid
    // calling PatternEngine::pm() too many times
    const uint8_t *row =
        PatternEngine::patternMatrix.data() + guess * CANDIDATE_SET_SIZE;
    for (auto &c : candSet)
        buckets[row[c.first]]++;

    // entropy computation
    double total = candSet.size();
    if (total == 0)
        return 0.0;
    double entropy = 0;
    for (int b = 0; b < 243; b++) {
        if (!buckets[b])
            continue;
        double prob = buckets[b] / total;
        entropy += prob * log2(prob);
    }
    return -entropy;
}

void Solver::filterWords(const int guess, pattern &p) {
    // Instead of removing words that do not follow the pattern,
    // we shuffle the vector such that the first k elements being the elements
    // that do follow the pattern
    uint8_t target = PatternEngine::encodePattern(p);

    int new_size = 0;
    for (int i = 0; i < candidateSetSize; i++)
        if (PatternEngine::pm(guess, candidateSet[i].first) == target)
            swap(candidateSet[new_size++], candidateSet[i]);
    // Control the actual candidateSet size with this variable
    candidateSetSize = new_size;
}

void Solver::reset() {
    for (int i = 0; i < CANDIDATE_SET_SIZE; i++) {
        candidateSet[i].first = i;
        candidateSet[i].second = csiData[i];
    }
    candidateSetSize = CANDIDATE_SET_SIZE;
}

// ===============================================================================

string Solver::bestGuessEntropy() {
    int guess = bestGuessIdxEntropy();
    if (guess == -1)
        throw runtime_error("Cannot compute best guess");
    return PatternEngine::guessSet[guess];
}

int Solver::bestGuessIdxEntropy() {
    int n = candidateSetSize;
    // If n <= 2, its beneficial to just return one of the candidates instead of
    // searching the entire guessSet for the optimal guess
    if (n <= 2)
        return candidateSet[0].second;

    // We use a static array here as an optimization
    static bool isCand[GUESS_SET_SIZE];
    std::memset(isCand, 0, sizeof(bool) * GUESS_SET_SIZE);
    for (int i = 0; i < n; i++)
        isCand[candidateSet[i].second] = true;

    // We loop through all of the guess set and calculate the entropy scores for
    // each one We then return the guess idx of the word that scored the highest
    double ma = -1e18;
    int maxidx = -1;
    for (int i = 0; i < GUESS_SET_SIZE; i++) {
        // If the current word we are scoring is a candidate, we want to
        // slightly favor it So, we add a small entropy value
        double entropy =
            entropyScore(i, span(candidateSet).first(candidateSetSize)) +
            (isCand[i] ? 0.001 : 0.0);
        if (entropy > ma) {
            ma = entropy;
            maxidx = i;
        }
    }
    return maxidx;
}

// ===============================================================================

void Solver::sortByEntropy(vector<int> &guessOrder,
                           span<const pair<int, int>> candidates,
                           bool fullSearch) {
    // This function is called a riduculous amount of times. So, we need it to
    // be fully optimized

    int n = candidates.size();
    if (n <= CANDIDATE_ONLY_THRESHOLD) {
        // If the candidate set size is below this threshold, we are only going
        // to evaluate these candidates
        vector<pair<double, int>> entropy(n);
        for (int i = 0; i < n; i++) {
            int g = candidates[i].second;
            entropy[i] = {entropyScore(g, candidates), g};
        }
        ranges::sort(entropy, greater<>());
        guessOrder.resize(n);
        for (int i = 0; i < n; i++)
            guessOrder[i] = entropy[i].second;
        return;
    }

    // We score the entire guess set with respect to the current candidates
    vector<pair<double, int>> entropy;
    entropy.resize(GUESS_SET_SIZE);
    // We parallelize the scoring as it is independent
#pragma omp parallel for schedule(static) if (!omp_in_parallel())
    for (int i = 0; i < GUESS_SET_SIZE; i++)
        entropy[i] = {entropyScore(i, candidates), i};

    int cap = GUESS_SET_SIZE;
    if (!fullSearch) {
        // We do not sort the entire entropy values if it is not a fullSearch
        // Instead, we sort only the first ENTROPY_SORT_CAP elements
        cap = min(ENTROPY_SORT_CAP, GUESS_SET_SIZE);
        ranges::nth_element(entropy, entropy.begin() + cap, greater<>());
    }
    ranges::sort(entropy.begin(), entropy.begin() + cap, greater<>());

    guessOrder.resize(cap);
    for (int i = 0; i < cap; i++)
        guessOrder[i] = entropy[i].second;
}

int Solver::minimax(span<const pair<int, int>> candidates, int alpha,
                    int *bestGuessOut, const ProgressFn &onProgress) {
    int n = candidates.size();
    // Base case: this is mathematically optimal to guess one of the candidates
    if (n <= 2) {
        if (bestGuessOut)
            *bestGuessOut = candidates[0].second;
        return n;
    }

    // if alpha is < 2, a better ans cant be achieved. So, we return
    if (alpha < 2)
        return alpha;
    // We want to cap the depth of the search
    alpha = min(alpha, MINIMAX_MAX_DEPTH);

    // We save convert the candidate set into a key and use that as a search
    // into the mmCache This is key is order independent If a result is already
    // cache, we use that
    static thread_local vector<uint16_t> keyBuf;
    keyBuf.resize(n);
    for (int i = 0; i < n; i++)
        keyBuf[i] = (uint16_t)candidates[i].first;
    ranges::sort(keyBuf);

    if (auto it = mmCache.find(keyBuf); it != mmCache.end()) {
        if (bestGuessOut)
            *bestGuessOut = it->second.second;
        return it->second.first;
    }
    vector<uint16_t> key(keyBuf.begin(), keyBuf.end());

    // Order the guesses according to the current candidate set
    vector<int> guessOrder;
    sortByEntropy(guessOrder, candidates, onProgress != nullptr);
    int totalGuesses = guessOrder.size();

    // flatBuckets holds the candidates according to the patten it follows
    vector<pair<int, int>> flatBuckets;
    flatBuckets.resize(n);
    // the current best is alpha
    int best = alpha;
    int bestGuessIdx = -1;

    for (int gi = 0; gi < totalGuesses; gi++) {
        int guessIdx = guessOrder[gi];
        int counts[243] = {}, starts[244] = {}, pos[243];
        // Loop through the candidates and count the number of words in each
        // bucket
        for (auto &c : candidates)
            counts[PatternEngine::pm(guessIdx, c.first)]++;
        // If the guess produces exactly one non-winning bucket contiaining all
        // candidates, then the reduced candidateSet would equal the current
        // set, meaning we cant parition the candidates, meaning zero
        // information
        bool zeroInfo = false;
        for (int p = 0; p < 243; p++)
            if (p != CORRECT_GUESS && counts[p] == n) {
                zeroInfo = true;
                break;
            }

        if (!zeroInfo) {
            // starts[p] = offset in flatBuckets where pattern p's bucket begins
            // pos[p] = working copy of starts
            for (int p = 0; p < 243; p++)
                starts[p + 1] = starts[p] + counts[p];
            memcpy(pos, starts, 243 * sizeof(int));
            for (auto &c : candidates)
                flatBuckets[pos[PatternEngine::pm(guessIdx, c.first)]++] = c;

            int worst = 0;
            bool pruned = false;

            for (int p = 0; p < 243 && !pruned; p++) {
                if (!counts[p])
                    continue;

                // for each non-empty bucket,
                // if p is not the correct guess, the cost = 1 + minimax(bucket
                //                                                       value)
                int cost;
                if (p == CORRECT_GUESS) {
                    cost = 1;
                } else {
                    auto bucket =
                        span(flatBuckets.data() + starts[p], counts[p]);
                    cost = 1 + minimax(bucket, best - 1);
                }

                // If the worst case found is worse than the current best, we
                // know we will not continue down with this guess. We prune
                worst = max(worst, cost);
                if (worst >= best)
                    pruned = true;
            }

            if (!pruned) {
                best = worst;
                bestGuessIdx = guessIdx;
            }
        }

        if (onProgress)
            onProgress(gi + 1, totalGuesses);
    }

    // Update the mmCache if we found a guess strictly better
    if (bestGuessIdx != -1)
        mmCache[std::move(key)] = make_pair(best, bestGuessIdx);

    if (bestGuessOut)
        *bestGuessOut = bestGuessIdx;
    return best;
}

void Solver::precomputeMinimax() {
    using namespace chrono;
    auto t0 = steady_clock::now();

    auto root =
        span<const pair<int, int>>(candidateSet).first(CANDIDATE_SET_SIZE);

    int barWidth = 50;
#ifdef TIOCGWINSZ
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 20)
        barWidth = ws.ws_col - 30;
#endif

    auto printProgress = [&](int done, int total) {
        double elapsed = duration<double>(steady_clock::now() - t0).count();
        double frac = (double)done / total;
        int filled = (int)(frac * barWidth);

        string eta = "  --:--";
        if (done > 0) {
            double rem = elapsed / done * (total - done);
            char buf[16];
            snprintf(buf, sizeof(buf), "  %02d:%02d", (int)(rem / 60),
                     (int)rem % 60);
            eta = buf;
        }
        char ebuf[16];
        snprintf(ebuf, sizeof(ebuf), "%02d:%02d", (int)(elapsed / 60),
                 (int)elapsed % 60);

        fprintf(stderr, "\r  [%s%s] %3d/%3d  elapsed %s  eta%s  cache %-8zu",
                string(filled, '#').c_str(),
                string(barWidth - filled, '-').c_str(), done, total, ebuf,
                eta.c_str(), mmCache.size());
        fflush(stderr);
    };

    int bestGuessIdx = -1;
    // Single source of truth for the root search — no duplicated loop.
    // The callback fires after each root-level guess is fully evaluated.
    int cost = minimax(root, INT_MAX, &bestGuessIdx, printProgress);
    if (bestGuessIdx == -1)
        throw runtime_error("Best guess could not be computed");

    double secs = duration<double>(steady_clock::now() - t0).count();
    fprintf(stderr, "\n");
    cout << "Minimax precompute done in " << fixed << setprecision(2) << secs
         << "s\n"
         << "Worst-case guesses: " << cost << "\n"
         << "Root guess:         " << PatternEngine::guessSet[bestGuessIdx]
         << "\n"
         << "Cache entries:      " << mmCache.size() << endl;
}

int Solver::bestGuessIdxMinimax() {
    if (candidateSetSize == 0)
        throw runtime_error("Candidate set is empty, cannot make a guess");
    if (candidateSetSize <= 2)
        return candidateSet[0].second;

    auto candidates =
        span<const pair<int, int>>(candidateSet).first(candidateSetSize);

    int bestGuessIdx = -1;
    minimax(candidates, INT_MAX, &bestGuessIdx);
    if (bestGuessIdx == -1)
        throw runtime_error("Minimax returned no valid guess");
    return bestGuessIdx;
}

string Solver::bestGuessMinimax() {
    int guess = bestGuessIdxMinimax();
    if (guess == -1)
        throw runtime_error("Cannot compute best guess");
    return PatternEngine::guessSet[guess];
}

// ===============================================================================

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

void Solver::saveMMCache(const string &fname) const {
    ofstream f(fname, ios::binary);
    if (!f)
        throw runtime_error("Cannot open " + fname);

    // Calculate the totalBytes of the mmCache
    size_t totalBytes = sizeof(uint32_t);
    for (const auto &[key, val] : mmCache)
        totalBytes += sizeof(uint32_t) +
                      (key.size() * sizeof(uint16_t) + (sizeof(int) * 2));
    vector<char> buffer(totalBytes);
    char *buf = buffer.data();

    uint32_t count = static_cast<uint32_t>(mmCache.size());
    memcpy(buf, &count, sizeof(count));
    buf += sizeof(count);

    for (const auto &[key, val] : mmCache) {
        uint32_t klen = static_cast<uint32_t>(key.size());

        // Pack key length
        memcpy(buf, &klen, sizeof(klen));
        buf += sizeof(klen);

        // Pack key data
        size_t key_bytes = klen * sizeof(uint16_t);
        memcpy(buf, key.data(), key_bytes);
        buf += key_bytes;

        // Pack the pair (cost, guess) simultaneously
        int32_t vals[2] = {val.first, val.second};
        memcpy(buf, vals, sizeof(vals));
        buf += sizeof(vals);
    }

    // Blast the entire buffer to disk in exactly ONE write call
    f.write(buffer.data(), totalBytes);
}

void Solver::loadMMCache(const string &fname) {
    // Open the file and jump to the end (ios::ate) to quickly get the file size
    ifstream f(fname, ios::binary | ios::ate);
    if (!f)
        throw runtime_error("Cannot open " + fname);

    streamsize size = f.tellg();
    f.seekg(0, ios::beg); // Rewind back to the start

    if (size == 0)
        return; // Handle edge case of an empty file

    // Read the entire file into memory in EXACTLY ONE disk read
    vector<char> buffer(size);
    if (!f.read(buffer.data(), size))
        throw runtime_error("Error reading file into memory: " + fname);

    // Setup pointers to iterate through the raw memory buffer
    const char *ptr = buffer.data();
    const char *end = ptr + size;

    if (ptr + sizeof(uint32_t) > end)
        throw runtime_error("File corrupted: too small for count");

    // Extract the count
    uint32_t count;
    memcpy(&count, ptr, sizeof(count));
    ptr += sizeof(count);

    mmCache.clear();
    mmCache.reserve(count);

    // Sequentially parse the buffer
    for (uint32_t i = 0; i < count; i++) {
        // Extract key length
        if (ptr + sizeof(uint32_t) > end)
            throw runtime_error("File corrupted: reading klen");
        uint32_t klen;
        memcpy(&klen, ptr, sizeof(klen));
        ptr += sizeof(klen);

        // Extract key data
        size_t key_bytes = klen * sizeof(uint16_t);
        if (ptr + key_bytes > end)
            throw runtime_error("File corrupted: reading key data");
        vector<uint16_t> key(klen);
        memcpy(key.data(), ptr, key_bytes);
        ptr += key_bytes;

        // Extract cost and guess simultaneously
        if (ptr + sizeof(int32_t) * 2 > end)
            throw runtime_error("File corrupted: reading values");
        int32_t vals[2];
        memcpy(vals, ptr, sizeof(vals));
        ptr += sizeof(vals);

        // Emplace directly into the map to avoid extra copies
        mmCache.emplace(std::move(key), make_pair(vals[0], vals[1]));
    }
}
