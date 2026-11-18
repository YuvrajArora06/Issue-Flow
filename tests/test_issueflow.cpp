#include "issueflow/algorithms.hpp"
#include "issueflow/domain.hpp"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

int failures = 0;

void check(bool condition, const std::string& name) {
    if (!condition) {
        ++failures;
        std::cerr << "FAIL: " << name << '\n';
    }
}

template <typename Callable>
bool throwsInvalidArgument(Callable callable) {
    try {
        callable();
    } catch (const std::invalid_argument&) {
        return true;
    } catch (...) {
        return false;
    }
    return false;
}

template <typename Callable>
bool throwsRuntimeError(Callable callable) {
    try {
        callable();
    } catch (const std::runtime_error&) {
        return true;
    } catch (...) {
        return false;
    }
    return false;
}

std::vector<std::string> binaryWords(int maximumLength) {
    std::vector<std::string> words = {""};
    for (int length = 1; length <= maximumLength; ++length) {
        const int count = 1 << length;
        for (int bits = 0; bits < count; ++bits) {
            std::string word(length, 'a');
            for (int index = 0; index < length; ++index) {
                if (bits & (1 << index)) {
                    word[index] = 'b';
                }
            }
            words.push_back(word);
        }
    }
    return words;
}

void testRepositoryWorkflow() {
    const issueflow::RepositoryData data{issueflow::sampleIssues(), issueflow::sampleDevelopers()};
    check(issueflow::validateRepository(data).valid, "sample repository validates");
    const issueflow::RepositoryData loaded = issueflow::loadRepositoryCsv(
        "data/developers.csv", "data/issues.csv");
    check(loaded.developers.size() == 3 && loaded.issues.size() == 5 &&
          issueflow::validateRepository(loaded).valid,
          "CSV loader reads a valid repository");
    check(loaded.issues.front().labels.size() == 2 && loaded.issues.front().labels[0] == "bug",
          "CSV loader reads issue labels");
    check(throwsRuntimeError([] {
              issueflow::loadRepositoryCsv("data/invalid_developers_header.csv", "data/issues.csv");
          }),
          "CSV loader rejects an unexpected header");
    check(throwsInvalidArgument([] { issueflow::knapsackWorkload({{"I", "Bad", "", 1, 0, "", {}, {}, {}}}, 1); }),
          "knapsack rejects zero effort");
    const auto ordered = issueflow::dependencyAwareOrder(data.issues);
    check(ordered.acyclic && ordered.order.size() == data.issues.size(), "dependency order is complete");
    int i1 = -1, i2 = -1, i4 = -1;
    for (int index = 0; index < static_cast<int>(ordered.order.size()); ++index) {
        if (ordered.order[index] == "I1") i1 = index;
        if (ordered.order[index] == "I2") i2 = index;
        if (ordered.order[index] == "I4") i4 = index;
    }
    check(i1 < i2 && i1 < i4, "dependencies precede dependent work");

    const auto merge = issueflow::prioritizeIssues(data.issues, issueflow::SortMethod::Merge);
    const auto quick = issueflow::prioritizeIssues(data.issues, issueflow::SortMethod::Quick);
    check(merge.front().id == "I1" && merge.size() == quick.size(), "priority sort produces expected first issue");
    for (std::size_t i = 0; i < merge.size(); ++i) check(merge[i].id == quick[i].id, "merge and quick agree on unique priorities");

    const auto matching = issueflow::maximumBipartiteMatching(data.developers, data.issues);
    check(matching.assignments.size() == 3, "maximum matching covers three one-to-one assignments");
    const auto greedy = issueflow::greedyCapacityAllocation(data.developers, data.issues);
    check(greedy.assignments.size() == 4, "capacity allocation covers four feasible issues");
    for (const auto& entry : greedy.remainingCapacity) check(entry.second >= 0, "allocation never exceeds capacity");

    const auto selected = issueflow::knapsackWorkload(data.issues, 8);
    check(!selected.empty(), "knapsack selects a feasible workload");

    const issueflow::IssueIdIndex issueIndex(data.issues);
    check(issueIndex.size() == data.issues.size() && issueIndex.find("I4") == 3 &&
          issueIndex.find("does-not-exist") == -1,
          "Issue-ID tree index retrieves the original issue position");

    const issueflow::AssignmentPlan jsonPlan = issueflow::maximumBipartiteMatching(data.developers, data.issues);
    issueflow::writeAssignmentsJson(jsonPlan, "build/test_assignments.json");
    std::ifstream jsonFile("build/test_assignments.json");
    std::string json((std::istreambuf_iterator<char>(jsonFile)), std::istreambuf_iterator<char>());
    check(json.find("\"assignments\"") != std::string::npos && json.find("\"remaining_capacity\"") != std::string::npos,
          "assignment JSON export contains required sections");

    issueflow::RepositoryData cyclic = data;
    cyclic.issues[0].dependencies.push_back("I2");
    check(!issueflow::validateRepository(cyclic).valid, "cyclic dependencies are rejected");
}

