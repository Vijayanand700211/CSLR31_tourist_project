#ifndef GRAPH_H
#define GRAPH_H

#include "graph_types.h"
#include <random>
#include <unordered_set>
#include <queue>
#include <vector>

class Graph {
public:
    vector<Node> nodes;
    vector<Edge> edges;
    int simTimeSec;
    std::mt19937 rng;
    vector<LogEntry> history;
    double leakThreshold;

    Graph(); // Constructor

    // --- Node and Edge Operations (graph_operations.cpp) ---
    void addNode(int id, const string& name, NodeType type, double capacity = 0);
    void addEdge(int from, int to, double capacity, double flowRate = 0, bool active = true, int valveStatus = 1);
    void rebuildOutgoingEdges();
    void deactivateEdge(int from, int to);
    void activateEdge(int from, int to);
    void displayNode(int id) const;
    void displayEdge(int from, int to) const;
    bool editNodeCapacity(int id, double newCapacity);
    bool editNodeName(int id, const string& newName);
    bool editNodeType(int id, NodeType newType);
    bool editNodeValveStatus(int id, int newValveStatus);
    bool editEdgeCapacity(int from, int to, double newCapacity);
    bool editEdgeFlowRate(int from, int to, double newFlowRate);
    bool editEdgeStatus(int from, int to, bool newStatus);
    bool editEdgeValve(int from, int to, int newValveStatus);
    Node* getNodeById(int id);
    const Node* getNodeByIdConst(int id) const;
    Edge* getEdgeByIndex(int idx);
    int getEdgeIndex(int from, int to) const;

    // --- Simulation Logic (graph_simulation.cpp) ---
    void updateTankLevels(int intervalSec, double maxReductionPerHour);
    bool findPath(int sourceId, int targetId, vector<int>& path, const unordered_set<int>& bannedEdges = {}) const;
    pair<double, double> supplyWaterAlongPath(int sourceId, const vector<int>& path, int intervalSec);
    void simulateStep(int intervalSec, int sourceId, double maxReductionPerHour, double prescribedLevel);

    // --- Logging and Utilities (graph_logging.cpp) ---
    void pushLog(const string& message);
    void printLastKLogs(int k) const;
    static string formatTime(int seconds);
    size_t historySize() const { return history.size(); }
};

#endif // GRAPH_H