#include "issueflow/domain.hpp"

#include <cctype>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <unordered_set>

namespace issueflow {
namespace {

std::string trim(std::string value) {
    std::size_t first = 0;
    while (first < value.size() && std::isspace(static_cast<unsigned char>(value[first]))) {
        ++first;
    }
    std::size_t last = value.size();
    while (last > first && std::isspace(static_cast<unsigned char>(value[last - 1]))) {
        --last;
    }
    return value.substr(first, last - first);
}

std::vector<std::string> parseCsvLine(const std::string& line) {
    std::vector<std::string> fields;
    std::string current;
    bool quoted = false;
    for (std::size_t i = 0; i < line.size(); ++i) {
        const char character = line[i];
        if (character == '"') {
            if (quoted && i + 1 < line.size() && line[i + 1] == '"') {
                current += '"';
                ++i;
            } else {
                quoted = !quoted;
            }
        } else if (character == ',' && !quoted) {
            fields.push_back(trim(current));
            current.clear();
        } else {
            current += character;
        }
    }
    if (quoted) {
        throw std::runtime_error("Malformed CSV: unmatched quote");
    }
    fields.push_back(trim(current));
    return fields;
}

std::vector<std::string> splitList(const std::string& value) {
    std::vector<std::string> result;
    std::stringstream stream(value);
    std::string item;
    while (std::getline(stream, item, '|')) {
        item = trim(item);
        if (!item.empty() && item != "-") {
            result.push_back(item);
        }
    }
    return result;
}

int parsePositiveInteger(const std::string& value, const std::string& fieldName,
                         bool allowZero = false) {
    try {
        std::size_t parsed{};
        const int number = std::stoi(trim(value), &parsed);
        if (parsed != trim(value).size() || number < (allowZero ? 0 : 1)) {
            throw std::runtime_error("invalid value");
        }
        return number;
    } catch (const std::exception&) {
        throw std::runtime_error("Invalid " + fieldName + ": '" + value + "'");
    }
}

std::string csvEscape(const std::string& value) {
    if (value.find_first_of(",\"") == std::string::npos) {
        return value;
    }
    std::string escaped = "\"";
    for (char character : value) {
        escaped += character;
        if (character == '"') {
            escaped += '"';
        }
    }
    escaped += '"';
    return escaped;
}

std::string jsonEscape(const std::string& value) {
    std::string escaped;
    const char hexadecimal[] = "0123456789ABCDEF";
    for (const unsigned char character : value) {
        switch (character) {
            case '"': escaped += "\\\""; break;
            case '\\': escaped += "\\\\"; break;
            case '\b': escaped += "\\b"; break;
            case '\f': escaped += "\\f"; break;
            case '\n': escaped += "\\n"; break;
            case '\r': escaped += "\\r"; break;
            case '\t': escaped += "\\t"; break;
            default:
                if (character < 0x20) {
                    escaped += "\\u00";
                    escaped += hexadecimal[(character >> 4) & 0x0F];
                    escaped += hexadecimal[character & 0x0F];
                } else {
                    escaped += static_cast<char>(character);
                }
        }
    }
    return escaped;
}

bool containsSkill(const std::vector<std::string>& skills, const std::string& required) {
    for (const std::string& skill : skills) {
        if (skill == required) {
            return true;
        }
    }
    return false;
}

void removeUtf8Bom(std::vector<std::string>& fields) {
    const std::string bom = "\xEF\xBB\xBF";
    if (!fields.empty() && fields[0].compare(0, bom.size(), bom) == 0) {
        fields[0].erase(0, bom.size());
    }
}

void requireHeader(const std::string& line, const std::vector<std::string>& expected,
                   const std::string& fileLabel) {
    std::vector<std::string> header = parseCsvLine(line);
    removeUtf8Bom(header);
    if (header != expected) {
        std::string message = fileLabel + " CSV header must be: ";
        for (std::size_t i = 0; i < expected.size(); ++i) {
            message += (i == 0 ? "" : ",") + expected[i];
        }
        throw std::runtime_error(message);
    }
}

}  // namespace

RepositoryData loadRepositoryCsv(const std::string& developersPath, const std::string& issuesPath) {
    RepositoryData data;

    std::ifstream developersFile(developersPath);
    if (!developersFile) {
        throw std::runtime_error("Cannot open developers CSV: " + developersPath);
    }
    std::string line;
    if (!std::getline(developersFile, line)) {
        throw std::runtime_error("Developers CSV is empty");
    }
    requireHeader(line, {"id", "name", "capacity", "skills"}, "Developers");
    int lineNumber = 1;
    while (std::getline(developersFile, line)) {
        ++lineNumber;
        if (trim(line).empty()) {
            continue;
        }
        const std::vector<std::string> fields = parseCsvLine(line);
        if (fields.size() != 4) {
            throw std::runtime_error("Developers CSV line " + std::to_string(lineNumber) +
                                     " must have 4 fields");
        }
        data.developers.push_back({fields[0], fields[1],
                                   parsePositiveInteger(fields[2], "developer capacity", true),
                                   splitList(fields[3])});
    }

    std::ifstream issuesFile(issuesPath);
    if (!issuesFile) {
        throw std::runtime_error("Cannot open issues CSV: " + issuesPath);
    }
    if (!std::getline(issuesFile, line)) {
        throw std::runtime_error("Issues CSV is empty");
    }
    requireHeader(line, {"id", "title", "description", "priority", "effort", "module", "skills", "dependencies", "labels"},
                  "Issues");
    lineNumber = 1;
    while (std::getline(issuesFile, line)) {
        ++lineNumber;
        if (trim(line).empty()) {
            continue;
        }
        const std::vector<std::string> fields = parseCsvLine(line);
        if (fields.size() != 9) {
            throw std::runtime_error("Issues CSV line " + std::to_string(lineNumber) +
                                     " must have 9 fields");
        }
        data.issues.push_back({fields[0], fields[1], fields[2],
                               parsePositiveInteger(fields[3], "issue priority"),
                               parsePositiveInteger(fields[4], "issue effort"), fields[5],
                               splitList(fields[6]), splitList(fields[7]), splitList(fields[8])});
    }
    return data;
}

void writeAssignmentsCsv(const AssignmentPlan& plan, const std::string& path) {
    std::ofstream output(path);
    if (!output) {
        throw std::runtime_error("Cannot write assignment CSV: " + path);
    }
    output << "developer_id,issue_id,effort\n";
    for (const Assignment& assignment : plan.assignments) {
        output << csvEscape(assignment.developerId) << ',' << csvEscape(assignment.issueId) << ','
               << assignment.effort << '\n';
    }
}

void writeAssignmentsJson(const AssignmentPlan& plan, const std::string& path) {
    std::ofstream output(path);
    if (!output) {
        throw std::runtime_error("Cannot write assignment JSON: " + path);
    }
    output << "{\n  \"assignments\": [\n";
    for (std::size_t index = 0; index < plan.assignments.size(); ++index) {
        const Assignment& assignment = plan.assignments[index];
        output << "    {\"developer_id\": \"" << jsonEscape(assignment.developerId)
               << "\", \"issue_id\": \"" << jsonEscape(assignment.issueId)
               << "\", \"effort\": " << assignment.effort << "}";
        output << (index + 1 == plan.assignments.size() ? "\n" : ",\n");
    }
    output << "  ],\n  \"unmatched_issue_ids\": [";
    for (std::size_t index = 0; index < plan.unmatchedIssueIds.size(); ++index) {
        output << (index == 0 ? "" : ", ") << "\"" << jsonEscape(plan.unmatchedIssueIds[index]) << "\"";
    }
    output << "],\n  \"remaining_capacity\": {";

    std::vector<std::pair<std::string, int>> capacities;
    for (const auto& entry : plan.remainingCapacity) {
        capacities.push_back(entry);
    }
    for (int index = 1; index < static_cast<int>(capacities.size()); ++index) {
        const auto item = capacities[index];
        int cursor = index - 1;
        while (cursor >= 0 && capacities[cursor].first > item.first) {
            capacities[cursor + 1] = capacities[cursor];
            --cursor;
        }
        capacities[cursor + 1] = item;
    }
    for (std::size_t index = 0; index < capacities.size(); ++index) {
        output << (index == 0 ? "" : ", ") << "\"" << jsonEscape(capacities[index].first)
               << "\": " << capacities[index].second;
    }
    output << "}\n}\n";
}

IssueIdIndex::IssueIdIndex(const std::vector<Issue>& issues) {
    for (int issueIndex = 0; issueIndex < static_cast<int>(issues.size()); ++issueIndex) {
        const std::string& issueId = issues[issueIndex].id;
        if (root_ == -1) {
            nodes_.push_back({issueId, issueIndex, -1, -1});
            root_ = 0;
            continue;
        }
        int cursor = root_;
        while (true) {
            if (issueId == nodes_[cursor].issueId) {
                nodes_[cursor].issueIndex = issueIndex;
                break;
            }
            const bool goLeft = issueId < nodes_[cursor].issueId;
            const int branch = goLeft ? nodes_[cursor].left : nodes_[cursor].right;
            if (branch == -1) {
                const int newNode = static_cast<int>(nodes_.size());
                nodes_.push_back({issueId, issueIndex, -1, -1});
                if (goLeft) {
                    nodes_[cursor].left = newNode;
                } else {
                    nodes_[cursor].right = newNode;
                }
                break;
            }
            cursor = branch;
        }
    }
}

int IssueIdIndex::find(const std::string& issueId) const {
    int cursor = root_;
    while (cursor != -1) {
        if (issueId == nodes_[cursor].issueId) {
            return nodes_[cursor].issueIndex;
        }
        cursor = issueId < nodes_[cursor].issueId ? nodes_[cursor].left : nodes_[cursor].right;
    }
    return -1;
}

std::size_t IssueIdIndex::size() const { return nodes_.size(); }

ValidationResult validateRepository(const RepositoryData& data) {
    ValidationResult result;
    std::unordered_set<std::string> issueIds;
    std::unordered_set<std::string> developerIds;

    for (const Issue& issue : data.issues) {
        if (issue.id.empty() || !issueIds.insert(issue.id).second) {
            result.errors.push_back("Issue IDs must be present and unique: '" + issue.id + "'");
        }
        if (issue.title.empty()) {
            result.errors.push_back("Issue " + issue.id + " has no title");
        }
        if (issue.priority < 1 || issue.priority > 10) {
            result.errors.push_back("Issue " + issue.id + " priority must be between 1 and 10");
        }
        if (issue.effort <= 0) {
            result.errors.push_back("Issue " + issue.id + " effort must be positive");
        }
        if (issue.requiredSkills.empty()) {
            result.warnings.push_back("Issue " + issue.id + " has no required skills");
        }
    }

    for (const Developer& developer : data.developers) {
        if (developer.id.empty() || !developerIds.insert(developer.id).second) {
            result.errors.push_back("Developer IDs must be present and unique: '" + developer.id + "'");
        }
        if (developer.capacity < 0) {
            result.errors.push_back("Developer " + developer.id + " capacity cannot be negative");
        }
        if (developer.skills.empty()) {
            result.warnings.push_back("Developer " + developer.id + " has no listed skills");
        }
    }

    for (const Issue& issue : data.issues) {
        for (const std::string& dependency : issue.dependencies) {
            if (dependency == issue.id) {
                result.errors.push_back("Issue " + issue.id + " cannot depend on itself");
            } else if (issueIds.find(dependency) == issueIds.end()) {
                result.errors.push_back("Issue " + issue.id + " references missing dependency " + dependency);
            }
        }
    }

    const DependencyResult dependencies = analyzeDependencies(data.issues);
    if (!dependencies.acyclic) {
        std::string message = "Dependency cycle detected involving";
        for (const std::string& id : dependencies.cycleNodes) {
            message += " " + id;
        }
        result.errors.push_back(message);
    }
    result.valid = result.errors.empty();
    return result;
}

DependencyResult analyzeDependencies(const std::vector<Issue>& issues) {
    DependencyResult result;
    const int count = static_cast<int>(issues.size());
    std::unordered_map<std::string, int> index;
    for (int i = 0; i < count; ++i) {
        index[issues[i].id] = i;
    }
    std::vector<int> indegree(count, 0);
    std::vector<std::vector<int>> dependents(count);
    for (int i = 0; i < count; ++i) {
        for (const std::string& dependency : issues[i].dependencies) {
            const auto found = index.find(dependency);
            if (found == index.end()) {
                result.acyclic = false;
                result.cycleNodes.push_back(issues[i].id);
                return result;
            }
            ++indegree[i];
            dependents[found->second].push_back(i);
        }
    }

    // Select higher-priority ready work first without relying on a library sort.
    for (int emitted = 0; emitted < count; ++emitted) {
        int next = -1;
        for (int i = 0; i < count; ++i) {
            if (indegree[i] == 0 &&
                (next == -1 || issues[i].priority > issues[next].priority ||
                 (issues[i].priority == issues[next].priority && issues[i].id < issues[next].id))) {
                next = i;
            }
        }
        if (next == -1) {
            result.acyclic = false;
            for (int i = 0; i < count; ++i) {
                if (indegree[i] > 0) {
                    result.cycleNodes.push_back(issues[i].id);
                }
            }
            return result;
        }
        indegree[next] = -1;
        result.order.push_back(issues[next].id);
        for (const int dependent : dependents[next]) {
            --indegree[dependent];
        }
    }
    return result;
}

bool developerMatchesIssue(const Developer& developer, const Issue& issue) {
    if (developer.capacity < issue.effort) {
        return false;
    }
    for (const std::string& required : issue.requiredSkills) {
        if (!containsSkill(developer.skills, required)) {
            return false;
        }
    }
    return true;
}

std::vector<Issue> sampleIssues() {
    return {
        {"I1", "Login crash", "Authentication fails after a valid login request.", 10, 4, "Auth", {"Auth"}, {}, {"bug", "critical"}},
        {"I2", "Memory leak", "Backend worker memory grows during long jobs.", 8, 8, "Backend", {"Backend"}, {"I1"}, {"bug", "performance"}},
        {"I3", "UI alignment", "Settings page controls do not align on narrow displays.", 3, 2, "Frontend", {"Frontend"}, {}, {"ui", "good-first-issue"}},
        {"I4", "DB timeout", "Checkout query reaches its timeout under load.", 9, 7, "Database", {"Database"}, {"I1"}, {"bug", "database"}},
        {"I5", "Cache metrics", "Add a dashboard for cache-hit telemetry.", 5, 3, "Backend", {"Backend"}, {}, {"enhancement", "observability"}},
    };
}

std::vector<Developer> sampleDevelopers() {
    return {
        {"D1", "Asha", 8, {"Backend", "Auth"}},
        {"D2", "Bharat", 5, {"Frontend"}},
        {"D3", "Charu", 10, {"Database", "Backend"}},
    };
}

RepositoryData generateSyntheticRepository(int issueCount, int developerCount) {
    RepositoryData data;
    const std::vector<std::string> skills = {"Auth", "Backend", "Frontend", "Database"};
    if (issueCount < 0 || developerCount < 0) {
        return data;
    }
    for (int i = 0; i < developerCount; ++i) {
        const int first = i % static_cast<int>(skills.size());
        const int second = (i * 3 + 1) % static_cast<int>(skills.size());
        std::vector<std::string> developerSkills = {skills[first]};
        if (second != first) {
            developerSkills.push_back(skills[second]);
        }
        data.developers.push_back({"D" + std::to_string(i + 1), "Developer " + std::to_string(i + 1),
                                   5 + (i % 8), developerSkills});
    }
    for (int i = 0; i < issueCount; ++i) {
        std::vector<std::string> dependencies;
        // Generated links always point backwards, so the data remains acyclic.
        if (i > 0 && i % 5 == 0) {
            dependencies.push_back("I" + std::to_string(i));
        }
        data.issues.push_back({"I" + std::to_string(i + 1), "Synthetic issue " + std::to_string(i + 1),
                               "Synthetic description for algorithm benchmark.", 1 + ((i * 7) % 10),
                               1 + ((i * 5) % 8), skills[i % static_cast<int>(skills.size())],
                               {skills[i % static_cast<int>(skills.size())]}, dependencies,
                               {"synthetic", i % 2 == 0 ? "bug" : "enhancement"}});
    }
    return data;
}

}  // namespace issueflow
