# Automated Test Evidence

Date: 2026-11-18

## Command executed

```powershell
g++ -std=c++17 -O2 -Werror -Wall -Wextra -Wpedantic -Iinclude src/domain.cpp src/algorithms.cpp tests/test_issueflow.cpp -o build/issueflow_tests_final.exe
.\build\issueflow_tests_final.exe
```

## Result

```text
All IssueFlow tests passed.
```

## Coverage represented by the automated suite

- Normal CSV loading, labels, validation, dependency ordering, matching, greedy allocation, 0/1 Knapsack, and JSON export.
- Regression inputs for malformed headers and cyclic dependencies.
- Iterative/recursive binary search and the manual Issue-ID tree index.
- Exhaustive small binary-string agreement across Naive, Rabin-Karp, finite-automaton, and KMP matching.
- All Algorithm Lab methods, including invalid-input guards.
- Generated benchmark rows, including sorted adversarial Quick Sort input.
