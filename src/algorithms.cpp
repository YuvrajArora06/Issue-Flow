#include "issueflow/algorithms.hpp"

#include <chrono>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <unordered_map>

namespace issueflow {
namespace {

constexpr long long INF = std::numeric_limits<long long>::max() / 4;

bool priorityBefore(const Issue& left, const Issue& right, AlgorithmStats& stats) {
    ++stats.comparisons;
    if (left.priority != right.priority) {
        return left.priority > right.priority;
    }
    ++stats.comparisons;
    if (left.effort != right.effort) {
        return left.effort < right.effort;
    }
    ++stats.comparisons;
    return left.id < right.id;
}

std::string paddedIssueId(int number) {
    std::string value = std::to_string(number);
    while (value.size() < 8) {
        value = "0" + value;
    }
    return "S" + value;
}

std::vector<Issue> makeSortedPriorityInput(int issueCount) {
    std::vector<Issue> issues;
    for (int index = 0; index < issueCount; ++index) {
        issues.push_back({paddedIssueId(index + 1), "Sorted benchmark issue", "Adversarial pivot-order input.",
                          10, 1, "Benchmark", {"Backend"}, {}, {"synthetic", "adversarial"}});
    }
    return issues;
}

void mergeRange(std::vector<Issue>& values, std::vector<Issue>& buffer, int left, int middle, int right,
                AlgorithmStats& stats) {
    int i = left;
    int j = middle + 1;
    int k = left;
    while (i <= middle && j <= right) {
        if (priorityBefore(values[i], values[j], stats)) {
            buffer[k++] = values[i++];
        } else {
            buffer[k++] = values[j++];
        }
        ++stats.operations;
    }
    while (i <= middle) {
        buffer[k++] = values[i++];
        ++stats.operations;
    }
    while (j <= right) {
        buffer[k++] = values[j++];
        ++stats.operations;
    }
    for (i = left; i <= right; ++i) {
        values[i] = buffer[i];
        ++stats.operations;
    }
}

void mergeSort(std::vector<Issue>& values, std::vector<Issue>& buffer, int left, int right,
               AlgorithmStats& stats) {
    if (left >= right) {
        return;
    }
    const int middle = left + (right - left) / 2;
    mergeSort(values, buffer, left, middle, stats);
    mergeSort(values, buffer, middle + 1, right, stats);
    mergeRange(values, buffer, left, middle, right, stats);
}

int partition(std::vector<Issue>& values, int left, int right, AlgorithmStats& stats) {
    const Issue pivot = values[right];
    int store = left;
    for (int i = left; i < right; ++i) {
        if (priorityBefore(values[i], pivot, stats)) {
            const Issue temporary = values[i];
            values[i] = values[store];
            values[store] = temporary;
            ++store;
            stats.operations += 3;
        }
    }
    const Issue temporary = values[store];
    values[store] = values[right];
    values[right] = temporary;
    stats.operations += 3;
    return store;
}

void quickSort(std::vector<Issue>& values, int left, int right, AlgorithmStats& stats) {
    if (left >= right) {
        return;
    }
    const int pivot = partition(values, left, right, stats);
    quickSort(values, left, pivot - 1, stats);
    quickSort(values, pivot + 1, right, stats);
}

int recursiveBinarySearch(const std::vector<Issue>& values, const std::string& id, int left, int right,
                          AlgorithmStats& stats) {
    if (left > right) {
        return -1;
    }
    const int middle = left + (right - left) / 2;
    ++stats.comparisons;
    if (values[middle].id == id) {
        return middle;
    }
    ++stats.comparisons;
    if (values[middle].id < id) {
        return recursiveBinarySearch(values, id, middle + 1, right, stats);
    }
    return recursiveBinarySearch(values, id, left, middle - 1, stats);
}

bool augmentIssue(int issueIndex, const std::vector<Developer>& developers,
                  const std::vector<Issue>& issues, std::vector<int>& developerMatch,
                  std::vector<bool>& seen, AlgorithmStats& stats) {
    for (int developer = 0; developer < static_cast<int>(developers.size()); ++developer) {
        ++stats.comparisons;
        if (seen[developer] || !developerMatchesIssue(developers[developer], issues[issueIndex])) {
            continue;
        }
        seen[developer] = true;
        ++stats.operations;
        if (developerMatch[developer] == -1 ||
            augmentIssue(developerMatch[developer], developers, issues, developerMatch, seen, stats)) {
            developerMatch[developer] = issueIndex;
            return true;
        }
    }
    return false;
}

int crossingMaximum(const std::vector<int>& values, int left, int middle, int right, AlgorithmStats& stats) {
    int sum = 0;
    int leftBest = std::numeric_limits<int>::min();
    for (int i = middle; i >= left; --i) {
        sum += values[i];
        ++stats.operations;
        if (sum > leftBest) {
            leftBest = sum;
        }
    }
    sum = 0;
    int rightBest = std::numeric_limits<int>::min();
    for (int i = middle + 1; i <= right; ++i) {
        sum += values[i];
        ++stats.operations;
        if (sum > rightBest) {
            rightBest = sum;
        }
    }
    return leftBest + rightBest;
}

int maximumSubarrayRecursive(const std::vector<int>& values, int left, int right, AlgorithmStats& stats) {
    if (left == right) {
        return values[left];
    }
    const int middle = left + (right - left) / 2;
    const int first = maximumSubarrayRecursive(values, left, middle, stats);
    const int second = maximumSubarrayRecursive(values, middle + 1, right, stats);
    const int cross = crossingMaximum(values, left, middle, right, stats);
    int best = first > second ? first : second;
    return best > cross ? best : cross;
}

Matrix makeMatrix(int n, long long value = 0) { return Matrix(n, std::vector<long long>(n, value)); }

Matrix addMatrix(const Matrix& a, const Matrix& b, int sign, AlgorithmStats& stats) {
    const int n = static_cast<int>(a.size());
    Matrix result = makeMatrix(n);
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            result[i][j] = a[i][j] + sign * b[i][j];
            ++stats.operations;
        }
    }
    return result;
}

Matrix sliceMatrix(const Matrix& source, int row, int column, int size) {
    Matrix result = makeMatrix(size);
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            result[i][j] = source[row + i][column + j];
        }
    }
    return result;
}

