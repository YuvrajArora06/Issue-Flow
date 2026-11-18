# Complexity Classes, Reductions, and Production Strategy

## Complexity classification

IssueFlow separates the problems it can solve exactly in polynomial time from the deliberately bounded exact demonstrations.

| Product decision problem | Classification used by IssueFlow | Reason |
| --- | --- | --- |
| Is an issue ID present in a sorted list or Issue-ID tree? | P | Binary search and binary-search-tree lookup run in logarithmic time on balanced input. |
| Is a dependency-aware release order possible? | P | Cycle detection and topological ordering run in `O(V + E)` once the graph is built. |
| Can at least `k` compatible developer-issue pairs be formed? | P | Bipartite maximum matching is polynomial. |
| Can one developer select issues with total effort at most `C` and priority at least `P`? | NP-complete decision version | This is the 0/1 Knapsack decision problem. The implementation uses pseudo-polynomial dynamic programming, `O(nC)`. |
| Can multiple developers receive issue sets while meeting all capacities and reaching a global target value? | NP-hard generalization | The multi-developer capacity-allocation form contains the one-developer Knapsack case. IssueFlow therefore uses matching and a documented greedy heuristic rather than claiming an exact polynomial solution. |
| Is there a route through all vertices with cost at most `B`? | NP-complete decision version | This is the Travelling Salesperson decision problem. The Held-Karp implementation is explicitly limited to 15 vertices. |
| Does a subset of effort values equal a target? | NP-complete decision version | This is Subset Sum. It is included as a small backtracking demonstration. |

N-Queens is included as a backtracking exercise. This report does not label its decision form NP-complete; the program measures it only on small, bounded inputs.

## Reduction mapping: 0/1 Knapsack to a single-developer workload

The 0/1 Knapsack decision problem supplies items `i`, value `v_i`, weight `w_i`, capacity `C`, and target value `V`.

| Knapsack element | IssueFlow element |
| --- | --- |
| Item `i` | Candidate issue `i` |
| Value `v_i` | Issue priority |
| Weight `w_i` | Estimated issue effort |
| Capacity `C` | One developer's available capacity |
| Target `V` | Required total priority |

This mapping is identity-preserving and computable in linear time. A feasible Knapsack subset is exactly a feasible issue workload with the same effort and priority total; the reverse mapping is identical. Therefore, the generalized capacity-allocation objective cannot be assumed to have a polynomial exact solution merely because bipartite matching does.

## Recurrences used in the report

| Algorithm | Recurrence | Solution / interpretation |
| --- | --- | --- |
| Merge Sort | `T(n) = 2T(n/2) + Θ(n)` | `Θ(n log n)` by the Master Theorem. |
| Maximum Subarray | `T(n) = 2T(n/2) + Θ(n)` | `Θ(n log n)`; a best subarray is left, right, or crossing. |
| Fast Power | `T(e) = T(e/2) + Θ(1)` | `Θ(log e)` because the exponent halves each step. |
| Quick Sort, balanced pivot | `T(n) = 2T(n/2) + Θ(n)` | `Θ(n log n)`. |
| Quick Sort, final-pivot adversarial input | `T(n) = T(n-1) + Θ(n)` | `Θ(n²)`. The benchmark evidence demonstrates this case. |
| Strassen multiplication | `T(n) = 7T(n/2) + Θ(n²)` | `Θ(n^log₂7)`, approximately `Θ(n^2.807)`. |

## Recommended production strategy

1. Reject invalid IDs, capacities, and cyclic dependencies before planning work.
2. Use dependency-aware ordering before assignment so prerequisite work remains visible.
3. Use maximum bipartite matching when one developer should receive at most one issue and maximum valid coverage is the objective.
4. Use the greedy capacity allocator when developers may take several issues; label its output as heuristic, not globally optimal.
5. Use 0/1 Knapsack for one-developer workload selection only when its capacity bound is small enough for `O(nC)` memory and time.
6. Keep TSP, N-Queens, Subset Sum, and branch-and-bound in bounded demonstration mode rather than a production release planner.

## Limitations and responsible use

Priority and effort are human estimates, so an assignment plan is decision support rather than a command to assign work. Skills can become stale, and a valid skill match does not capture availability, consent, ownership, or fairness. Maintainers should review every output, avoid using the tool to make personnel decisions automatically, and protect any real repository data before replacing the synthetic course dataset.
