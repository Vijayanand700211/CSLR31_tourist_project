#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <unordered_map>
#include "graph.h"

void Graph::updateTankLevels(int intervalSec, double maxReductionPerHour){
    // convert maxReductionPerHour units/hour to units/sec
    double maxReductionPerSec = maxReductionPerHour / 3600.0;
    uniform_real_distribution<double> dist(0.0, 1.0);

    for (auto& n : nodes){
        if(n.id==0) continue;
        double r = dist(rng); // random factor in [0,1)
        double reduction = r * maxReductionPerSec * intervalSec;
        double before = n.currentLevel;

        n.currentLevel = max(0.0, n.currentLevel - reduction);

        if (reduction > 0.0) {
            ostringstream oss;
            oss << (n.type == NodeType::Tank ? "Tank " : "Industry ")
                << n.id << " consumed " << reduction
                << " units (before=" << before
                << ", after=" << n.currentLevel << ")";
                pushLog(oss.str());
        }
    }
}

// BFS pathfinding; bannededges is set of edge indices to avoid because they might be broken
bool Graph::findPath(int sourceId, int targetId, vector<int>& path, const unordered_set<int>& bannedEdges) const {
    unordered_map<int,int> parentNode;
    unordered_map<int,int> parentEdge; // edge index used to reach node
    queue<int> q;
    unordered_set<int> visited;

    q.push(sourceId);
    visited.insert(sourceId);

    while (!q.empty()) {
        int current = q.front(); q.pop();

        if (current == targetId) {
            // reconstruct path as list of edge indices
            path.clear();
            int node = targetId;
            while (node != sourceId){
                int eidx = parentEdge[node];
                path.push_back(eidx);
                node = parentNode[node];
            }
            reverse(path.begin(), path.end());
            return true;
        }

        // explore outgoing edges from current
        const Node* cn = getNodeByIdConst(current);
        if (!cn) continue;
        for (int eidx : cn->outgoingEdges) {
            if (bannedEdges.count(eidx)) continue;
            const Edge& e = edges[eidx];
            if (!e.active) continue;
            if (e.valveStatus == 0) continue; // pipe closed
            int neighbor = e.to;
            if (!visited.count(neighbor)) {
                visited.insert(neighbor);
                parentNode[neighbor] = current;
                parentEdge[neighbor] = eidx;
                q.push(neighbor);
            }
        }
    }
    return false;
}

// Supply water along a path of edge indices.
// Returns (expectedDelivered, actualDelivered).
pair<double,double> Graph::supplyWaterAlongPath(int sourceId, const vector<int>& path, int intervalSec) {
    if (path.empty()) return {0,0};

    Node* source = getNodeById(sourceId);
    if (!source) return {0,0};
    int targetNodeId = edges[path.back()].to;
    Node* target = getNodeById(targetNodeId);
    if (!target) return {0,0};

    // Find bottleneck supply rate across the entire path
    double bottleneck = numeric_limits<double>::infinity();
    for (int idx : path) {
        const Edge& e = edges[idx];
        double r = min(e.capacity, e.flowRate);
        if (r < bottleneck) bottleneck = r;
    }
    if (!isfinite(bottleneck)) return {0,0};

    double supplyRate = 0.8 * bottleneck; // units/sec
    double expected = supplyRate * static_cast<double>(intervalSec);

    // How much target tank can actually accept
    double remaining = max(0.0, target->storageCapacity - target->currentLevel);
    if (remaining <= 0.0) {
        return {0,0}; // tank already full
    }

    // Adjust interval if expected would overshoot
    if (expected > remaining) {
        intervalSec = static_cast<int>(remaining / supplyRate);
        expected = remaining;
    }

    // Amount that will be transferred
    double transfer = min(remaining, expected);

    // If source is a normal tank (not reservoir), reduce its level
    if (source->id !=0) {
        source->currentLevel = max(0.0, source->currentLevel - transfer);
    }

    double before = target->currentLevel;
    target->currentLevel = min(target->currentLevel + transfer, target->storageCapacity);
    double actualDelivered = target->currentLevel - before;

    // Log supply event
    {
        ostringstream oss;
        oss << "Supplied to Tank " << target->id << " via path (";
        for (size_t i = 0; i < path.size(); ++i) {
            if (i) oss << "->";
            oss << edges[path[i]].from;
        }
        oss << "->" << target->id << ") expected=" << expected
            << " actual=" << actualDelivered
            << " (before=" << before << ", after=" << target->currentLevel << ")";
        pushLog(oss.str());
    }

    return {expected, actualDelivered};
}


