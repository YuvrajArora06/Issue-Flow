# Test Plan

Run the automated suite with `ctest --test-dir build -C Release --output-on-failure` or the direct compiler command in the README. The suite covers the normal repository workflow, CSV parsing, all text-search variants, the algorithm laboratory, matching, flow, and invalid dependency cycles.

| Category | Dataset/action | Expected result |
| --- | --- | --- |
| Normal | `data/developers.csv` + `data/issues.csv` | Validation passes; five issues and three developers load. |
| Priority | Option 4 on normal data | `I1` is first; Merge and Quick Sort agree on the deterministic comparator. |
| Dependency | Option 7 on normal data | `I1` appears before `I2` and `I4`. |
| Matching | Option 8 on normal data | Three one-to-one developer assignments; unmatched issues reported. |
| Capacity | Option 9 on normal data | Four feasible assignments, no negative remaining capacity. |
| Capacity shock | `data/capacity_shock_developers.csv` + normal issues | More work remains unresolved; no capacity is exceeded. |
| Invalid dependency | Normal developers + `data/invalid_cycle_issues.csv` | Loading is refused with a cycle error. |
| Invalid CSV schema | `data/invalid_developers_header.csv` + normal issues | Loading is refused with the required header order. |
| Boundary | Empty pattern in description search | Every valid string boundary is a match. |
| Boundary | Binary search for a missing ID | Reports not found without an invalid index. |
| Boundary | Knapsack capacity zero | Returns an empty workload. |
| Adversarial | Quick Sort already sorted / reverse-priority generated data | Benchmark exposes pivot-order sensitivity. |
| Stress | Option 11 | Completes deterministic 100, 500, and 1000 issue benchmark runs and may export CSV. |
| Exact algorithms | Option 12 | Outputs known values: maximum subarray 6, N-Queens(4) 2, max flow 23, CRT 23. |

## Regression rule

Any code change must be followed by a clean compile with warnings enabled and the complete automated suite. If an algorithm's objective changes, update its mapping, expected test result, and complexity note together.
