# Dataset Provenance

All rows in this directory are synthetic course-work records created for IssueFlow. They do not originate from GitHub, a company repository, or personal developer data.

| File | Purpose | Provenance |
| --- | --- | --- |
| `developers.csv` | Normal-operation developers | Synthetic names, skills, and capacities. |
| `issues.csv` | Normal-operation issues, labels, skills, and dependencies | Synthetic repository scenario. |
| `capacity_shock_developers.csv` | Capacity-shock scenario | Reduced capacities derived from the normal synthetic developer scenario. |
| `invalid_cycle_issues.csv` | Invalid-input regression case | Synthetic two-node cycle. |
| `invalid_developers_header.csv` | CSV-schema regression case | Synthetic malformed header. |

The generated benchmark datasets are produced deterministically by `generateSyntheticRepository` in `src/domain.cpp`. The sorted adversarial benchmark input is produced by `makeSortedPriorityInput` in `src/algorithms.cpp`.

No third-party dataset licence is required because no third-party records are included. If real repository data is added later, record its source, licence, collection date, and privacy restrictions here before submission.
