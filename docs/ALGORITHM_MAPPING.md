# Algorithm Mapping, Complexity, and Correctness Notes

The project deliberately avoids ready-made algorithm implementations for its assessed core logic. The table names the implementation actually present in `src/algorithms.cpp`; complexities describe that implementation rather than an ideal library alternative.

| Syllabus technique | IssueFlow mapping | Time complexity | Auxiliary space | Evidence / invariant |
| --- | --- | --- | --- | --- |
| Merge Sort | Priority ordering of issues | Θ(n log n) | Θ(n) | Merge copies the lower-ranked remaining item; each merged segment is sorted. |
| Quick Sort | Priority-order comparator | Best/avg Θ(n log n), worst Θ(n²) | Θ(log n) avg recursion, Θ(n) worst | Partition places pivot at its final order position. |
| Iterative/recursive Binary Search | ID retrieval from an ID-sorted copy | Θ(log n) | Θ(1) / Θ(log n) | Target, if present, remains in the active sorted interval. |
| Issue-ID binary-search tree | Domain-specific issue-record index | O(h): O(log n) only when the tree is balanced, O(n) worst case | O(n) | Every left child ID is smaller than its parent; every right child ID is larger. |
| Maximum subarray | Largest cumulative release-risk improvement | Θ(n log n) | Θ(log n) | Best subarray lies left, right, or crosses the midpoint. `T(n)=2T(n/2)+Θ(n)`. |
| Fast exponentiation | Growth/scaling utility | Θ(log e) | Θ(1) | Each step squares the base and halves the exponent. `T(e)=T(e/2)+Θ(1)`. |
| Celebrity discovery | Find a universally depended-on release owner in a relation matrix | Θ(n) | Θ(1) | Elimination leaves one possible candidate, then verification proves it. |
| Classical / Strassen multiplication | Dependency/metric matrix comparison | Θ(n³) / Θ(n^log₂7) | Θ(n²), plus recursion | Both compute the same matrix product; padding enables Strassen on non-power-of-two sizes. |
| Activity selection | Non-overlapping maintenance-window selection | Θ(n²) with manual insertion sorting | Θ(n) result | Earliest-finish compatible activity is safe by the exchange argument. |
| Interval partitioning | Minimum concurrent maintenance lanes | Θ(n²) | Θ(n) | Reuses the earliest compatible lane; otherwise opens a new lane. |
| Huffman coding | Compact issue-event labels | Θ(k²) with manual repeated-min selection | Θ(k) | Combining two least-frequent nodes yields an optimal prefix-code subproblem. |
| Dijkstra | Non-negative team/service routing matrix | Θ(V²) | Θ(V) | Settled node has its final shortest distance. |
| Prim / Kruskal | Minimum integration/infrastructure link cost | Θ(V²) / Θ(E²) | Θ(V) | Prim grows a cut-minimum tree; Kruskal never joins a cycle. |
| 0/1 Knapsack | Best issue workload under a capacity | Θ(nC) | Θ(nC) | Table cell is the best priority using first `i` issues and capacity `c`. |
| Matrix-chain multiplication | Cheapest chain of analytics matrix operations | Θ(n³) | Θ(n²) | Every optimum has an optimal final split. |
| LCS | Compare module/version labels | Θ(mn) | Θ(mn) | Equal final characters extend an optimal prefix LCS; otherwise discard one suffix. |
| Optimal BST | Frequency-weighted issue-ID lookup tree cost | Θ(n³) | Θ(n²) | Every candidate root partitions into independent optimal subtrees. |
| Bellman-Ford | Route analysis allowing negative weights | Θ(VE) | Θ(V) | After `i` rounds, paths using at most `i` edges are correct. |
| Floyd-Warshall | All-pairs module/service route analysis | Θ(V³) | Θ(V²) | Iteration `k` permits exactly vertices `0..k` as intermediates. |
| Bounded TSP | Small release-tour planning instance | Θ(n²2ⁿ) | Θ(n2ⁿ) | Held-Karp state is an optimal path for its visited subset and final node. |
| N-Queens | Backtracking constraint validation | O(n!) | O(n) state | Each recursive row keeps only non-attacking placements. |
| Subset Sum | Backtracking capacity/effort validation | O(2ⁿ) | O(n) stack | Branches exhaust inclusion/exclusion choices. |
| FIFO/LIFO/least-cost Branch and Bound | Exact 0/1 workload decision strategies | Worst O(2ⁿ) | O(number of live nodes) | Fractional-knapsack bound safely prunes nodes that cannot beat the incumbent. |
| Ford-Fulkerson (BFS residual variant) | Capacity-flow model | O(VE²) | Θ(V²) matrix | Each augmentation follows residual capacity and increases feasible flow. |
| Maximum bipartite matching | Primary developer-to-issue assignment | O(U·E) DFS augmentation | O(U+V) | An augmenting path increases matching size by one; no augmenting path means maximum cardinality. |
| Naive / Rabin-Karp / finite automaton / KMP | Title/description pattern search | O(nm) / expected O(n+m) / preprocessing O(m³·Σ), scan O(n) / O(n+m) | O(1) / O(1) / O(mΣ) / O(m) | Each returns exactly the starting positions whose substring equals the pattern. |
| Chinese Remainder Theorem | Modular scheduling utility | O(k log M) | Θ(1) beyond input | Extended Euclid finds each modular inverse; pairwise-coprime moduli guarantee a unique result modulo the product. |

## Product-level correctness arguments

### Validation

Validation first establishes record-level preconditions, then checks every dependency endpoint, then runs cycle detection. All assignment and dependency features consume only a validated repository through the normal loading workflow.

### Priority order

The comparator is deterministic: descending priority, then ascending effort, then ascending ID. Merge Sort and Quick Sort therefore receive the same total ordering. The test suite checks agreement on the supplied dataset.

### Dependency-aware order

An issue is emitted only when all incoming dependency edges have been removed, so every dependency precedes its dependent. Failure to find a zero-indegree node before completion is exactly the cycle condition.

### Maximum matching

The matcher starts from an empty matching and applies one DFS augmentation for each issue. The symmetric difference of a matching and an augmenting path has one additional matched edge, so every successful augmentation increases coverage. When all attempted augmentations fail, Berge's theorem gives maximum cardinality.

### 0/1 workload selection

For each issue and capacity, the dynamic-programming table takes the better of excluding the issue or including it plus the best feasible previous capacity. Induction over the first `i` issues proves the final cell is optimal for the selected developer capacity.

## Empirical comparison protocol

Option 11 uses deterministic generated repositories of 100, 500, and 1000 issues. It also generates an already priority-sorted input which is adversarial for the final-element Quick Sort pivot. It emits per-row elapsed milliseconds, comparisons, operations, an auxiliary-memory estimate, and quality/coverage. For the final report:

1. Run it three times on the same machine after closing other heavy applications.
2. Export each result to CSV and report median elapsed time for every algorithm/input-size pair.
3. Graph input size against time and operation count separately.
4. Compare matching coverage with greedy capacity-allocation coverage, and explain the differing objectives.
5. Record compiler, C++ standard, operating system, CPU, RAM, build type, and timestamp beside the tables.

Do not claim one method is universally better: Merge Sort offers predictable worst-case time and extra memory; Quick Sort often uses less extra storage but has a bad pivot-order worst case. Maximum matching maximizes one-to-one coverage, whereas the greedy allocator considers remaining capacity but is heuristic.
