#include <bits/stdc++.h>
#include <pattern.h>
#include <solver.h>

int main() {
    Solver s("data/guess-set.txt", "data/candidate-set.txt",
             "data/candidate-set-indices.txt", "data/pattern-matrix.bin");

    cout << s.candidateSet.size() << " " << s.bestGuess() << endl;
    vector<int> res{0, 0, 0, 1, 1};
    s.filterWords(res);
    cout << s.candidateSet.size() << " " << s.bestGuess() << endl;
    res = {0, 2, 0, 2, 2};
    s.filterWords(res);
    cout << s.candidateSet.size() << " " << s.bestGuess() << endl;
    res = {0, 0, 1, 1, 0};
    s.filterWords(res);
    cout << s.candidateSet.size() << " " << s.bestGuess() << endl;
    res = {0, 0, 0, 0, 0};
    s.filterWords(res);
    cout << s.candidateSet.size() << " " << s.bestGuess() << endl;
}
