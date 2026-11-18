# Developer and API Guide

## Build and test

The application is portable C++17. Use the CMake instructions in the README where CMake is installed, or run the MinGW build tasks included in `.vscode/tasks.json`.

The project has three source layers:

| Location | Responsibility |
| --- | --- |
| `include/issueflow/domain.hpp`, `src/domain.cpp` | Data model, CSV parser, validation, assignment exporters, dependency analysis, and the Issue-ID tree index. |
| `include/issueflow/algorithms.hpp`, `src/algorithms.cpp` | Manually implemented algorithms and benchmark runner. |
| `src/main.cpp` | Menu-driven CLI, user prompts, output formatting, and export selection. |

## Public domain API

| API | Purpose |
| --- | --- |
| `loadRepositoryCsv(developersPath, issuesPath)` | Reads validated header-based local CSV files. |
| `validateRepository(data)` | Checks IDs, effort, priorities, capacities, dependencies, and cycles. |
| `analyzeDependencies(issues)` | Returns dependency order or cycle nodes. |
| `IssueIdIndex(issues).find(id)` | Uses the manually implemented binary-search-tree index. |
| `writeAssignmentsCsv(plan, path)` | Exports assignment plans in CSV. |
| `writeAssignmentsJson(plan, path)` | Exports assignment plans in JSON. |

## Issue CSV schema

```text
id,title,description,priority,effort,module,skills,dependencies,labels
```

- `priority`: integer from 1 to 10.
- `effort`: positive integer capacity units.
- `skills`, `dependencies`, and `labels`: `|`-separated lists; use `-` when there are none.
- Fields containing commas must use standard CSV double quotes.

## Extending the product

1. Add a field to `Issue` or `Developer` in `domain.hpp`.
2. Update the CSV schema and parser validation in `domain.cpp`.
3. Update the sample dataset and one invalid/boundary fixture when applicable.
4. Add unit/integration coverage in `tests/test_issueflow.cpp`.
5. Update only the relevant documentation table or user instruction.

Keep algorithm implementations manual. Do not replace a core assessed algorithm with a standard-library algorithm. Non-core utilities such as file output may use normal C++ library facilities.