Matrix strassenRecursive(const Matrix& a, const Matrix& b, AlgorithmStats& stats) {
    const int n = static_cast<int>(a.size());
    if (n <= 2) {
        Matrix result = makeMatrix(n);
        for (int i = 0; i < n; ++i) {
            for (int k = 0; k < n; ++k) {
                for (int j = 0; j < n; ++j) {
                    result[i][j] += a[i][k] * b[k][j];
                    ++stats.operations;
                }
            }
        }
        return result;
    }
    const int half = n / 2;
    const Matrix a11 = sliceMatrix(a, 0, 0, half);
    const Matrix a12 = sliceMatrix(a, 0, half, half);
    const Matrix a21 = sliceMatrix(a, half, 0, half);
    const Matrix a22 = sliceMatrix(a, half, half, half);
    const Matrix b11 = sliceMatrix(b, 0, 0, half);
    const Matrix b12 = sliceMatrix(b, 0, half, half);
    const Matrix b21 = sliceMatrix(b, half, 0, half);
    const Matrix b22 = sliceMatrix(b, half, half, half);

    const Matrix m1 = strassenRecursive(addMatrix(a11, a22, 1, stats), addMatrix(b11, b22, 1, stats), stats);
    const Matrix m2 = strassenRecursive(addMatrix(a21, a22, 1, stats), b11, stats);
    const Matrix m3 = strassenRecursive(a11, addMatrix(b12, b22, -1, stats), stats);
    const Matrix m4 = strassenRecursive(a22, addMatrix(b21, b11, -1, stats), stats);
    const Matrix m5 = strassenRecursive(addMatrix(a11, a12, 1, stats), b22, stats);
    const Matrix m6 = strassenRecursive(addMatrix(a21, a11, -1, stats), addMatrix(b11, b12, 1, stats), stats);
    const Matrix m7 = strassenRecursive(addMatrix(a12, a22, -1, stats), addMatrix(b21, b22, 1, stats), stats);

    const Matrix c11 = addMatrix(addMatrix(addMatrix(m1, m4, 1, stats), m5, -1, stats), m7, 1, stats);
    const Matrix c12 = addMatrix(m3, m5, 1, stats);
    const Matrix c21 = addMatrix(m2, m4, 1, stats);
    const Matrix c22 = addMatrix(addMatrix(addMatrix(m1, m2, -1, stats), m3, 1, stats), m6, 1, stats);
    Matrix result = makeMatrix(n);
    for (int i = 0; i < half; ++i) {
        for (int j = 0; j < half; ++j) {
            result[i][j] = c11[i][j];
            result[i][j + half] = c12[i][j];
            result[i + half][j] = c21[i][j];
            result[i + half][j + half] = c22[i][j];
        }
    }
    return result;
}

int chooseMinimumNode(const std::vector<int>& active, const std::vector<long long>& weights,
                      int excluded = -1) {
    int result = -1;
    for (const int index : active) {
        if (index == excluded) {
            continue;
        }
        if (result == -1 || weights[index] < weights[result]) {
            result = index;
        }
    }
    return result;
}

void collectHuffmanCodes(int index, const std::vector<int>& left, const std::vector<int>& right,
                         const std::vector<char>& symbols, const std::string& prefix,
                         std::vector<HuffmanCode>& output) {
    if (left[index] == -1 && right[index] == -1) {
        output.push_back({symbols[index], prefix.empty() ? "0" : prefix});
        return;
    }
    collectHuffmanCodes(left[index], left, right, symbols, prefix + "0", output);
    collectHuffmanCodes(right[index], left, right, symbols, prefix + "1", output);
}

class DisjointSet {
public:
    explicit DisjointSet(int size) : parent_(size), rank_(size, 0) {
        for (int i = 0; i < size; ++i) parent_[i] = i;
    }
    int find(int value) {
        if (parent_[value] != value) parent_[value] = find(parent_[value]);
        return parent_[value];
    }
    bool unite(int first, int second) {
        first = find(first);
        second = find(second);
        if (first == second) return false;
        if (rank_[first] < rank_[second]) {
            parent_[first] = second;
        } else if (rank_[first] > rank_[second]) {
            parent_[second] = first;
        } else {
            parent_[second] = first;
            ++rank_[first];
        }
        return true;
    }
private:
    std::vector<int> parent_;
    std::vector<int> rank_;
};

void solveQueens(int row, int n, std::vector<bool>& columns, std::vector<bool>& firstDiagonal,
                 std::vector<bool>& secondDiagonal, int& total, AlgorithmStats& stats) {
    if (row == n) {
        ++total;
        return;
    }
    for (int column = 0; column < n; ++column) {
        const int first = row - column + n - 1;
        const int second = row + column;
        ++stats.comparisons;
        if (columns[column] || firstDiagonal[first] || secondDiagonal[second]) continue;
        columns[column] = firstDiagonal[first] = secondDiagonal[second] = true;
        ++stats.operations;
        solveQueens(row + 1, n, columns, firstDiagonal, secondDiagonal, total, stats);
        columns[column] = firstDiagonal[first] = secondDiagonal[second] = false;
    }
}

bool subsetSumRecursive(const std::vector<int>& values, int index, int remaining, AlgorithmStats& stats) {
    ++stats.operations;
    if (remaining == 0) return true;
    if (index == static_cast<int>(values.size()) || remaining < 0) return false;
    return subsetSumRecursive(values, index + 1, remaining - values[index], stats) ||
           subsetSumRecursive(values, index + 1, remaining, stats);
}

struct KnapsackItem { int value{}; int weight{}; };
struct KnapsackNode { int level{-1}; int value{}; int weight{}; double bound{}; };

double knapsackBound(const KnapsackNode& node, const std::vector<KnapsackItem>& items, int capacity) {
    if (node.weight >= capacity) return 0.0;
    double result = node.value;
    int totalWeight = node.weight;
    int index = node.level + 1;
    while (index < static_cast<int>(items.size()) && totalWeight + items[index].weight <= capacity) {
        totalWeight += items[index].weight;
        result += items[index].value;
        ++index;
    }
    if (index < static_cast<int>(items.size())) {
        result += static_cast<double>(capacity - totalWeight) * items[index].value / items[index].weight;
    }
    return result;
}

long long extendedGcd(long long a, long long b, long long& x, long long& y) {
    if (b == 0) {
        x = 1;
        y = 0;
        return a;
    }
    long long nextX{}, nextY{};
    const long long gcd = extendedGcd(b, a % b, nextX, nextY);
    x = nextY;
    y = nextX - (a / b) * nextY;
    return gcd;
}

void requireSquareGraph(const std::vector<std::vector<long long>>& graph,
                        const std::string& algorithm) {
    const std::size_t size = graph.size();
    for (const auto& row : graph) {
        if (row.size() != size) {
            throw std::invalid_argument(algorithm + " requires a square adjacency matrix");
        }
    }
}

void requireUndirectedGraph(const std::vector<std::vector<long long>>& graph,
                            const std::string& algorithm) {
    requireSquareGraph(graph, algorithm);
    for (std::size_t from = 0; from < graph.size(); ++from) {
        for (std::size_t to = from + 1; to < graph.size(); ++to) {
            if (graph[from][to] != graph[to][from]) {
                throw std::invalid_argument(algorithm + " requires a symmetric adjacency matrix");
            }
            if (graph[from][to] < 0) {
                throw std::invalid_argument(algorithm + " requires non-negative edge weights");
            }
        }
    }
}

