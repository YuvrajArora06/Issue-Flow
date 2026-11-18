#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace issueflow {

struct Issue {
    std::string id;
    std::string title;
    std::string description;
    int priority{};       // 1 (lowest) through 10 (highest)
    int effort{};         // positive integer effort units
    std::string module;
    std::vector<std::string> requiredSkills;
    std::vector<std::string> dependencies;
    std::vector<std::string> labels;
};

struct Developer {
    std::string id;
    std::string name;
    int capacity{};       // available effort units
    std::vector<std::string> skills;
};

struct RepositoryData {
    std::vector<Issue> issues;
    std::vector<Developer> developers;
};

struct ValidationResult {
    bool valid{true};
    std::vector<std::string> errors;
    std::vector<std::string> warnings;
};

struct DependencyResult {
    bool acyclic{true};
    std::vector<std::string> order;
    std::vector<std::string> cycleNodes;
};

struct Assignment {
    std::string developerId;
    std::string issueId;
    int effort{};
};

struct AssignmentPlan {
    std::vector<Assignment> assignments;
    std::vector<std::string> unmatchedIssueIds;
    std::unordered_map<std::string, int> remainingCapacity;
};

// Reads header-based offline CSV files. Fields with commas must be double-quoted.
// Lists inside a field use the | separator, e.g. Backend|Auth.
RepositoryData loadRepositoryCsv(const std::string& developersPath,
                                 const std::string& issuesPath);

void writeAssignmentsCsv(const AssignmentPlan& plan, const std::string& path);
void writeAssignmentsJson(const AssignmentPlan& plan, const std::string& path);
ValidationResult validateRepository(const RepositoryData& data);
DependencyResult analyzeDependencies(const std::vector<Issue>& issues);

// A manually implemented binary-search-tree index for issue ID retrieval.
// Returned indices refer to the issue vector used to build this index.
class IssueIdIndex {
public:
    explicit IssueIdIndex(const std::vector<Issue>& issues);

    int find(const std::string& issueId) const;
    std::size_t size() const;

private:
    struct Node {
        std::string issueId;
        int issueIndex{};
        int left{-1};
        int right{-1};
    };

    std::vector<Node> nodes_;
    int root_{-1};
};

bool developerMatchesIssue(const Developer& developer, const Issue& issue);
std::vector<Issue> sampleIssues();
std::vector<Developer> sampleDevelopers();
RepositoryData generateSyntheticRepository(int issueCount, int developerCount);

}  // namespace issueflow
