# Wordle Solver

C++20 Wordle solver with two strategies sharing a precomputed pattern matrix: an **entropy solver** minimizing average guesses, and a **minimax + alpha-beta solver** minimizing worst-case guesses.

## Results

| Strategy | Avg. guesses |
|---|---|---|
| Entropy (greedy) | 3.46 |
| Minimax + alpha-beta | 3.52 |

## Word lists

- **Guess set** (`data/guess-set.txt`, 12,972 words) — all valid guesses.
- **Candidate set** (`data/candidate-set.txt`, 2,315 words) — possible answers.

A guess need not be a candidate; non-candidate guesses can still be maximally informative.

## Pattern encoding

Feedback (`GREEN`/`YELLOW`/`GREY` per letter) is encoded as a base-3 integer in `[0, 242]`. `computePattern` uses a two-pass green-then-yellow scan with a letter-frequency array to handle duplicate letters per official Wordle rules.

`patternMatrix[guess][candidate]` precomputes every (guess, candidate) pattern once (OpenMP-parallelized), cached to `data/pattern-matrix.bin`.

## Entropy solver

For guess $g$, candidates partition by feedback pattern $p_i$ with probability $P(p_i) = \text{count}(p_i) / |\text{candidates}|$. Expected information gain:

$$H(g) = -\sum_{i=1}^{243} P(p_i) \log_2 P(p_i)$$

The solver picks $\arg\max_g H(g)$ over the full guess set. This minimizes *expected* decision-tree depth but is blind to worst-case bucket shape.

**Endgame:** at ≤2 candidates, entropy is degenerate, so the solver guesses a candidate directly — provably optimal ($\mathbb{E}=1.5$ vs. 2.0 for a non-candidate probe). A $\epsilon=0.001$ tiebreaker favors candidate guesses on ties.

## Minimax solver

Solves:

$$\text{cost}(C) = \min_{g} \left[ \max_{p_i} \big(1 + \text{cost}(\text{partition}(g, p_i))\big) \right]$$

directly optimizing worst-case depth instead of expected depth.

- **Alpha-beta pruning:** `alpha` tracks the best worst-case found so far; a guess is abandoned the moment its running worst case meets or exceeds `alpha`.
- **Entropy-ordered search:** guesses are tried in entropy order (`sortByEntropy`) so a strong `alpha` is found early, maximizing pruning. Below `CANDIDATE_ONLY_THRESHOLD` (15) only remaining candidates are tried; above it, only the top `ENTROPY_SORT_CAP` (300) guesses by entropy are searched.
- **Memoization:** each call is keyed by the sorted candidate-index set in `mmCache`, since the same reduced set is reachable via many guess paths. Precomputed once and persisted to `data/mmcache.bin`.
- **Bounds:** `MINIMAX_MAX_DEPTH` (30) and an `alpha < 2` early-out cap the recursion.

## Why two solvers

Entropy minimizes $\mathbb{E}[\text{depth}]$; minimax minimizes $\max(\text{depth})$. This is why openers can differ: `soare` maximizes entropy (5.886 bits) but minimax may prefer `salet`/`crane` for a shallower worst case, since entropy only sees bucket sizes, not how hard each bucket is to resolve afterward. Neither is "wrong" — each is optimal for its own objective.

## Project structure

```
include/        pattern.h (PatternEngine), solver.h (Solver)
src/             pattern.cpp, solver.cpp, main.cpp
data/            word lists, cached pattern matrix, cached minimax results
```

`candidateSet: vector<pair<int,int>>` — `.first` indexes `patternMatrix` columns, `.second` indexes the guess set. `filterWords` partitions in place via swaps (no reallocation).

## Building

Requires CMake ≥ 3.14, C++20, OpenMP.

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make
./WordleSolver
```

`pattern-matrix.bin` and `mmcache.bin` are built and cached on first run if missing.

## Usage

`main()` runs an interactive loop (`playGame`): prints the best entropy and minimax guesses each turn, accepts `guess pattern` (e.g. `crane 02001`, digits = grey/yellow/green), and filters candidates.

For a full benchmark across all 2,315 answers, call `testAvgGuess(s)` instead — reports guess-count distribution, failures, and average for both strategies.

## Regenerating word lists

```bash
cd data && python3 order-sets.py
```

Re-sorts `guess-set.txt` and rewrites `candidate-set-indices.bin`. Delete `pattern-matrix.bin` and `mmcache.bin` to force a rebuild.