void requireSquareCapacityMatrix(const std::vector<std::vector<int>>& capacity) {
    const std::size_t size = capacity.size();
    for (const auto& row : capacity) {
        if (row.size() != size) {
            throw std::invalid_argument("Ford-Fulkerson requires a square capacity matrix");
        }
        for (const int edgeCapacity : row) {
            if (edgeCapacity < 0) {
                throw std::invalid_argument("Ford-Fulkerson capacities cannot be negative");
            }
        }
    }
}

template <typename Operation>
AlgorithmStats measureRepeated(Operation operation, int repetitions) {
    AlgorithmStats representative;
    const auto started = std::chrono::steady_clock::now();
    for (int iteration = 0; iteration < repetitions; ++iteration) {
        AlgorithmStats current;
        operation(&current);
        if (iteration == 0) {
            representative = current;
        }
    }
    const auto ended = std::chrono::steady_clock::now();
    representative.milliseconds =
        std::chrono::duration<double, std::milli>(ended - started).count() / repetitions;
    return representative;
}

}  // namespace

std::string toString(SortMethod method) { return method == SortMethod::Merge ? "Merge sort" : "Quick sort"; }

std::string toString(TextSearchMethod method) {
    switch (method) {
        case TextSearchMethod::Naive: return "Naive";
        case TextSearchMethod::RabinKarp: return "Rabin-Karp";
        case TextSearchMethod::FiniteAutomaton: return "Finite automaton";
        case TextSearchMethod::Kmp: return "KMP";
    }
    return "Unknown";
}

std::vector<Issue> prioritizeIssues(const std::vector<Issue>& issues, SortMethod method, AlgorithmStats* output) {
    AlgorithmStats stats;
    const auto started = std::chrono::steady_clock::now();
    std::vector<Issue> result = issues;
    if (method == SortMethod::Merge && result.size() > 1) {
        std::vector<Issue> buffer(result.size());
        stats.auxiliaryBytes = buffer.size() * sizeof(Issue);
        mergeSort(result, buffer, 0, static_cast<int>(result.size()) - 1, stats);
    } else if (method == SortMethod::Quick && result.size() > 1) {
        quickSort(result, 0, static_cast<int>(result.size()) - 1, stats);
        stats.auxiliaryBytes = sizeof(Issue) * result.size(); // recursion-stack upper estimate
    }
    const auto ended = std::chrono::steady_clock::now();
    stats.milliseconds = std::chrono::duration<double, std::milli>(ended - started).count();
    if (output) *output = stats;
    return result;
}

int findIssueById(const std::vector<Issue>& sortedById, const std::string& id,
                  SearchMethod method, AlgorithmStats* output) {
    AlgorithmStats stats;
    const auto started = std::chrono::steady_clock::now();
    int result = -1;
    if (method == SearchMethod::RecursiveBinary) {
        result = recursiveBinarySearch(sortedById, id, 0, static_cast<int>(sortedById.size()) - 1, stats);
        stats.auxiliaryBytes = sizeof(int) * sortedById.size();
    } else {
        int left = 0;
        int right = static_cast<int>(sortedById.size()) - 1;
        while (left <= right) {
            const int middle = left + (right - left) / 2;
            ++stats.comparisons;
            if (sortedById[middle].id == id) { result = middle; break; }
            ++stats.comparisons;
            if (sortedById[middle].id < id) left = middle + 1;
            else right = middle - 1;
        }
    }
    const auto ended = std::chrono::steady_clock::now();
    stats.milliseconds = std::chrono::duration<double, std::milli>(ended - started).count();
    if (output) *output = stats;
    return result;
}

std::vector<int> findTextMatches(const std::string& text, const std::string& pattern,
                                 TextSearchMethod method, AlgorithmStats* output) {
    AlgorithmStats stats;
    const auto started = std::chrono::steady_clock::now();
    std::vector<int> matches;
    if (pattern.empty()) {
        for (int i = 0; i <= static_cast<int>(text.size()); ++i) matches.push_back(i);
    } else if (pattern.size() <= text.size()) {
        const int n = static_cast<int>(text.size());
        const int m = static_cast<int>(pattern.size());
        if (method == TextSearchMethod::Naive) {
            for (int start = 0; start <= n - m; ++start) {
                int matched = 0;
                while (matched < m) {
                    ++stats.comparisons;
                    if (text[start + matched] != pattern[matched]) break;
                    ++matched;
                }
                if (matched == m) matches.push_back(start);
            }
        } else if (method == TextSearchMethod::RabinKarp) {
            constexpr long long base = 256;
            constexpr long long modulus = 1000000007;
            long long power = 1, targetHash = 0, windowHash = 0;
            for (int i = 0; i < m - 1; ++i) power = (power * base) % modulus;
            for (int i = 0; i < m; ++i) {
                targetHash = (targetHash * base + static_cast<unsigned char>(pattern[i])) % modulus;
                windowHash = (windowHash * base + static_cast<unsigned char>(text[i])) % modulus;
            }
            for (int start = 0; start <= n - m; ++start) {
                ++stats.comparisons;
                if (targetHash == windowHash) {
                    int matched = 0;
                    while (matched < m && text[start + matched] == pattern[matched]) {
                        ++matched;
                        ++stats.comparisons;
                    }
                    if (matched == m) matches.push_back(start);
                }
                if (start < n - m) {
                    windowHash = (base * (windowHash - static_cast<unsigned char>(text[start]) * power) +
                                  static_cast<unsigned char>(text[start + m])) % modulus;
                    if (windowHash < 0) windowHash += modulus;
                }
            }
        } else if (method == TextSearchMethod::FiniteAutomaton) {
            std::vector<std::vector<int>> transition(m + 1, std::vector<int>(256, 0));
            stats.auxiliaryBytes = (m + 1) * 256 * sizeof(int);
            for (int state = 0; state <= m; ++state) {
                for (int character = 0; character < 256; ++character) {
                    int candidate = (state < m && character == static_cast<unsigned char>(pattern[state]))
                                        ? state + 1 : state;
                    while (candidate > 0) {
                        bool prefix = true;
                        for (int i = 0; i < candidate; ++i) {
                            const int textIndex = state - candidate + 1 + i;
                            char source = i == candidate - 1 ? static_cast<char>(character)
                                                              : (textIndex >= 0 ? pattern[textIndex] : '\0');
                            ++stats.comparisons;
                            if (source != pattern[i]) { prefix = false; break; }
                        }
                        if (prefix) break;
                        --candidate;
                    }
                    transition[state][character] = candidate;
                }
            }
            int state = 0;
            for (int i = 0; i < n; ++i) {
                state = transition[state][static_cast<unsigned char>(text[i])];
                if (state == m) matches.push_back(i - m + 1);
            }
        } else {
            std::vector<int> lps(m, 0);
            stats.auxiliaryBytes = m * sizeof(int);
            for (int i = 1, length = 0; i < m;) {
                ++stats.comparisons;
                if (pattern[i] == pattern[length]) lps[i++] = ++length;
                else if (length != 0) length = lps[length - 1];
                else lps[i++] = 0;
            }
            for (int i = 0, j = 0; i < n;) {
                ++stats.comparisons;
                if (text[i] == pattern[j]) { ++i; ++j; }
                if (j == m) { matches.push_back(i - j); j = lps[j - 1]; }
                else if (i < n && text[i] != pattern[j]) j == 0 ? ++i : j = lps[j - 1];
            }
        }
    }
    const auto ended = std::chrono::steady_clock::now();
    stats.milliseconds = std::chrono::duration<double, std::milli>(ended - started).count();
    if (output) *output = stats;
    return matches;
}