// Simulate single time step (intervalSec seconds)
// Simulate single time step (intervalSec seconds) with priority-based tank filling
void Graph::simulateStep(int intervalSec, int sourceId, double maxReductionPerHour, double prescribedLevel) {
    // Advance simulation time
    simTimeSec += intervalSec;

    // 1) Reduce tank/industry levels due to consumption/leak
    updateTankLevels(intervalSec, maxReductionPerHour);

    // 2) Create priority queue of tanks that need refilling
    // Priority criteria:
    // - Lower current level (more empty tanks get higher priority)
    // - Higher storage capacity (larger tanks get higher priority when equally empty)
    // - You can add other criteria like tank importance/type if needed

    // Using a max-heap based on priority score
    struct TankPriority {
        int nodeId;
        double currentLevel;
        double storageCapacity;
        double priorityScore;

        // Higher priority score means more urgent
        bool operator<(const TankPriority& other) const {
            return priorityScore < other.priorityScore;
        }
    };

    priority_queue<TankPriority> tankQueue;

    // Calculate priority for each tank below prescribed level
    for (auto& n : nodes) {
        if (n.type != NodeType::Tank) continue;
        if (n.currentLevel >= prescribedLevel) continue;

        TankPriority tp;
        tp.nodeId = n.id;
        tp.currentLevel = n.currentLevel;
        tp.storageCapacity = n.storageCapacity;

        // Priority calculation:
        // - Higher priority for more empty tanks (lower current level)
        // - Higher priority for larger tanks when equally empty
        // - You can customize this formula based on your needs
        double emptinessRatio = 1.0 - (n.currentLevel / prescribedLevel);
        tp.priorityScore = emptinessRatio * n.storageCapacity;

        tankQueue.push(tp);

        ostringstream preMsg;
        preMsg << "Tank " << n.id << " (" << n.name << ") below prescribed level: "
               << n.currentLevel << " < " << prescribedLevel << " | Priority: " << tp.priorityScore;
        pushLog(preMsg.str());
    }

    cout << "\n--- Priority-based refill sequence at " << Graph::formatTime(simTimeSec) << " ---\n";

    // 3) Process tanks in priority order
    int tanksProcessed = 0;
    const int MAX_TANKS_PER_STEP = 3; // Limit tanks processed per step to avoid starvation

    while (!tankQueue.empty() && tanksProcessed < MAX_TANKS_PER_STEP) {
        TankPriority currentTank = tankQueue.top();
        tankQueue.pop();

        Node* tankNode = getNodeById(currentTank.nodeId);
        if (!tankNode) continue;

        cout << "Processing Tank " << currentTank.nodeId << " (" << tankNode->name
             << ") - Priority: " << currentTank.priorityScore
             << " Level: " << currentTank.currentLevel << "/" << currentTank.storageCapacity << "\n";

        vector<int> path;
        if (!findPath(sourceId, currentTank.nodeId, path)) {
            string msg = "No available path from source " + to_string(sourceId) +
                         " to tank " + to_string(currentTank.nodeId);
            pushLog(msg);
            cout << "  " << msg << "\n";
            tanksProcessed++;
            continue;
        }

        // Attempt filling along found path
        auto [expected, actual] = supplyWaterAlongPath(sourceId, path, intervalSec);

        cout << "  Expected delivered (units): " << expected
             << " | Actual delivered: " << actual << "\n";

        // If expected > 0 and actual < threshold*expected => leak suspected
        if (expected > 0 && actual < leakThreshold * expected) {
            ostringstream leakMsg;
            leakMsg << "Leak suspected on path to Tank " << currentTank.nodeId
                    << " (expected=" << expected << ", actual=" << actual << ")";
            pushLog(leakMsg.str());
            cout << "  >>> Leak suspected on path to Tank " << currentTank.nodeId
                 << " (actual < " << leakThreshold*100 << "% of expected).\n";

            // Try alternate route (ban edges in current path)
            unordered_set<int> banned;
            for (int idx : path) banned.insert(idx);

            vector<int> altPath;
            if (findPath(sourceId, currentTank.nodeId, altPath, banned)) {
                cout << "  Alternate route found. Attempting alternate route...\n";
                auto [exp2, act2] = supplyWaterAlongPath(sourceId, altPath, intervalSec);
                cout << "    Alternate expected: " << exp2 << " | Actual: " << act2 << "\n";
                if (exp2 > 0 && act2 < leakThreshold * exp2) {
                    string altMsg = "Alternate route also suspected leaking for Tank " + to_string(currentTank.nodeId);
                    pushLog(altMsg);
                    cout << "    >>> Alternate route also under-delivered.\n";
                } else {
                    string okMsg = "Alternate route delivered adequately to Tank " + to_string(currentTank.nodeId);
                    pushLog(okMsg);
                    cout << "    Alternate route delivered adequately.\n";
                }
            } else {
                string noneMsg = "No alternate route available for Tank " + to_string(currentTank.nodeId) +
                                 ". Please inspect pipes.";
                pushLog(noneMsg);
                cout << "  No alternate route available. Please inspect pipes or mark leak repaired.\n";
            }
        }

        tanksProcessed++;
    }

    // Log if some tanks weren't processed due to limit
    if (!tankQueue.empty()) {
        ostringstream limitMsg;
        limitMsg << tanksProcessed << " tanks processed this step. "
                 << tankQueue.size() << " tanks remaining in queue for next step.";
        pushLog(limitMsg.str());
        cout << limitMsg.str() << "\n";
    }

    // 4) Print snapshot
    cout << "\n--- Snapshot at " << Graph::formatTime(simTimeSec) << " ---\n";
    cout << fixed << setprecision(2);
    for (const auto& no : nodes) {
        cout << "Node " << no.id << " (" << no.name << "): level=" << no.currentLevel
             << " / " << no.storageCapacity << " | valveStatus=" << no.valveStatus
             << " | outgoing=" << no.outgoingEdges.size() << "\n";
    }
    cout << "Edges:\n";
    for (size_t i = 0; i < edges.size(); ++i) {
        const auto& e = edges[i];
        cout << "  Edge[" << i << "] " << e.from << "->" << e.to
             << " cap=" << e.capacity << " flowRate=" << e.flowRate
             << " active=" << (e.active ? "Y":"N")
             << " valve=" << e.valveStatus << "\n";
    }
    cout << "----------------------------------------\n";
}
