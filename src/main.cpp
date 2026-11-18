#include "issueflow/algorithms.hpp"
#include "issueflow/domain.hpp"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>

namespace {

using issueflow::AlgorithmStats;
using issueflow::AssignmentPlan;
using issueflow::Developer;
using issueflow::Issue;
using issueflow::RepositoryData;

std::string readLine(const std::string& prompt) {
    std::cout << prompt;
    std::string value;
    std::getline(std::cin, value);
    return value;
}

int readChoice(const std::string& prompt, int minimum, int maximum) {
    while (true) {
        const std::string text = readLine(prompt);
        std::stringstream parser(text);
        int value{};
        char trailing{};
        if ((parser >> value) && !(parser >> trailing) && value >= minimum && value <= maximum) return value;
        std::cout << "Enter a number from " << minimum << " to " << maximum << ".\n";
    }
}

void printStats(const AlgorithmStats& stats) {
    std::cout << "  time: " << std::fixed << std::setprecision(3) << stats.milliseconds << " ms"
              << " | comparisons: " << stats.comparisons
              << " | operations: " << stats.operations
              << " | auxiliary memory estimate: " << stats.auxiliaryBytes << " bytes\n";
}

std::string displayList(const std::vector<std::string>& values) {
    if (values.empty()) {
        return "-";
    }
    std::string result;
    for (std::size_t index = 0; index < values.size(); ++index) {
        result += (index == 0 ? "" : ", ") + values[index];
    }
    return result;
}

void printIssue(const Issue& issue) {
    std::cout << "  " << issue.id << " | P" << issue.priority << " | effort " << issue.effort
              << " | " << issue.module << " | labels: " << displayList(issue.labels)
              << " | " << issue.title << '\n';
}

void showRepository(const RepositoryData& data) {
    std::cout << "\nDevelopers (" << data.developers.size() << ")\n";
    for (const Developer& developer : data.developers) {
        std::cout << "  " << developer.id << " | " << developer.name << " | capacity " << developer.capacity << " | skills: ";
        for (std::size_t i = 0; i < developer.skills.size(); ++i) std::cout << (i ? ", " : "") << developer.skills[i];
        std::cout << '\n';
    }
    std::cout << "\nIssues (" << data.issues.size() << ")\n";
    for (const Issue& issue : data.issues) printIssue(issue);
}

void showValidation(const RepositoryData& data) {
    const issueflow::ValidationResult validation = issueflow::validateRepository(data);
    if (validation.valid) std::cout << "Repository data is valid.\n";
    else {
        std::cout << "Repository data is invalid:\n";
        for (const std::string& error : validation.errors) std::cout << "  ERROR: " << error << '\n';
    }
    for (const std::string& warning : validation.warnings) std::cout << "  WARNING: " << warning << '\n';
}

void showPlan(const AssignmentPlan& plan) {
    std::cout << "Assignments (" << plan.assignments.size() << ")\n";
    for (const issueflow::Assignment& assignment : plan.assignments) {
        std::cout << "  " << assignment.developerId << " -> " << assignment.issueId
                  << " (effort " << assignment.effort << ")\n";
    }
    std::cout << "Remaining capacity\n";
    for (const auto& entry : plan.remainingCapacity) std::cout << "  " << entry.first << ": " << entry.second << '\n';
    if (!plan.unmatchedIssueIds.empty()) {
        std::cout << "Unresolved issues: ";
        for (std::size_t i = 0; i < plan.unmatchedIssueIds.size(); ++i) std::cout << (i ? ", " : "") << plan.unmatchedIssueIds[i];
        std::cout << '\n';
    }
}

std::vector<Issue> sortById(std::vector<Issue> issues) {
    for (int i = 1; i < static_cast<int>(issues.size()); ++i) {
        Issue item = issues[i];
        int j = i - 1;
        while (j >= 0 && issues[j].id > item.id) {
            issues[j + 1] = issues[j];
            --j;
        }
        issues[j + 1] = item;
    }
    return issues;
}

void comparePrioritization(const RepositoryData& data) {
    AlgorithmStats mergeStats, quickStats;
    const auto merge = issueflow::prioritizeIssues(data.issues, issueflow::SortMethod::Merge, &mergeStats);
    const auto quick = issueflow::prioritizeIssues(data.issues, issueflow::SortMethod::Quick, &quickStats);
    std::cout << "\nMerge-sort priority order\n";
    for (const Issue& issue : merge) printIssue(issue);
    printStats(mergeStats);
    std::cout << "Quick sort comparison\n";
    printStats(quickStats);
    bool same = merge.size() == quick.size();
    for (std::size_t i = 0; same && i < merge.size(); ++i) if (merge[i].id != quick[i].id) same = false;
    std::cout << "Order agreement: " << (same ? "yes" : "no (tie ordering can differ)") << "\n";
}

void searchIssue(const RepositoryData& data) {
    const std::string id = readLine("Issue ID: ");
    const int method = readChoice("1) Iterative binary  2) Recursive binary  3) Issue-ID tree index: ", 1, 3);
    int index = -1;
    AlgorithmStats stats;
    if (method == 3) {
        const issueflow::IssueIdIndex indexTree(data.issues);
        index = indexTree.find(id);
        std::cout << "Issue-ID tree index contains " << indexTree.size() << " node(s).\n";
        if (index == -1) std::cout << "Issue not found.\n";
        else { std::cout << "Found\n"; printIssue(data.issues[index]); }
        return;
    }
    const auto sorted = sortById(data.issues);
    index = issueflow::findIssueById(sorted, id,
        method == 1 ? issueflow::SearchMethod::IterativeBinary : issueflow::SearchMethod::RecursiveBinary, &stats);
    if (index == -1) std::cout << "Issue not found.\n";
    else { std::cout << "Found\n"; printIssue(sorted[index]); }
    printStats(stats);
}

void searchDescriptions(const RepositoryData& data) {
    const std::string pattern = readLine("Text to search in issue titles and descriptions: ");
    const int choice = readChoice("1) Naive  2) Rabin-Karp  3) Finite automaton  4) KMP: ", 1, 4);
    const auto method = static_cast<issueflow::TextSearchMethod>(choice - 1);
    std::cout << "Matches using " << issueflow::toString(method) << "\n";
    for (const Issue& issue : data.issues) {
        AlgorithmStats stats;
        const std::vector<int> matches = issueflow::findTextMatches(issue.title + " " + issue.description, pattern, method, &stats);
        if (!matches.empty()) std::cout << "  " << issue.id << " : " << matches.size() << " match(es)\n";
    }
}

void showDependencies(const RepositoryData& data) {
    const issueflow::DependencyResult result = issueflow::dependencyAwareOrder(data.issues);
    if (!result.acyclic) {
        std::cout << "Dependency cycle or invalid dependency involving: ";
        for (const std::string& id : result.cycleNodes) std::cout << id << ' ';
        std::cout << '\n';
        return;
    }
    std::cout << "Dependency-aware release order\n  ";
    for (std::size_t i = 0; i < result.order.size(); ++i) std::cout << (i ? " -> " : "") << result.order[i];
    std::cout << '\n';
}

void exportPlan(const AssignmentPlan& plan) {
    const int format = readChoice("Export plan: 0) Skip  1) CSV  2) JSON: ", 0, 2);
    if (format == 0) return;
    const std::string path = readLine(format == 1 ? "CSV export path: " : "JSON export path: ");
    if (path.empty()) return;
    try {
        if (format == 1) {
            issueflow::writeAssignmentsCsv(plan, path);
        } else {
            issueflow::writeAssignmentsJson(plan, path);
        }
        std::cout << "Wrote " << path << "\n";
    } catch (const std::exception& error) {
        std::cout << "Export failed: " << error.what() << '\n';
    }
}

void runAllocation(const RepositoryData& data, bool matching) {
    AlgorithmStats stats;
    const AssignmentPlan plan = matching
        ? issueflow::maximumBipartiteMatching(data.developers, data.issues, &stats)
        : issueflow::greedyCapacityAllocation(data.developers, data.issues, &stats);
    std::cout << (matching ? "Maximum bipartite matching baseline\n" : "Greedy capacity-aware allocation\n");
    showPlan(plan);
    printStats(stats);
    exportPlan(plan);
}

void chooseKnapsack(const RepositoryData& data) {
    const int capacity = readChoice("Capacity in effort units (0-10000): ", 0, 10000);
    AlgorithmStats stats;
    const auto selected = issueflow::knapsackWorkload(data.issues, capacity, &stats);
    std::cout << "0/1 knapsack selected issue IDs: ";
    for (std::size_t i = 0; i < selected.size(); ++i) std::cout << (i ? ", " : "") << selected[i];
    std::cout << '\n';
    printStats(stats);
}

void runBenchmarks() {
    const std::vector<issueflow::BenchmarkRow> rows = issueflow::runCoreBenchmarks({100, 500, 1000});
    std::cout << "\nAlgorithm benchmark results\n";
    std::cout << std::left << std::setw(31) << "Algorithm" << std::setw(10) << "Input" << std::setw(14)
              << "Time (ms)" << std::setw(14) << "Operations" << "Quality\n";
    for (const auto& row : rows) {
        std::cout << std::left << std::setw(31) << row.algorithm << std::setw(10) << row.inputSize
                  << std::setw(14) << std::fixed << std::setprecision(3) << row.stats.milliseconds
                  << std::setw(14) << row.stats.operations << row.quality << '\n';
    }
    const std::string path = readLine("Save benchmark CSV path (leave blank to skip): ");
    if (path.empty()) return;
    std::ofstream output(path);
    if (!output) { std::cout << "Could not write benchmark file.\n"; return; }
    output << "algorithm,input_size,time_ms,comparisons,operations,auxiliary_bytes,quality\n";
    for (const auto& row : rows) output << row.algorithm << ',' << row.inputSize << ',' << row.stats.milliseconds << ','
                                        << row.stats.comparisons << ',' << row.stats.operations << ','
                                        << row.stats.auxiliaryBytes << ',' << row.quality << '\n';
    std::cout << "Wrote " << path << "\n";
}

void runAlgorithmLab() {
    const std::vector<int> risk = {-2, 1, -3, 4, -1, 2, 1, -5, 4};
    const std::vector<std::vector<long long>> graph = {
        {0, 2, 5, 1}, {2, 0, 3, 2}, {5, 3, 0, 3}, {1, 2, 3, 0}
    };
    const std::vector<std::vector<int>> flow = {
        {0, 16, 13, 0, 0, 0}, {0, 0, 10, 12, 0, 0}, {0, 4, 0, 0, 14, 0},
        {0, 0, 9, 0, 0, 20}, {0, 0, 0, 7, 0, 4}, {0, 0, 0, 0, 0, 0}
    };
    std::cout << "\nAlgorithm lab: deterministic, viva-friendly examples\n";
    std::cout << "  Maximum subarray (release-risk delta): " << issueflow::maximumSubarraySum(risk) << "\n";
    std::cout << "  Fast power 3^10: " << issueflow::fastPower(3, 10) << "\n";
    std::cout << "  Celebrity discovery: " << issueflow::findCelebrity({{0,1,1}, {0,0,1}, {0,0,0}}) << "\n";
    const auto classic = issueflow::classicalMatrixMultiply({{1,2}, {3,4}}, {{5,6}, {7,8}});
    const auto strassen = issueflow::strassenMatrixMultiply({{1,2}, {3,4}}, {{5,6}, {7,8}});
    std::cout << "  Matrix multiplication agreement: " << (classic == strassen ? "yes" : "no") << "\n";
    std::cout << "  Activity selection count: " << issueflow::activitySelection({{1,4,"A"}, {3,5,"B"}, {0,6,"C"}, {5,7,"D"}, {8,11,"E"}}).size() << "\n";
    std::cout << "  Minimum interval partitions: " << issueflow::minimumIntervalPartitions({{1,4,"A"}, {2,5,"B"}, {5,7,"C"}}) << "\n";
    std::cout << "  Huffman symbols: " << issueflow::huffmanCodes("issueflow").size() << "\n";
    std::cout << "  Dijkstra distance 0->2: " << issueflow::dijkstra(graph, 0)[2] << "\n";
    std::cout << "  Prim/Kruskal MST: " << issueflow::primMstWeight(graph) << "/" << issueflow::kruskalMstWeight(graph) << "\n";
    std::cout << "  Matrix-chain cost: " << issueflow::matrixChainCost({40,20,30,10,30}) << "\n";
    std::cout << "  LCS length: " << issueflow::lcsLength("AGGTAB", "GXTXAYB") << "\n";
    std::cout << "  Optimal BST cost: " << issueflow::optimalBstCost({0.34,0.08,0.50,0.08}) << "\n";
    std::cout << "  Bellman-Ford distance 0->3: " << issueflow::bellmanFord(graph, 0)[3] << "\n";
    std::cout << "  Floyd-Warshall distance 1->3: " << issueflow::floydWarshall(graph)[1][3] << "\n";
    std::cout << "  Bounded TSP tour cost: " << issueflow::boundedTsp(graph) << "\n";
    std::cout << "  N-Queens solutions for n=4: " << issueflow::countNQueensSolutions(4) << "\n";
    std::cout << "  Subset sum (target 9): " << (issueflow::subsetSumExists({3,34,4,12,5,2}, 9) ? "true" : "false") << "\n";
    std::cout << "  Branch-and-bound knapsack: " << issueflow::branchBoundKnapsack({60,100,120}, {10,20,30}, 50, issueflow::BranchBoundStrategy::LeastCost) << "\n";
    std::cout << "  Ford-Fulkerson max flow: " << issueflow::fordFulkersonMaxFlow(flow, 0, 5) << "\n";
    std::cout << "  Chinese Remainder Theorem: " << issueflow::chineseRemainder({2,3,2}, {3,5,7}) << "\n";
}

void printMenu() {
    std::cout << "\n=== IssueFlow - Offline DAA Workbench ===\n"
              << "1. Show loaded repository\n2. Load developers and issues CSV\n3. Validate repository\n"
              << "4. Compare priority sorting\n5. Find issue by ID (binary or tree index)\n6. Search descriptions\n"
              << "7. Analyze dependencies\n8. Maximum bipartite matching\n9. Greedy capacity allocation\n"
              << "10. 0/1 knapsack workload selection\n11. Run benchmark suite\n12. Run algorithm lab\n0. Exit\n";
}

}  // namespace