AssignmentPlan maximumBipartiteMatching(const std::vector<Developer>& developers,
                                        const std::vector<Issue>& issues, AlgorithmStats* output) {
    AlgorithmStats stats;
    const auto started = std::chrono::steady_clock::now();
    std::vector<int> developerMatch(developers.size(), -1);
    std::vector<bool> issueMatched(issues.size(), false);
    for (int issue = 0; issue < static_cast<int>(issues.size()); ++issue) {
        std::vector<bool> seen(developers.size(), false);
        stats.auxiliaryBytes = (developers.size() * sizeof(bool)) +
                               (developerMatch.size() * sizeof(int));
        if (augmentIssue(issue, developers, issues, developerMatch, seen, stats)) issueMatched[issue] = true;
    }
    AssignmentPlan plan;
    for (int developer = 0; developer < static_cast<int>(developers.size()); ++developer) {
        plan.remainingCapacity[developers[developer].id] = developers[developer].capacity;
        if (developerMatch[developer] != -1) {
            const Issue& issue = issues[developerMatch[developer]];
            plan.assignments.push_back({developers[developer].id, issue.id, issue.effort});
            plan.remainingCapacity[developers[developer].id] -= issue.effort;
        }
    }
    for (int issue = 0; issue < static_cast<int>(issues.size()); ++issue) {
        bool matched = false;
        for (int developer : developerMatch) if (developer == issue) matched = true;
        if (!matched) plan.unmatchedIssueIds.push_back(issues[issue].id);
    }
    const auto ended = std::chrono::steady_clock::now();
    stats.milliseconds = std::chrono::duration<double, std::milli>(ended - started).count();
    if (output) *output = stats;
    return plan;
}

AssignmentPlan greedyCapacityAllocation(const std::vector<Developer>& developers,
                                        const std::vector<Issue>& issues, AlgorithmStats* output) {
    AlgorithmStats stats;
    const auto started = std::chrono::steady_clock::now();
    AssignmentPlan plan;
    std::vector<Issue> candidates = prioritizeIssues(issues, SortMethod::Merge, &stats);
    for (const Developer& developer : developers) plan.remainingCapacity[developer.id] = developer.capacity;
    for (const Issue& issue : candidates) {
        int chosen = -1;
        for (int developer = 0; developer < static_cast<int>(developers.size()); ++developer) {
            ++stats.comparisons;
            const int remaining = plan.remainingCapacity[developers[developer].id];
            if (remaining < issue.effort) continue;
            bool skillsFit = true;
            for (const std::string& required : issue.requiredSkills) {
                bool found = false;
                for (const std::string& skill : developers[developer].skills) {
                    ++stats.comparisons;
                    if (skill == required) { found = true; break; }
                }
                if (!found) { skillsFit = false; break; }
            }
            if (skillsFit && (chosen == -1 || remaining < plan.remainingCapacity[developers[chosen].id])) {
                chosen = developer;
            }
        }
        if (chosen == -1) {
            plan.unmatchedIssueIds.push_back(issue.id);
        } else {
            plan.assignments.push_back({developers[chosen].id, issue.id, issue.effort});
            plan.remainingCapacity[developers[chosen].id] -= issue.effort;
            ++stats.operations;
        }
    }
    const auto ended = std::chrono::steady_clock::now();
    stats.milliseconds = std::chrono::duration<double, std::milli>(ended - started).count();
    if (output) *output = stats;
    return plan;
}

DependencyResult dependencyAwareOrder(const std::vector<Issue>& issues) { return analyzeDependencies(issues); }

std::vector<std::string> knapsackWorkload(const std::vector<Issue>& issues, int capacity,
                                          AlgorithmStats* output) {
    if (capacity < 0) throw std::invalid_argument("Capacity cannot be negative");
    for (const Issue& issue : issues) {
        if (issue.effort <= 0) {
            throw std::invalid_argument("0/1 knapsack requires positive issue effort");
        }
    }
    AlgorithmStats stats;
    const auto started = std::chrono::steady_clock::now();
    const int n = static_cast<int>(issues.size());
    std::vector<std::vector<int>> table(n + 1, std::vector<int>(capacity + 1, 0));
    stats.auxiliaryBytes = (n + 1) * (capacity + 1) * sizeof(int);
    for (int i = 1; i <= n; ++i) {
        const int value = issues[i - 1].priority;
        const int weight = issues[i - 1].effort;
        for (int current = 0; current <= capacity; ++current) {
            table[i][current] = table[i - 1][current];
            if (weight <= current) {
                const int include = table[i - 1][current - weight] + value;
                if (include > table[i][current]) table[i][current] = include;
            }
            ++stats.operations;
        }
    }
    std::vector<std::string> selected;
    for (int i = n, current = capacity; i > 0; --i) {
        if (table[i][current] != table[i - 1][current]) {
            selected.push_back(issues[i - 1].id);
            current -= issues[i - 1].effort;
        }
    }
    const auto ended = std::chrono::steady_clock::now();
    stats.milliseconds = std::chrono::duration<double, std::milli>(ended - started).count();
    if (output) *output = stats;
    return selected;
}

int maximumSubarraySum(const std::vector<int>& values, AlgorithmStats* output) {
    if (values.empty()) throw std::invalid_argument("Maximum subarray requires at least one value");
    AlgorithmStats stats;
    const auto started = std::chrono::steady_clock::now();
    const int result = maximumSubarrayRecursive(values, 0, static_cast<int>(values.size()) - 1, stats);
    stats.milliseconds = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - started).count();
    if (output) *output = stats;
    return result;
}

std::int64_t fastPower(std::int64_t base, std::int64_t exponent, AlgorithmStats* output) {
    if (exponent < 0) throw std::invalid_argument("Fast power only accepts non-negative exponents");
    AlgorithmStats stats;
    const auto started = std::chrono::steady_clock::now();
    std::int64_t result = 1;
    while (exponent > 0) {
        if (exponent & 1LL) { result *= base; ++stats.operations; }
        base *= base;
        ++stats.operations;
        exponent >>= 1;
    }
    stats.milliseconds = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - started).count();
    if (output) *output = stats;
    return result;
}

