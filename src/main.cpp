#include <algorithm>
#include <bits/stdc++.h>
#include <pattern.h>
#include <solver.h>
#include <stdexcept>

void computeAvgGuesses(Solver &s, function<int()> guessFn) {
    vector<int> nGuesses;
    vector<string> failures;
    int guessSplit[6] = {};

    // Terminal width for progress bar
    int barWidth = 50;
#ifdef TIOCGWINSZ
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 20)
        barWidth = ws.ws_col - 35;
#endif

    auto t0 = chrono::steady_clock::now();

    auto printProgress = [&](int done) {
        double elapsed =
            chrono::duration<double>(chrono::steady_clock::now() - t0).count();
        double frac = (double)done / CANDIDATE_SET_SIZE;
        int filled = (int)(frac * barWidth);

        string eta = "--:--";
        if (done > 0) {
            double rem = elapsed / done * (CANDIDATE_SET_SIZE - done);
            char buf[16];
            snprintf(buf, sizeof(buf), "%02d:%02d", (int)(rem / 60),
                     (int)rem % 60);
            eta = buf;
        }
        char ebuf[16];
        snprintf(ebuf, sizeof(ebuf), "%02d:%02d", (int)(elapsed / 60),
                 (int)elapsed % 60);

        fprintf(stderr, "\r  [%s%s] %4d/%4d  elapsed %s  eta %s  fail %-4zu",
                string(filled, '#').c_str(),
                string(barWidth - filled, '-').c_str(), done,
                CANDIDATE_SET_SIZE, ebuf, eta.c_str(), failures.size());
        fflush(stderr);
    };

    for (int i = 0; i < CANDIDATE_SET_SIZE; i++) {
        s.reset();
        const string &curAnswer = PatternEngine::candidateSetWords[i];

        bool solved = false;
        for (int g = 1; g <= 6; g++) {
            int guessIdx = guessFn();
            uint8_t p = PatternEngine::computePattern(
                PatternEngine::guessSet[guessIdx], curAnswer);

            if (p == CORRECT_GUESS) {
                nGuesses.push_back(g);
                guessSplit[g - 1]++;
                solved = true;
                break;
            }

            // Only filter if there are more guesses to make.
            // No point filtering after guess 6 — the loop ends anyway.
            if (g < 6) {
                pattern pat = PatternEngine::decodePattern(p);
                s.filterWords(guessIdx, pat);
            }
        }

        if (!solved)
            failures.push_back(curAnswer);

        printProgress(i + 1);
    }

    // Move past the progress bar before printing results
    fprintf(stderr, "\n");

    // ── Results ──────────────────────────────────────────────────────────
    int guessSum = 0;
    for (int g : nGuesses)
        guessSum += g;

    cout << "\nResults over " << CANDIDATE_SET_SIZE << " words:\n";
    for (int i = 1; i <= 6; i++)
        cout << "  " << i << " guess" << (i == 1 ? " : " : "es: ")
             << guessSplit[i - 1] << "\n";

    if (!failures.empty()) {
        cout << "  Failed (" << failures.size() << "): ";
        for (const auto &w : failures)
            cout << w << " ";
        cout << "\n";
    }

    cout << "  Solved:          " << nGuesses.size() << " / "
         << CANDIDATE_SET_SIZE << "\n";
    cout << fixed << setprecision(4);
    cout << "  Average guesses: "
         << (nGuesses.empty() ? 0.0 : guessSum / (double)nGuesses.size())
         << "\n\n";
}

void testAvgGuess(Solver &s) {
    auto bgiMinimax = bind(&Solver::bestGuessIdxMinimax, &s);
    auto bgiEntropy = bind(&Solver::bestGuessIdxEntropy, &s);
    computeAvgGuesses(s, bgiMinimax);
    computeAvgGuesses(s, bgiEntropy);
}

void playGame(Solver &s) {
    s.reset();
    while (s.candidateSetSize > 0) {
        auto candidates =
            span<pair<int, int>>(s.candidateSet).first(s.candidateSetSize);
        int entropyIdx = s.bestGuessIdxEntropy();
        int minimaxIdx = s.bestGuessIdxMinimax();

        cout << "\nCurrent candidate set size: " << s.candidateSetSize << endl;
        cout << "Entropy  best: " << PatternEngine::guessSet[entropyIdx]
             << "  entropy=" << s.entropyScore(entropyIdx, candidates) << "\n";

        cout << "Minimax  best: " << PatternEngine::guessSet[minimaxIdx]
             << "  entropy=" << s.entropyScore(minimaxIdx, candidates) << "\n"
             << endl;

        if (s.candidateSetSize == 1)
            break;

        cout << "Enter Guess & pattern: ";
        string guess;
        int pat;
        cin >> guess >> pat;
        int guessIdx = PatternEngine::getWordGuessIdx(guess);
        if (guessIdx == -1) {
            cout << "Invalid guess!" << endl;
            continue;
        }
        pattern p;
        while (pat) {
            p.push_back(pat % 10);
            pat /= 10;
        }
        reverse(p.begin(), p.end());
        s.filterWords(guessIdx, p);
    }
    if (s.candidateSetSize == 0)
        throw runtime_error("Candidate Set Size = 0");
}

int main() {
    Solver s("data/guess-set.txt", "data/candidate-set.txt",
             "data/candidate-set-indices.bin", "data/pattern-matrix.bin",
             "data/mmcache.bin");
    cout << endl;

    // testAvgGuess(s);

    playGame(s);
}
