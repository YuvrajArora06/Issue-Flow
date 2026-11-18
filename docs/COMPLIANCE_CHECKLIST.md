# Project 50 Compliance Checklist

Status is based on the current repository as of 2026-11-18. A check means the source and/or evidence artifact exists; a partial marker identifies the remaining honest requirement.

## Project explanation in ten lines

| Requirement | Status | Evidence |
| --- | --- | --- |
| Issues, labels, modules, developers, dependencies, records, arrays, trees, and graphs | ✅ | `Issue` includes labels/modules/dependencies; vectors store records; `IssueIdIndex` is a manual tree; dependency, weighted, and capacity graphs are implemented. |
| Matching, dependency-aware priority, constraints, validation, measurable indicators | ✅ | Matching/allocation, topological ordering, validation, and algorithm metrics. |
| Merge/Quick Sort, iterative/recursive Binary Search, domain index | ✅ | CLI options 4 and 5; manual Issue-ID tree index. |
| Maximum subarray, fast power, celebrity, classical/Strassen multiplication | ✅ | Algorithm Lab and automated tests. |
| Activity selection, partitioning, Huffman, Dijkstra, Prim, Kruskal | ✅ | Algorithm Lab and automated tests. |
| Knapsack, matrix chain, LCS, OBST, Bellman-Ford, Floyd-Warshall, bounded TSP | ✅ | Algorithm Lab and automated tests. |
| N-Queens, Subset Sum, FIFO/LIFO/least-cost Branch and Bound | ✅ | Algorithm Lab and automated tests. |
| Ford-Fulkerson and maximum bipartite matching | ✅ | Algorithm Lab plus integrated assignment workflow. |
| CRT and Naive/Rabin-Karp/finite-automaton/KMP matching | ✅ | Algorithm Lab and exhaustive matcher agreement test. |
| Recurrences, complexity, empirical results, correctness, P/NP, reduction, production strategy | ✅ | Algorithm mapping, complexity-class/reduction report, benchmark CSV, graph, and production recommendation. |

## Expected product

| Expected product | Status | Evidence |
| --- | --- | --- |
| Portable C/C++ triage product with text search and assignment plans | ✅ | Offline C++17 CLI. |
| Reusable library with mandatory algorithms behind interfaces | ✅ | `include/issueflow/` and `src/algorithms.cpp`. |
| Menu CLI for data, algorithm selection, scenarios, CSV/JSON export | ✅ | CLI options 2-12; assignment CSV/JSON export and benchmark CSV export. |
| Benchmark runner for time, memory, solution quality, scale, and adversarial input | ✅ | Option 11 and `evidence/`. |
| Tests with small oracles, unit/integration, edge, invalid, regression, and stress coverage | ✅ | `tests/test_issueflow.cpp`, invalid-data fixtures, and benchmark stress runs. |
| Git repository, history, release tag, and individual contribution statement | ⚠️ | CMake/source/docs exist. Git history/tag and the signed declaration still require the student. |

## Minimum demonstrations

| Scenario | Status | How to demonstrate |
| --- | --- | --- |
| Normal operation | ✅ | Load `data/developers.csv` and `data/issues.csv`. |
| Capacity shock | ✅ | Load `data/capacity_shock_developers.csv` with normal issues and compare unresolved work. |
| Large-data stress | ✅ | Run option 11. |
| Two-strategy comparison on same input | ✅ | Compare Merge vs Quick Sort; compare matching vs greedy allocation. |

## Final submission checklist

| Submission item | Status | Remaining action, if any |
| --- | --- | --- |
| Compilable source and automated build | ✅ | Build tasks and CMake configuration exist. |
| Algorithm implementations and mapping table | ✅ | See `docs/ALGORITHM_MAPPING.md`. |
| Sample/generated datasets with provenance | ✅ | See `data/PROVENANCE.md`. |
| Unit/integration/regression/stress-test evidence | ✅ | See `evidence/test_results_2026-11-18.md`. |
| Benchmark table, graph, compiler/hardware details | ⚠️ | Results and graph exist; add CPU marketing name and RAM from the final machine. |
| Correctness and complexity report with recurrence solutions | ✅ | See algorithm mapping and complexity-class report. |
| P, NP, NP-complete, NP-hard discussion and reduction mapping | ✅ | See `docs/COMPLEXITY_CLASS_AND_REDUCTION.md`. |
| User guide, developer guide, architecture, API notes | ✅ | README, developer guide, and architecture document. |
| Recorded demonstration or live demonstration script | ✅ | Live demonstration script exists. |
| Git history, release tag, signed contribution/originality statement | ❌ | Must be created and signed by the student; do not fabricate these records. |