int findCelebrity(const std::vector<std::vector<int>>& knows, AlgorithmStats* output) {
    const int n = static_cast<int>(knows.size());
    for (const auto& row : knows) if (static_cast<int>(row.size()) != n) throw std::invalid_argument("Celebrity matrix must be square");
    if (n == 0) {
        if (output) *output = {};
        return -1;
    }
    AlgorithmStats stats;
    const auto started = std::chrono::steady_clock::now();
    int candidate = 0;
    for (int person = 1; person < n; ++person) {
        ++stats.comparisons;
        if (knows[candidate][person]) candidate = person;
    }
    for (int person = 0; person < n; ++person) {
        if (person == candidate) continue;
        stats.comparisons += 2;
        if (knows[candidate][person] || !knows[person][candidate]) {
            if (output) *output = stats;
            return -1;
        }
    }
    stats.milliseconds = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - started).count();
    if (output) *output = stats;
    return candidate;
}

Matrix classicalMatrixMultiply(const Matrix& a, const Matrix& b, AlgorithmStats* output) {
    const int rows = static_cast<int>(a.size());
    const int shared = rows ? static_cast<int>(a[0].size()) : 0;
    if (static_cast<int>(b.size()) != shared) throw std::invalid_argument("Incompatible matrix dimensions");
    const int columns = b.empty() ? 0 : static_cast<int>(b[0].size());
    for (const auto& row : a) if (static_cast<int>(row.size()) != shared) throw std::invalid_argument("Jagged matrix A");
    for (const auto& row : b) if (static_cast<int>(row.size()) != columns) throw std::invalid_argument("Jagged matrix B");
    AlgorithmStats stats;
    const auto started = std::chrono::steady_clock::now();
    Matrix result(rows, std::vector<long long>(columns, 0));
    stats.auxiliaryBytes = rows * columns * sizeof(long long);
    for (int i = 0; i < rows; ++i) for (int k = 0; k < shared; ++k) for (int j = 0; j < columns; ++j) {
        result[i][j] += a[i][k] * b[k][j];
        ++stats.operations;
    }
    stats.milliseconds = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - started).count();
    if (output) *output = stats;
    return result;
}

Matrix strassenMatrixMultiply(const Matrix& a, const Matrix& b, AlgorithmStats* output) {
    const int rows = static_cast<int>(a.size());
    const int shared = rows ? static_cast<int>(a[0].size()) : 0;
    if (static_cast<int>(b.size()) != shared) throw std::invalid_argument("Incompatible matrix dimensions");
    const int columns = b.empty() ? 0 : static_cast<int>(b[0].size());
    for (const auto& row : a) if (static_cast<int>(row.size()) != shared) throw std::invalid_argument("Jagged matrix A");
    for (const auto& row : b) if (static_cast<int>(row.size()) != columns) throw std::invalid_argument("Jagged matrix B");
    int size = 1;
    const int largest = rows > shared ? (rows > columns ? rows : columns) : (shared > columns ? shared : columns);
    while (size < largest) size <<= 1;
    AlgorithmStats stats;
    const auto started = std::chrono::steady_clock::now();
    Matrix first = makeMatrix(size), second = makeMatrix(size);
    for (int i = 0; i < rows; ++i) for (int j = 0; j < shared; ++j) first[i][j] = a[i][j];
    for (int i = 0; i < shared; ++i) for (int j = 0; j < columns; ++j) second[i][j] = b[i][j];
    Matrix paddedResult = strassenRecursive(first, second, stats);
    Matrix result(rows, std::vector<long long>(columns));
    for (int i = 0; i < rows; ++i) for (int j = 0; j < columns; ++j) result[i][j] = paddedResult[i][j];
    stats.auxiliaryBytes = 3ULL * size * size * sizeof(long long);
    stats.milliseconds = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - started).count();
    if (output) *output = stats;
    return result;
}

std::vector<Interval> activitySelection(std::vector<Interval> intervals, AlgorithmStats* output) {
    AlgorithmStats stats;
    const auto started = std::chrono::steady_clock::now();
    for (int i = 1; i < static_cast<int>(intervals.size()); ++i) {
        Interval item = intervals[i]; int j = i - 1;
        while (j >= 0 && intervals[j].finish > item.finish) { intervals[j + 1] = intervals[j]; --j; ++stats.operations; }
        intervals[j + 1] = item;
    }
    std::vector<Interval> selected;
    int lastFinish = std::numeric_limits<int>::min();
    for (const Interval& interval : intervals) {
        ++stats.comparisons;
        if (interval.start >= lastFinish) { selected.push_back(interval); lastFinish = interval.finish; }
    }
    stats.milliseconds = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - started).count();
    if (output) *output = stats;
    return selected;
}

int minimumIntervalPartitions(std::vector<Interval> intervals, AlgorithmStats* output) {
    AlgorithmStats stats;
    const auto started = std::chrono::steady_clock::now();
    for (int i = 1; i < static_cast<int>(intervals.size()); ++i) {
        Interval item = intervals[i]; int j = i - 1;
        while (j >= 0 && intervals[j].start > item.start) { intervals[j + 1] = intervals[j]; --j; ++stats.operations; }
        intervals[j + 1] = item;
    }
    std::vector<int> partitionFinishes;
    for (const Interval& interval : intervals) {
        int selected = -1;
        for (int p = 0; p < static_cast<int>(partitionFinishes.size()); ++p) {
            ++stats.comparisons;
            if (partitionFinishes[p] <= interval.start &&
                (selected == -1 || partitionFinishes[p] < partitionFinishes[selected])) selected = p;
        }
        if (selected == -1) partitionFinishes.push_back(interval.finish);
        else partitionFinishes[selected] = interval.finish;
    }
    stats.milliseconds = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - started).count();
    if (output) *output = stats;
    return static_cast<int>(partitionFinishes.size());
}

std::vector<HuffmanCode> huffmanCodes(const std::string& text, AlgorithmStats* output) {
    AlgorithmStats stats;
    const auto started = std::chrono::steady_clock::now();
    long long frequency[256]{};
    for (unsigned char character : text) ++frequency[character];
    std::vector<long long> weights;
    std::vector<char> symbols;
    std::vector<int> left, right, active;
    for (int i = 0; i < 256; ++i) if (frequency[i] > 0) {
        weights.push_back(frequency[i]); symbols.push_back(static_cast<char>(i)); left.push_back(-1); right.push_back(-1); active.push_back(static_cast<int>(weights.size()) - 1);
    }
    if (active.empty()) return {};
    while (active.size() > 1) {
        const int first = chooseMinimumNode(active, weights);
        const int second = chooseMinimumNode(active, weights, first);
        std::vector<int> next;
        for (const int node : active) if (node != first && node != second) next.push_back(node);
        weights.push_back(weights[first] + weights[second]); symbols.push_back('\0'); left.push_back(first); right.push_back(second);
        next.push_back(static_cast<int>(weights.size()) - 1); active = next; ++stats.operations;
    }
    std::vector<HuffmanCode> result;
    collectHuffmanCodes(active[0], left, right, symbols, "", result);
    stats.auxiliaryBytes = weights.size() * (sizeof(long long) + sizeof(char) + 2 * sizeof(int));
    stats.milliseconds = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - started).count();
    if (output) *output = stats;
    return result;
}