void testSearching() {
    const std::string text = "ababa";
    const std::vector<int> expected = {0, 2};
    for (const auto method : {issueflow::TextSearchMethod::Naive, issueflow::TextSearchMethod::RabinKarp,
                              issueflow::TextSearchMethod::FiniteAutomaton, issueflow::TextSearchMethod::Kmp}) {
        check(issueflow::findTextMatches(text, "aba", method) == expected,
              "text matcher finds overlapping matches");
    }
    const std::vector<std::string> texts = binaryWords(5);
    const std::vector<std::string> patterns = binaryWords(4);
    for (const std::string& candidateText : texts) {
        for (const std::string& candidatePattern : patterns) {
            const auto expected = issueflow::findTextMatches(
                candidateText, candidatePattern, issueflow::TextSearchMethod::Naive);
            for (const auto method : {issueflow::TextSearchMethod::RabinKarp,
                                      issueflow::TextSearchMethod::FiniteAutomaton,
                                      issueflow::TextSearchMethod::Kmp}) {
                check(issueflow::findTextMatches(candidateText, candidatePattern, method) == expected,
                      "all text matchers agree on exhaustive binary strings");
            }
        }
    }
    std::vector<issueflow::Issue> byId = issueflow::sampleIssues();
    for (int i = 1; i < static_cast<int>(byId.size()); ++i) {
        auto item = byId[i]; int j = i - 1;
        while (j >= 0 && byId[j].id > item.id) { byId[j + 1] = byId[j]; --j; }
        byId[j + 1] = item;
    }
    check(issueflow::findIssueById(byId, "I4", issueflow::SearchMethod::IterativeBinary) >= 0,
          "iterative binary search finds issue");
    check(issueflow::findIssueById(byId, "I4", issueflow::SearchMethod::RecursiveBinary) >= 0,
          "recursive binary search finds issue");
}

