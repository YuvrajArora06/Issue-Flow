# IssueFlow

IssueFlow is an offline C++17 command-line application for prioritizing open-source issues, checking dependency order, finding issue records, searching issue text, and assigning compatible developers. It is deliberately structured as a Design and Analysis of Algorithms workbench: each assessed algorithm is manually implemented and can be demonstrated from the program or its tests.

## Product workflow

1. Load developer and issue records from local CSV files, or begin with the built-in sample repository.
2. Validate IDs, priority and effort ranges, skills, dependencies, and cycles.
3. Prioritize work with Merge Sort or Quick Sort and compare measurements.
4. Produce a prerequisite-first release order with dependency analysis.
5. Assign developers with maximum bipartite matching, then compare that one-to-one baseline with a capacity-aware greedy allocation.
6. Search issue IDs with iterative or recursive Binary Search or a manual Issue-ID tree index; search text with Naive, Rabin-Karp, finite-automaton, or KMP matching.
7. Run the algorithm laboratory and benchmark runner for reproducible DAA evidence.

## Build and test

### CMake (recommended)

```powershell
cmake -S . -B build
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

On single-configuration generators, the executables are usually `build/issueflow_cli` and `build/issueflow_tests`. On Visual Studio generators, use `build/Release/`.

### Direct MinGW build

```powershell
g++ -std=c++17 -Wall -Wextra -Wpedantic -Iinclude src/domain.cpp src/algorithms.cpp src/main.cpp -o build/issueflow_cli.exe
g++ -std=c++17 -Wall -Wextra -Wpedantic -Iinclude src/domain.cpp src/algorithms.cpp tests/test_issueflow.cpp -o build/issueflow_tests.exe
.\build\issueflow_tests.exe
.\build\issueflow_cli.exe
```

### VS Code setup

The repository includes `.vscode/c_cpp_properties.json` and `.vscode/tasks.json` for the installed MinGW compiler at `C:/MinGW/bin/g++.exe`. Reload the VS Code window after opening the project, then use **Terminal > Run Build Task** to build the CLI or **Tasks: Run Task > Test IssueFlow** to rebuild and run the tests. This supplies IntelliSense with the project `include/` directory and C++17 standard so header-resolution diagnostics should clear.

## Offline CSV input

Use `data/developers.csv` and `data/issues.csv` as the normal-operation scenario. List-valued fields use `|`; use standard double quotes for fields containing commas.

`developers.csv`

```text
id,name,capacity,skills
D1,Asha,8,Backend|Auth
```

`issues.csv`

```text
id,title,description,priority,effort,module,skills,dependencies,labels
I1,Login crash,Authentication fails after a valid login request,10,4,Auth,Auth,-,bug|critical
```

Rules enforced by validation:

- IDs are unique and non-empty.
- Priority is in `1..10`; issue effort is positive; capacity is non-negative.
- Every dependency names an existing, different issue.
- Dependency graphs must be acyclic.
- An assignment requires every listed skill and sufficient capacity.

## Menu guide

| Option | Purpose |
| --- | --- |
| 2 | Load local developer and issue CSV files. Invalid datasets are not accepted. |
| 4 | Compare manual Merge Sort and Quick Sort for issue priority. |
| 5 | Retrieve a known ID using iterative/recursive Binary Search or the Issue-ID tree index. |
| 6 | Search titles/descriptions. |
| 7 | Display a prerequisite-first dependency order or report cycle nodes. |
| 8 | Run the primary maximum bipartite-matching baseline. One developer is matched to at most one issue. |
| 9 | Run the capacity-aware greedy comparator; a developer may receive multiple feasible issues. |
| 10 | Select a priority-maximizing workload under a capacity with 0/1 Knapsack. |
| 11 | Compare timing, operation count, auxiliary-space estimate, and allocation coverage on generated and adversarial datasets. |
| 12 | Run deterministic examples for the remaining syllabus algorithms. |

Options 8 and 9 export assignment plans as CSV or JSON. Option 11 exports benchmark rows as CSV.

## Required demonstrations

- **Normal operation:** load `data/developers.csv` and `data/issues.csv`, validate, compare sorting, show dependency order, then run options 8 and 9.
- **Capacity shock:** load `data/capacity_shock_developers.csv` with `data/issues.csv`; compare unresolved work against normal operation.
- **Invalid/adversarial dependency input:** load `data/invalid_cycle_issues.csv` with the normal developers file; validation must reject the cycle.
- **Large-data stress:** option 11 deterministically generates 100, 500, and 1000 issue datasets, including sorted adversarial Quick Sort input. Use the CSV export for the final report tables/graphs.

## Project evidence

- Algorithm mapping, complexity, recurrence notes, and core correctness arguments: [docs/ALGORITHM_MAPPING.md](docs/ALGORITHM_MAPPING.md)
- Architecture and data flow: [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md)
- Test matrix: [docs/TEST_PLAN.md](docs/TEST_PLAN.md)
- Demonstration/viva script: [docs/DEMO_SCRIPT.md](docs/DEMO_SCRIPT.md)
- Complexity classes, reduction mapping, recurrences, and production strategy: [docs/COMPLEXITY_CLASS_AND_REDUCTION.md](docs/COMPLEXITY_CLASS_AND_REDUCTION.md)
- Developer/API guide: [docs/DEVELOPER_GUIDE.md](docs/DEVELOPER_GUIDE.md)
- Dataset provenance: [data/PROVENANCE.md](data/PROVENANCE.md)
- Benchmark evidence: [evidence/benchmark_results_2026-11-18.csv](evidence/benchmark_results_2026-11-18.csv)
- Automated test evidence: [evidence/test_results_2026-11-18.md](evidence/test_results_2026-11-18.md)

## Current limitations

The matching result is intentionally a one-to-one maximum-cardinality baseline, while the greedy allocator handles multiple issues per developer but is not globally optimal. The exact algorithms are guarded to small inputs (`n <= 15` for TSP and `n <= 14` for N-Queens). The prototype is offline by design: it accepts local CSV datasets and does not call GitHub or any web API.