std::vector<long long> dijkstra(const std::vector<std::vector<long long>>& graph, int source, AlgorithmStats* output) {
    const int n = static_cast<int>(graph.size());
    if (source < 0 || source >= n) throw std::invalid_argument("Invalid Dijkstra source");
    requireSquareGraph(graph, "Dijkstra");
    AlgorithmStats stats;
    const auto started = std::chrono::steady_clock::now();
    std::vector<long long> distance(n, INF); std::vector<bool> used(n, false); distance[source] = 0;
    for (int step = 0; step < n; ++step) {
        int node = -1;
        for (int i = 0; i < n; ++i) if (!used[i] && (node == -1 || distance[i] < distance[node])) node = i;
        if (node == -1 || distance[node] == INF) break;
        used[node] = true;
        for (int neighbor = 0; neighbor < n; ++neighbor) if (graph[node][neighbor] < INF) {
            if (graph[node][neighbor] < 0) throw std::invalid_argument("Dijkstra requires non-negative weights");
            if (distance[node] + graph[node][neighbor] < distance[neighbor]) distance[neighbor] = distance[node] + graph[node][neighbor];
            ++stats.operations;
        }
    }
    stats.auxiliaryBytes = n * (sizeof(long long) + sizeof(bool));
    stats.milliseconds = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - started).count();
    if (output) *output = stats;
    return distance;
}

long long primMstWeight(const std::vector<std::vector<long long>>& graph, AlgorithmStats* output) {
    const int n = static_cast<int>(graph.size());
    if (n == 0) return 0;
    requireUndirectedGraph(graph, "Prim");
    AlgorithmStats stats; const auto started = std::chrono::steady_clock::now();
    std::vector<long long> key(n, INF); std::vector<bool> inTree(n, false); key[0] = 0; long long total = 0;
    for (int step = 0; step < n; ++step) {
        int node = -1;
        for (int i = 0; i < n; ++i) if (!inTree[i] && (node == -1 || key[i] < key[node])) node = i;
        if (node == -1 || key[node] == INF) throw std::runtime_error("Graph is disconnected");
        inTree[node] = true; total += key[node];
        for (int neighbor = 0; neighbor < n; ++neighbor) if (!inTree[neighbor] && graph[node][neighbor] < key[neighbor]) { key[neighbor] = graph[node][neighbor]; ++stats.operations; }
    }
    stats.milliseconds = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - started).count(); if (output) *output = stats; return total;
}

long long kruskalMstWeight(const std::vector<std::vector<long long>>& graph, AlgorithmStats* output) {
    const int n = static_cast<int>(graph.size());
    requireUndirectedGraph(graph, "Kruskal");
    struct Edge { int first; int second; long long weight; };
    std::vector<Edge> edges;
    for (int i = 0; i < n; ++i) for (int j = i + 1; j < n; ++j) if (graph[i][j] < INF) edges.push_back({i, j, graph[i][j]});
    AlgorithmStats stats; const auto started = std::chrono::steady_clock::now();
    for (int i = 1; i < static_cast<int>(edges.size()); ++i) { Edge item = edges[i]; int j = i - 1; while (j >= 0 && edges[j].weight > item.weight) { edges[j + 1] = edges[j]; --j; ++stats.operations; } edges[j + 1] = item; }
    DisjointSet sets(n); long long total = 0; int selected = 0;
    for (const Edge& edge : edges) if (sets.unite(edge.first, edge.second)) { total += edge.weight; if (++selected == n - 1) break; }
    if (n > 0 && selected != n - 1) throw std::runtime_error("Graph is disconnected");
    stats.milliseconds = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - started).count(); if (output) *output = stats; return total;
}

long long matrixChainCost(const std::vector<int>& dimensions, AlgorithmStats* output) {
    for (const int dimension : dimensions) {
        if (dimension <= 0) {
            throw std::invalid_argument("Matrix dimensions must be positive");
        }
    }
    if (dimensions.size() < 2) return 0;
    const int n = static_cast<int>(dimensions.size()) - 1; AlgorithmStats stats; const auto started = std::chrono::steady_clock::now();
    std::vector<std::vector<long long>> cost(n, std::vector<long long>(n, 0));
    for (int length = 2; length <= n; ++length) for (int left = 0; left <= n - length; ++left) { int right = left + length - 1; cost[left][right] = INF; for (int split = left; split < right; ++split) { const long long candidate = cost[left][split] + cost[split + 1][right] + 1LL * dimensions[left] * dimensions[split + 1] * dimensions[right + 1]; if (candidate < cost[left][right]) cost[left][right] = candidate; ++stats.operations; } }
    stats.auxiliaryBytes = n * n * sizeof(long long); stats.milliseconds = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - started).count(); if (output) *output = stats; return cost[0][n - 1];
}

int lcsLength(const std::string& left, const std::string& right, AlgorithmStats* output) {
    AlgorithmStats stats; const auto started = std::chrono::steady_clock::now();
    std::vector<std::vector<int>> table(left.size() + 1, std::vector<int>(right.size() + 1));
    for (std::size_t i = 1; i <= left.size(); ++i) for (std::size_t j = 1; j <= right.size(); ++j) { if (left[i - 1] == right[j - 1]) table[i][j] = table[i - 1][j - 1] + 1; else table[i][j] = table[i - 1][j] > table[i][j - 1] ? table[i - 1][j] : table[i][j - 1]; ++stats.operations; }
    stats.auxiliaryBytes = table.size() * table[0].size() * sizeof(int); stats.milliseconds = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - started).count(); if (output) *output = stats; return table[left.size()][right.size()];
}

double optimalBstCost(const std::vector<double>& frequencies, AlgorithmStats* output) {
    for (const double frequency : frequencies) {
        if (!std::isfinite(frequency) || frequency < 0.0) {
            throw std::invalid_argument("Optimal BST frequencies must be finite and non-negative");
        }
    }
    const int n = static_cast<int>(frequencies.size()); if (n == 0) return 0.0; AlgorithmStats stats; const auto started = std::chrono::steady_clock::now();
    std::vector<std::vector<double>> cost(n, std::vector<double>(n)); for (int i = 0; i < n; ++i) cost[i][i] = frequencies[i];
    for (int length = 2; length <= n; ++length) for (int left = 0; left <= n - length; ++left) { int right = left + length - 1; double sum = 0; for (int i = left; i <= right; ++i) sum += frequencies[i]; cost[left][right] = std::numeric_limits<double>::infinity(); for (int root = left; root <= right; ++root) { double childCost = (root > left ? cost[left][root - 1] : 0.0) + (root < right ? cost[root + 1][right] : 0.0); if (childCost + sum < cost[left][right]) cost[left][right] = childCost + sum; ++stats.operations; } }
    stats.auxiliaryBytes = n * n * sizeof(double); stats.milliseconds = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - started).count(); if (output) *output = stats; return cost[0][n - 1];
}