void testAlgorithmWorkbench() {
    check(issueflow::maximumSubarraySum({-2,1,-3,4,-1,2,1,-5,4}) == 6, "maximum subarray");
    check(issueflow::fastPower(3, 10) == 59049, "fast power");
    check(issueflow::findCelebrity({{0,1,1}, {0,0,1}, {0,0,0}}) == 2, "celebrity discovery");
    check(issueflow::findCelebrity({}) == -1, "empty celebrity matrix has no celebrity");
    const auto classic = issueflow::classicalMatrixMultiply({{1,2}, {3,4}}, {{5,6}, {7,8}});
    const auto strassen = issueflow::strassenMatrixMultiply({{1,2}, {3,4}}, {{5,6}, {7,8}});
    check(classic == strassen && classic[1][1] == 50, "classical and Strassen multiplication agree");
    check(throwsInvalidArgument([] { issueflow::strassenMatrixMultiply({{1,2}, {3}}, {{1,2}, {3,4}}); }),
          "Strassen rejects jagged matrices");
    check(issueflow::activitySelection({{1,4,"A"}, {3,5,"B"}, {0,6,"C"}, {5,7,"D"}, {8,11,"E"}}).size() == 3,
          "activity selection");
    check(issueflow::minimumIntervalPartitions({{1,4,"A"}, {2,5,"B"}, {5,7,"C"}}) == 2,
          "interval partitioning");
    check(!issueflow::huffmanCodes("issueflow").empty(), "Huffman coding");

    const std::vector<std::vector<long long>> graph = {{0,2,5,1}, {2,0,3,2}, {5,3,0,3}, {1,2,3,0}};
    check(issueflow::dijkstra(graph, 0)[2] == 4, "Dijkstra shortest path");
    check(throwsInvalidArgument([] { issueflow::dijkstra({{0,1}, {1}}, 0); }),
          "Dijkstra rejects jagged graphs");
    check(issueflow::primMstWeight(graph) == 6 && issueflow::kruskalMstWeight(graph) == 6, "MST algorithms agree");
    check(throwsInvalidArgument([] { issueflow::primMstWeight({{0,1}, {2,0}}); }),
          "Prim rejects directed graphs");
    check(issueflow::matrixChainCost({40,20,30,10,30}) == 26000, "matrix-chain multiplication");
    check(throwsInvalidArgument([] { issueflow::matrixChainCost({10,0,30}); }),
          "matrix-chain rejects non-positive dimensions");
    check(issueflow::lcsLength("AGGTAB", "GXTXAYB") == 4, "LCS");
    check(issueflow::optimalBstCost({0.34,0.08,0.50,0.08}) > 0.0, "optimal BST");
    check(throwsInvalidArgument([] { issueflow::optimalBstCost({0.5,-0.1}); }),
          "optimal BST rejects negative frequencies");
    bool negativeCycle = true;
    check(issueflow::bellmanFord(graph, 0, &negativeCycle)[3] == 1 && !negativeCycle, "Bellman-Ford");
    check(issueflow::floydWarshall(graph)[1][3] == 2, "Floyd-Warshall");
    check(issueflow::boundedTsp(graph) == 9, "bounded TSP");
    check(throwsInvalidArgument([] { issueflow::boundedTsp({{0,1}, {1}}); }),
          "bounded TSP rejects jagged graphs");
    check(issueflow::countNQueensSolutions(4) == 2, "N-Queens");
    check(issueflow::subsetSumExists({3,34,4,12,5,2}, 9), "subset sum");
    check(throwsInvalidArgument([] { issueflow::subsetSumExists({3,-1}, 2); }),
          "subset sum rejects negative values");
    for (const auto strategy : {issueflow::BranchBoundStrategy::Fifo, issueflow::BranchBoundStrategy::Lifo,
                                issueflow::BranchBoundStrategy::LeastCost}) {
        check(issueflow::branchBoundKnapsack({60,100,120}, {10,20,30}, 50, strategy) == 220,
              "branch-and-bound knapsack");
    }
    const std::vector<std::vector<int>> flow = {{0,16,13,0,0,0}, {0,0,10,12,0,0}, {0,4,0,0,14,0},
                                                 {0,0,9,0,0,20}, {0,0,0,7,0,4}, {0,0,0,0,0,0}};
    check(issueflow::fordFulkersonMaxFlow(flow, 0, 5) == 23, "Ford-Fulkerson");
    check(throwsInvalidArgument([] { issueflow::fordFulkersonMaxFlow({{0,1}, {0,0}}, 0, 0); }),
          "Ford-Fulkerson rejects identical source and sink");
    check(throwsInvalidArgument([] { issueflow::fordFulkersonMaxFlow({{0,1}, {0}}, 0, 1); }),
          "Ford-Fulkerson rejects jagged matrices");
    check(issueflow::chineseRemainder({2,3,2}, {3,5,7}) == 23, "Chinese Remainder Theorem");

    const auto benchmarks = issueflow::runCoreBenchmarks({20});
    check(benchmarks.size() == 6 && benchmarks.back().algorithm == "Quick sort (sorted input)",
          "benchmark runner includes the adversarial quick-sort case");
}

}  // namespace

int main() {
    testRepositoryWorkflow();
    testSearching();
    testAlgorithmWorkbench();
    if (failures != 0) {
        std::cerr << failures << " test assertion(s) failed.\n";
        return EXIT_FAILURE;
    }
    std::cout << "All IssueFlow tests passed.\n";
    return EXIT_SUCCESS;
}
