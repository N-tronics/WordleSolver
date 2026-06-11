#include <bits/stdc++.h>
#include <pattern.h>
#include <solver.h>

int main() {
    Solver s("data/guess-set.txt", "data/candidate-set.txt",
             "data/candidate-set-indices.bin", "data/pattern-matrix.bin");

    cout << s.bestGuess() << " " << s.entropyScore(s.bestGuessIdx()) << endl;
    return 0;

    vector<int> nGuesses;
    vector<string> failures;
    for (int i = 0; i < CANDIDATE_SET_SIZE; i++) {
        s.reset();
        string curAnswer = PatternEngine::candidateSetWords[i];
        cout << "Testing on word " << i + 1 << " : " << curAnswer << " ... "
             << flush;
        for (int g = 1; g <= 7; g++) {
            string guess = s.bestGuess();
            pattern p = PatternEngine::getPattern(guess, curAnswer);
            if (PatternEngine::encodePattern(p) == CORRECT_GUESS) {
                nGuesses.push_back(g);
                cout << "Took " << g << " guesses" << endl;
                break;
            } else if (g == 7) {
                failures.push_back(curAnswer);
                cout << "Failed" << endl;
                break;
            }
            s.filterWords(p);
        }
    }
    cout << "Correct guesses: " << nGuesses.size() << endl;
    vector<int> guessSplit(6, 0);
    int guessSum = 0;
    for (int i : nGuesses) {
        guessSplit[i - 1]++;
        guessSum += i;
    }
    for (int i = 1; i <= 6; i++)
        cout << i << " guesses: " << guessSplit[i - 1] << endl;
    if (!failures.empty()) {
        cout << "Failures: ";
        for (auto s : failures)
            cout << s << " ";
        cout << endl;
    }
    cout << "Average guesses: " << guessSum / (double)nGuesses.size() << endl;
}