std::vector<long long> bellmanFord(const std::vector<std::vector<long long>>& graph, int source, bool* hasNegativeCycle, AlgorithmStats* output) {
    const int n = static_cast<int>(graph.size()); if (source < 0 || source >= n) throw std::invalid_argument("Invalid Bellman-Ford source"); AlgorithmStats stats; const auto started = std::chrono::steady_clock::now(); std::vector<long long> distance(n, INF); distance[source] = 0;
    requireSquareGraph(graph, "Bellman-Ford");
    for (int iteration = 1; iteration < n; ++iteration) { bool changed = false; for (int from = 0; from < n; ++from) for (int to = 0; to < n; ++to) if (graph[from][to] < INF && distance[from] < INF && distance[from] + graph[from][to] < distance[to]) { distance[to] = distance[from] + graph[from][to]; changed = true; ++stats.operations; } if (!changed) break; }
    bool cycle = false; for (int from = 0; from < n; ++from) for (int to = 0; to < n; ++to) if (graph[from][to] < INF && distance[from] < INF && distance[from] + graph[from][to] < distance[to]) cycle = true;
    if (hasNegativeCycle) {
        *hasNegativeCycle = cycle;
    }
    stats.milliseconds = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - started).count();
    if (output) *output = stats;
    return distance;
}

std::vector<std::vector<long long>> floydWarshall(std::vector<std::vector<long long>> graph, AlgorithmStats* output) {
    const int n = static_cast<int>(graph.size()); AlgorithmStats stats; const auto started = std::chrono::steady_clock::now();
    requireSquareGraph(graph, "Floyd-Warshall");
    for (int via = 0; via < n; ++via) for (int from = 0; from < n; ++from) for (int to = 0; to < n; ++to) if (graph[from][via] < INF && graph[via][to] < INF && graph[from][via] + graph[via][to] < graph[from][to]) { graph[from][to] = graph[from][via] + graph[via][to]; ++stats.operations; }
    stats.auxiliaryBytes = n * n * sizeof(long long); stats.milliseconds = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - started).count(); if (output) *output = stats; return graph;
}

long long boundedTsp(const std::vector<std::vector<long long>>& graph, AlgorithmStats* output) {
    const int n = static_cast<int>(graph.size());
    requireSquareGraph(graph, "Bounded TSP");
    if (n <= 1) {
        return 0;
    }
    if (n > 15) {
        throw std::invalid_argument("Bounded TSP permits at most 15 vertices");
    }
    AlgorithmStats stats;
    const auto started = std::chrono::steady_clock::now();
    const int masks = 1 << n;
    std::vector<std::vector<long long>> dp(masks, std::vector<long long>(n, INF)); dp[1][0] = 0;
    for (int mask = 1; mask < masks; ++mask) for (int last = 0; last < n; ++last) if (dp[mask][last] < INF) for (int next = 0; next < n; ++next) if (!(mask & (1 << next)) && graph[last][next] < INF) { const int nextMask = mask | (1 << next); const long long candidate = dp[mask][last] + graph[last][next]; if (candidate < dp[nextMask][next]) dp[nextMask][next] = candidate; ++stats.operations; }
    long long answer = INF; for (int last = 1; last < n; ++last) if (graph[last][0] < INF && dp[masks - 1][last] + graph[last][0] < answer) answer = dp[masks - 1][last] + graph[last][0];
    stats.auxiliaryBytes = static_cast<std::uint64_t>(masks) * n * sizeof(long long); stats.milliseconds = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - started).count(); if (output) *output = stats; return answer;
}

int countNQueensSolutions(int n, AlgorithmStats* output) {
    if (n < 0 || n > 14) {
        throw std::invalid_argument("N-Queens supports n from 0 to 14");
    }
    AlgorithmStats stats;
    const auto started = std::chrono::steady_clock::now();
    int total = 0;
    std::vector<bool> columns(n), first(2 * n), second(2 * n);
    solveQueens(0, n, columns, first, second, total, stats);
    stats.milliseconds = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - started).count();
    if (output) *output = stats;
    return total;
}

bool subsetSumExists(const std::vector<int>& values, int target, AlgorithmStats* output) {
    if (target < 0) {
        throw std::invalid_argument("Subset-sum target must be non-negative");
    }
    for (const int value : values) {
        if (value < 0) {
            throw std::invalid_argument("Subset-sum values must be non-negative");
        }
    }
    AlgorithmStats stats; const auto started = std::chrono::steady_clock::now(); const bool result = subsetSumRecursive(values, 0, target, stats); stats.milliseconds = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - started).count(); if (output) *output = stats; return result;
}

int branchBoundKnapsack(const std::vector<int>& values, const std::vector<int>& weights, int capacity, BranchBoundStrategy strategy, AlgorithmStats* output) {
    if (values.size() != weights.size() || capacity < 0) throw std::invalid_argument("Invalid branch-and-bound knapsack input");
    std::vector<KnapsackItem> items; for (std::size_t i = 0; i < values.size(); ++i) { if (weights[i] <= 0) throw std::invalid_argument("Weights must be positive"); items.push_back({values[i], weights[i]}); }
    for (int i = 1; i < static_cast<int>(items.size()); ++i) { KnapsackItem item = items[i]; int j = i - 1; while (j >= 0 && static_cast<double>(items[j].value) / items[j].weight < static_cast<double>(item.value) / item.weight) { items[j + 1] = items[j]; --j; } items[j + 1] = item; }
    AlgorithmStats stats; const auto started = std::chrono::steady_clock::now(); std::vector<KnapsackNode> frontier; KnapsackNode root; root.bound = knapsackBound(root, items, capacity); frontier.push_back(root); int best = 0;
    while (!frontier.empty()) {
        int selected = 0;
        if (strategy == BranchBoundStrategy::Lifo) selected = static_cast<int>(frontier.size()) - 1;
        else if (strategy == BranchBoundStrategy::LeastCost) for (int i = 1; i < static_cast<int>(frontier.size()); ++i) if (frontier[i].bound > frontier[selected].bound) selected = i;
        const KnapsackNode node = frontier[selected]; frontier.erase(frontier.begin() + selected);
        if (node.level == static_cast<int>(items.size()) - 1 || node.bound <= best) continue;
        const int next = node.level + 1; KnapsackNode include{next, node.value + items[next].value, node.weight + items[next].weight, 0};
        if (include.weight <= capacity && include.value > best) {
            best = include.value;
        }
        include.bound = knapsackBound(include, items, capacity);
        if (include.bound > best) {
            frontier.push_back(include);
        }
        KnapsackNode exclude{next, node.value, node.weight, 0}; exclude.bound = knapsackBound(exclude, items, capacity); if (exclude.bound > best) frontier.push_back(exclude); stats.operations += 2;
    }
    stats.milliseconds = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - started).count(); if (output) *output = stats; return best;
}

