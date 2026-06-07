#include <bits/stdc++.h>
#include <pattern.h>
#include <solver.h>

int main() {
    Solver s("data/guess-set.txt", "data/candidate-set.txt",
             "data/candidate-set-indices.txt", "data/pattern-matrix.bin");

    cout << s.candidateSet.size() << " " << s.bestGuess() << endl;
    vector<int> res1{0, 0, 0, 0, 0};
    s.filterWords(res1);
    cout << s.candidateSet.size() << " " << s.bestGuess() << endl;
    vector<int> res2{0, 0, 0, 0, 1};
    s.filterWords(res2);
    cout << s.candidateSet.size() << " " << s.bestGuess() << endl;
    vector<int> res3{0, 1, 0, 2, 0};
    s.filterWords(res3);
    cout << s.candidateSet.size() << " " << s.bestGuess() << endl;
}
