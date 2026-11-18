# Demonstration and Viva Script

## Four-minute product demonstration

1. Start the program and explain the entities: issue ID, priority, effort, required skills, dependencies; developer ID, skills, and capacity.
2. Use option 1 to show the built-in normal dataset, then option 3 to show validation.
3. Use option 4. Explain the deterministic priority comparator and why Merge Sort and Quick Sort are compared.
4. Use option 7. Point out that `I1` must be released before `I2` and `I4`.
5. Use option 8. Explain the bipartite graph: developers are one side, issues the other, compatibility is an edge. State that the mode maximizes one-to-one assignment coverage.
6. Use option 9. Contrast the capacity-aware greedy result with matching; explain why it can cover more issues but is not globally optimal.
7. Use option 6 with `timeout` or `Backend` to show a KMP search, then option 5 to show Binary Search.
8. Use option 11 to explain time, operation, space, and solution-quality fields. Export CSV if requested.
9. Use option 12 and identify the known outputs for N-Queens, max flow, and CRT.

## Likely viva questions

| Question | Short answer |
| --- | --- |
| Why is matching primary? | It directly models compatible developer-to-issue edges and maximizes the number of one-to-one valid assignments. |
| Why is the greedy result different? | It accepts multiple tasks per developer while capacity remains; its objective is practical coverage, not a proof of optimal global allocation. |
| How do cycles fail safely? | Topological processing cannot emit every node; validation rejects the dataset before plans are calculated. |
| Why compare Merge and Quick Sort? | They sort the same priority model but trade predictable memory/time against practical in-place performance and pivot sensitivity. |
| Where is dynamic programming used? | 0/1 Knapsack selects an optimal priority-weighted workload under capacity; matrix chain, LCS, OBST, shortest-path, and TSP modules are also implemented. |
| What is the worst limitation? | Exact TSP, N-Queens, Subset Sum, and branch-and-bound are exponential, so the program explicitly limits or demonstrates them on small instances. |
| What change can you make live? | Add a CSV issue, change its dependency or skill, reload the file, validate it, and show the changed order/assignment. |