int fordFulkersonMaxFlow(std::vector<std::vector<int>> capacity, int source, int sink, AlgorithmStats* output) {
    const int n = static_cast<int>(capacity.size()); if (source < 0 || source >= n || sink < 0 || sink >= n || source == sink) throw std::invalid_argument("Source and sink must be different valid vertices");
    requireSquareCapacityMatrix(capacity);
    AlgorithmStats stats; const auto started = std::chrono::steady_clock::now(); int total = 0;
    while (true) { std::vector<int> parent(n, -1), queue; int head = 0; queue.push_back(source); parent[source] = source; while (head < static_cast<int>(queue.size()) && parent[sink] == -1) { const int current = queue[head++]; for (int next = 0; next < n; ++next) if (parent[next] == -1 && capacity[current][next] > 0) { parent[next] = current; queue.push_back(next); } }
        if (parent[sink] == -1) {
            break;
        }
        int path = std::numeric_limits<int>::max();
        for (int current = sink; current != source; current = parent[current]) {
            const int previous = parent[current];
            if (capacity[previous][current] < path) path = capacity[previous][current];
        }
        for (int current = sink; current != source; current = parent[current]) { int previous = parent[current]; capacity[previous][current] -= path; capacity[current][previous] += path; ++stats.operations; } total += path;
    }
    stats.auxiliaryBytes = n * n * sizeof(int); stats.milliseconds = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - started).count(); if (output) *output = stats; return total;
}

long long chineseRemainder(const std::vector<long long>& remainders, const std::vector<long long>& moduli) {
    if (remainders.empty() || remainders.size() != moduli.size()) throw std::invalid_argument("CRT requires equally sized, non-empty inputs");
    long long product = 1; for (long long modulus : moduli) { if (modulus <= 1 || product > std::numeric_limits<long long>::max() / modulus) throw std::invalid_argument("Invalid or oversized CRT modulus"); product *= modulus; }
    long long result = 0; for (std::size_t i = 0; i < moduli.size(); ++i) { const long long partial = product / moduli[i]; long long inverse{}, ignored{}; if (extendedGcd(partial, moduli[i], inverse, ignored) != 1) throw std::invalid_argument("CRT moduli must be pairwise coprime"); inverse = (inverse % moduli[i] + moduli[i]) % moduli[i]; result = (result + (remainders[i] % moduli[i]) * partial % product * inverse) % product; } return (result + product) % product;
}

std::vector<BenchmarkRow> runCoreBenchmarks(const std::vector<int>& sizes) {
    std::vector<BenchmarkRow> rows;
    for (const int size : sizes) {
        if (size <= 0) continue;
        const RepositoryData data = generateSyntheticRepository(size, size / 4 + 1);
        const int sortRepetitions = size <= 100 ? 50 : (size <= 500 ? 20 : 5);
        const int matchingRepetitions = size <= 100 ? 10 : 1;
        const int greedyRepetitions = size <= 500 ? 10 : 3;
        const AlgorithmStats merge = measureRepeated(
            [&data](AlgorithmStats* stats) { prioritizeIssues(data.issues, SortMethod::Merge, stats); },
            sortRepetitions);
        const AlgorithmStats quick = measureRepeated(
            [&data](AlgorithmStats* stats) { prioritizeIssues(data.issues, SortMethod::Quick, stats); },
            sortRepetitions);
        const auto mergeResult = prioritizeIssues(data.issues, SortMethod::Merge);
        const auto quickResult = prioritizeIssues(data.issues, SortMethod::Quick);
        bool sameOrder = mergeResult.size() == quickResult.size();
        for (std::size_t i = 0; sameOrder && i < mergeResult.size(); ++i) if (mergeResult[i].id != quickResult[i].id) sameOrder = false;
        const AlgorithmStats matching = measureRepeated(
            [&data](AlgorithmStats* stats) { maximumBipartiteMatching(data.developers, data.issues, stats); },
            matchingRepetitions);
        const AlgorithmStats greedy = measureRepeated(
            [&data](AlgorithmStats* stats) { greedyCapacityAllocation(data.developers, data.issues, stats); },
            greedyRepetitions);
        const AssignmentPlan matchPlan = maximumBipartiteMatching(data.developers, data.issues);
        const AssignmentPlan greedyPlan = greedyCapacityAllocation(data.developers, data.issues);
        const std::vector<Issue> adversarialInput = makeSortedPriorityInput(size);
        const AlgorithmStats adversarialMerge = measureRepeated(
            [&adversarialInput](AlgorithmStats* stats) { prioritizeIssues(adversarialInput, SortMethod::Merge, stats); },
            sortRepetitions);
        const AlgorithmStats adversarialQuick = measureRepeated(
            [&adversarialInput](AlgorithmStats* stats) { prioritizeIssues(adversarialInput, SortMethod::Quick, stats); },
            sortRepetitions);
        const auto adversarialMergeResult = prioritizeIssues(adversarialInput, SortMethod::Merge);
        const auto adversarialQuickResult = prioritizeIssues(adversarialInput, SortMethod::Quick);
        bool adversarialAgreement = adversarialMergeResult.size() == adversarialQuickResult.size();
        for (std::size_t index = 0; adversarialAgreement && index < adversarialMergeResult.size(); ++index) {
            if (adversarialMergeResult[index].id != adversarialQuickResult[index].id) {
                adversarialAgreement = false;
            }
        }
        rows.push_back({"Merge sort", size, merge, sameOrder ? "same priority order as quick sort" : "order differs"});
        rows.push_back({"Quick sort", size, quick, sameOrder ? "same priority order as merge sort" : "order differs"});
        rows.push_back({"Maximum bipartite matching", size, matching, std::to_string(matchPlan.assignments.size()) + " issues matched"});
        rows.push_back({"Greedy capacity allocation", size, greedy, std::to_string(greedyPlan.assignments.size()) + " issues allocated"});
        rows.push_back({"Merge sort (sorted input)", size, adversarialMerge,
                        adversarialAgreement ? "adversarial order remains correct" : "order differs"});
        rows.push_back({"Quick sort (sorted input)", size, adversarialQuick,
                        adversarialAgreement ? "adversarial pivot order remains correct" : "order differs"});
    }
    return rows;
}

}  // namespace issueflow
