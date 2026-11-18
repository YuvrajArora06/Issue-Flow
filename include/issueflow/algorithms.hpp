#pragma once

#include "issueflow/domain.hpp"

#include <cstdint>
#include <functional>
#include <string>
#include <utility>
#include <vector>

namespace issueflow {

enum class SortMethod { Merge, Quick };
enum class SearchMethod { IterativeBinary, RecursiveBinary };
enum class TextSearchMethod { Naive, RabinKarp, FiniteAutomaton, Kmp };
enum class BranchBoundStrategy { Fifo, Lifo, LeastCost };

struct AlgorithmStats {
    std::uint64_t comparisons{};
    std::uint64_t operations{};
    std::uint64_t auxiliaryBytes{};
    double milliseconds{};
};

struct BenchmarkRow {
    std::string algorithm;
    int inputSize{};
    AlgorithmStats stats;
    std::string quality;
};

std::string toString(SortMethod method);
std::string toString(TextSearchMethod method);

std::vector<Issue> prioritizeIssues(const std::vector<Issue>& issues,
                                    SortMethod method,
                                    AlgorithmStats* stats = nullptr);
int findIssueById(const std::vector<Issue>& sortedById, const std::string& id,
                  SearchMethod method, AlgorithmStats* stats = nullptr);

std::vector<int> findTextMatches(const std::string& text, const std::string& pattern,
                                 TextSearchMethod method,
                                 AlgorithmStats* stats = nullptr);

// Baseline one-to-one assignment. An edge exists only when skills and capacity fit.
AssignmentPlan maximumBipartiteMatching(const std::vector<Developer>& developers,
                                        const std::vector<Issue>& issues,
                                        AlgorithmStats* stats = nullptr);

// Capacity-aware heuristic. A developer can receive multiple compatible issues.
AssignmentPlan greedyCapacityAllocation(const std::vector<Developer>& developers,
                                        const std::vector<Issue>& issues,
                                        AlgorithmStats* stats = nullptr);

// Dependency-aware release ordering: every dependency appears before its dependent issue.
DependencyResult dependencyAwareOrder(const std::vector<Issue>& issues);

// Dynamic-programming workload selection for a single developer capacity.
std::vector<std::string> knapsackWorkload(const std::vector<Issue>& issues,
                                          int capacity,
                                          AlgorithmStats* stats = nullptr);

// Divide-and-conquer utilities mapped to release-risk data.
int maximumSubarraySum(const std::vector<int>& values, AlgorithmStats* stats = nullptr);
std::int64_t fastPower(std::int64_t base, std::int64_t exponent,
                       AlgorithmStats* stats = nullptr);
int findCelebrity(const std::vector<std::vector<int>>& knows,
                  AlgorithmStats* stats = nullptr);
using Matrix = std::vector<std::vector<long long>>;
Matrix classicalMatrixMultiply(const Matrix& a, const Matrix& b,
                               AlgorithmStats* stats = nullptr);
Matrix strassenMatrixMultiply(const Matrix& a, const Matrix& b,
                              AlgorithmStats* stats = nullptr);

// Greedy algorithms.
struct Interval { int start{}; int finish{}; std::string id; };
std::vector<Interval> activitySelection(std::vector<Interval> intervals,
                                        AlgorithmStats* stats = nullptr);
int minimumIntervalPartitions(std::vector<Interval> intervals,
                              AlgorithmStats* stats = nullptr);
struct HuffmanCode { char symbol{}; std::string code; };
std::vector<HuffmanCode> huffmanCodes(const std::string& text,
                                      AlgorithmStats* stats = nullptr);
std::vector<long long> dijkstra(const std::vector<std::vector<long long>>& graph, int source,
                                AlgorithmStats* stats = nullptr);
long long primMstWeight(const std::vector<std::vector<long long>>& graph,
                        AlgorithmStats* stats = nullptr);
long long kruskalMstWeight(const std::vector<std::vector<long long>>& graph,
                           AlgorithmStats* stats = nullptr);

// Dynamic programming and graph analysis.
long long matrixChainCost(const std::vector<int>& dimensions,
                          AlgorithmStats* stats = nullptr);
int lcsLength(const std::string& left, const std::string& right,
              AlgorithmStats* stats = nullptr);
double optimalBstCost(const std::vector<double>& frequencies,
                      AlgorithmStats* stats = nullptr);
std::vector<long long> bellmanFord(const std::vector<std::vector<long long>>& graph, int source,
                                   bool* hasNegativeCycle = nullptr,
                                   AlgorithmStats* stats = nullptr);
std::vector<std::vector<long long>> floydWarshall(std::vector<std::vector<long long>> graph,
                                                  AlgorithmStats* stats = nullptr);
long long boundedTsp(const std::vector<std::vector<long long>>& graph,
                     AlgorithmStats* stats = nullptr);

// Backtracking, branch-and-bound, flow and number theory.
int countNQueensSolutions(int n, AlgorithmStats* stats = nullptr);
bool subsetSumExists(const std::vector<int>& values, int target,
                     AlgorithmStats* stats = nullptr);
int branchBoundKnapsack(const std::vector<int>& values, const std::vector<int>& weights,
                        int capacity, BranchBoundStrategy strategy,
                        AlgorithmStats* stats = nullptr);
int fordFulkersonMaxFlow(std::vector<std::vector<int>> capacity, int source, int sink,
                         AlgorithmStats* stats = nullptr);
long long chineseRemainder(const std::vector<long long>& remainders,
                           const std::vector<long long>& moduli);

std::vector<BenchmarkRow> runCoreBenchmarks(const std::vector<int>& sizes);

}  // namespace issueflow
