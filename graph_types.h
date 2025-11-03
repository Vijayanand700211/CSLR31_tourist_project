#ifndef GRAPH_TYPES_H
#define GRAPH_TYPES_H

#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

using namespace std;

// Enum to define the type of a node
enum class NodeType {
    Tank,
    Industry
};

// Represents a node in the graph (e.g., a tank or an industrial facility)
struct Node {
    int id;
    NodeType type;
    string name;
    double storageCapacity;
    double currentLevel;
    int valveStatus;
    vector<int> outgoingEdges; // Indices into the main Graph::edges vector

    Node(int id = -1, NodeType type = NodeType::Tank, const string& name = "",
         double storageCapacity = 0, double currentLevel = 0, int valveStatus = 0)
        : id(id), type(type), name(name),
          storageCapacity(storageCapacity), currentLevel(currentLevel), valveStatus(valveStatus) {}
};

// Represents a directed edge in the graph (e.g., a pipe)
struct Edge {
    int from;
    int to;
    double capacity;
    double flowRate;
    bool active;
    int valveStatus;

    Edge(int from = -1, int to = -1, double capacity = 0, double flowRate = 0, bool active = true, int valveStatus = 1)
        : from(from), to(to), capacity(capacity), flowRate(flowRate), active(active), valveStatus(valveStatus) {}
};

// Represents a single log entry for simulation history
struct LogEntry {
    int simTimeSec;
    string message;

    string toString() const;
};

#endif // GRAPH_TYPES_H