int main() {
    RepositoryData data{issueflow::sampleIssues(), issueflow::sampleDevelopers()};
    std::cout << "IssueFlow - Algorithmic Open-Source Issue Triage and Developer Assignment Engine\n"
              << "Loaded the built-in demonstration repository. Use option 2 to load offline CSV files.\n";
    while (std::cin.good()) {
        printMenu();
        const int choice = readChoice("Select an option: ", 0, 12);
        try {
            if (choice == 0) break;
            if (choice == 1) showRepository(data);
            else if (choice == 2) {
                const std::string developers = readLine("Developers CSV path: ");
                const std::string issues = readLine("Issues CSV path: ");
                RepositoryData candidate = issueflow::loadRepositoryCsv(developers, issues);
                const auto validation = issueflow::validateRepository(candidate);
                if (!validation.valid) {
                    std::cout << "Files were not loaded because validation failed:\n";
                    for (const auto& error : validation.errors) std::cout << "  ERROR: " << error << '\n';
                } else {
                    data = candidate;
                    std::cout << "Loaded " << data.developers.size() << " developers and " << data.issues.size() << " issues.\n";
                }
            } else if (choice == 3) showValidation(data);
            else if (choice == 4) comparePrioritization(data);
            else if (choice == 5) searchIssue(data);
            else if (choice == 6) searchDescriptions(data);
            else if (choice == 7) showDependencies(data);
            else if (choice == 8) runAllocation(data, true);
            else if (choice == 9) runAllocation(data, false);
            else if (choice == 10) chooseKnapsack(data);
            else if (choice == 11) runBenchmarks();
            else if (choice == 12) runAlgorithmLab();
        } catch (const std::exception& error) {
            std::cout << "Operation failed: " << error.what() << '\n';
        }
    }
    std::cout << "IssueFlow closed.\n";
    return 0;
}
