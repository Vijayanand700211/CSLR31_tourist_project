#include "graph.h"
#include <iostream>
// ---------------- node and edge operations ----------------
void Graph::addNode(int id, const string& name, NodeType type, double capacity){
    //adds a new node (tank or industry) to the graph if the id is unique.
    for (const auto& n : nodes){
        if (n.id == id){
            cerr << "Node with ID " << id << " already exists.\n";
            return;
        }
    }
    nodes.emplace_back(id, type, name, capacity, 0.0, 0);
}

void Graph::addEdge(int from, int to, double capacity, double flowRate, bool active, int valveStatus){
    //adds a pipe (edge) between two nodes if not already present.
    for (const auto& e : edges){
        if (e.from == from && e.to == to){
            cerr << "Edge from " << from << " to " << to << " already exists.\n";
            return;
        }
    }
    edges.emplace_back(from, to, capacity, flowRate, active, valveStatus);
    rebuildOutgoingEdges();
}

void Graph::rebuildOutgoingEdges(){
    //rebuilds each node’s outgoing edge list after updates so valve selections always match current connections
    for (auto& n : nodes) n.outgoingEdges.clear();
    for (size_t i = 0; i < edges.size(); ++i){
        const auto& e = edges[i];
        if(!e.active) continue;
        for (auto& n : nodes){
            if (n.id == e.from){
                n.outgoingEdges.push_back(static_cast<int>(i));
                break;
            }
        }
    }
}

void Graph::deactivateEdge(int from, int to){
    //Deactivates(if pipe is damaged or not usable) the edge from one node to another, rebuilds adjacency, and logs the change.
    for (auto& e : edges){
        if (e.from == from && e.to == to){
            e.active = false;
            rebuildOutgoingEdges();
            pushLog("Edge " + to_string(from) + "->" + to_string(to) + " deactivated by user.");
            return;
        }
    }
}

void Graph::activateEdge(int from, int to){
    //activates(if pipe is repaired) the edge from one node to another, rebuilds adjacency, and logs the change.
    for (auto& e : edges) {
        if (e.from == from && e.to == to){
            e.active = true;
            rebuildOutgoingEdges();
            pushLog("Edge " + to_string(from) + "->" + to_string(to) + " activated by user.");
            return;
        }
    }
}

void Graph::displayNode(int id) const{
    // to print the present details of the node
    for (const auto& n : nodes){
        if (n.id == id){
            cout << "Node ID: " << n.id << "\n"
                 << "Name: " << n.name << "\n"
                 << "Type: " << (n.type == NodeType::Tank ? "Tank" : "Industry") << "\n"
                 << "Capacity: " << n.storageCapacity << "\n"
                 << "Current Level: " << n.currentLevel << "\n"
                 << "Valve Status: " << n.valveStatus << "\n"
                 << "Outgoing edges count: " << n.outgoingEdges.size() << "\n";
            return;
        }
    }
    cout << "Node with ID " << id << " not found.\n";
}

void Graph::displayEdge(int from, int to) const{
    for (const auto& e : edges){
        if (e.from == from && e.to == to){
            cout << "Edge from " << e.from << " to " << e.to << "\n"
                 << "Capacity (units/sec): " << e.capacity << "\n"
                 << "Flow Rate (units/sec): " << e.flowRate << "\n"
                 << "Active: " << (e.active ? "Yes" : "No") << "\n"
                 << "Valve Status: " << e.valveStatus << "\n";
            return;
        }
    }
    cout << "Edge from " << from << " to " << to << " not found.\n";
}

bool Graph::editNodeCapacity(int id, double newCapacity){
    //to edit the capacity of the node
    for (auto& n : nodes){
        if (n.id == id) {
            n.storageCapacity = newCapacity;
            pushLog("Node " + to_string(id) + " capacity set to " + to_string(newCapacity));
            return true;
        }
    }
    return false;
}

bool Graph::editNodeName(int id, const string& newName){
    // to change name of the node
    for (auto& n : nodes){
        if (n.id == id){
            n.name = newName;
            pushLog("Node " + to_string(id) + " name changed to " + newName);
            return true;
        }
    }
    return false;
}

bool Graph::editNodeType(int id, NodeType newType){
    // to change the type of the node
    for (auto& n : nodes){
        if (n.id == id) {
            n.type = newType;
            pushLog("Node " + to_string(id) + " type changed.");
            return true;
        }
    }
    return false;
}

bool Graph::editNodeValveStatus(int id, int newValveStatus){
    // to edit the valve status whether to fill the tank or to pass on to others
    for (auto& n : nodes){
        if (n.id == id){
            n.valveStatus = newValveStatus;
            pushLog("Node " + to_string(id) + " valveStatus set to " + to_string(newValveStatus));
            return true;
        }
    }
    return false;
}

bool Graph::editEdgeCapacity(int from, int to, double newCapacity) {
    for (auto& e : edges) {
        if (e.from == from && e.to == to) {
            e.capacity = newCapacity;
            pushLog("Edge " + to_string(from) + "->" + to_string(to) + " capacity set to " + to_string(newCapacity));
            return true;
        }
    }
    return false;
}

bool Graph::editEdgeFlowRate(int from, int to, double newFlowRate) {
    for (auto& e : edges) {
        if (e.from == from && e.to == to) {
            e.flowRate = newFlowRate;
            pushLog("Edge " + to_string(from) + "->" + to_string(to) + " flowRate set to " + to_string(newFlowRate));
            return true;
        }
    }
    return false;
}

bool Graph::editEdgeStatus(int from, int to, bool newStatus) {
    for (auto& e : edges) {
        if (e.from == from && e.to == to) {
            e.active = newStatus;
            rebuildOutgoingEdges();
            pushLog("Edge " + to_string(from) + "->" + to_string(to) + " active set to " + (newStatus ? "true":"false"));
            return true;
        }
    }
    return false;
}

bool Graph::editEdgeValve(int from, int to, int newValveStatus) {
    for (auto& e : edges) {
        if (e.from == from && e.to == to) {
            e.valveStatus = newValveStatus;
            pushLog("Edge " + to_string(from) + "->" + to_string(to) + " valve set to " + to_string(newValveStatus));
            return true;
        }
    }
    return false;
}

// ---------------- helpers ----------------
Node* Graph::getNodeById(int id) {
    for (auto& n : nodes) if (n.id == id) return &n;
    return nullptr;
}
const Node* Graph::getNodeByIdConst(int id) const {
    for (const auto& n : nodes) if (n.id == id) return &n;
    return nullptr;
}
Edge* Graph::getEdgeByIndex(int idx) {
    if (idx < 0 || idx >= static_cast<int>(edges.size())) return nullptr;
    return &edges[idx];
}
int Graph::getEdgeIndex(int from, int to) const {
    for (size_t i = 0; i < edges.size(); ++i) {
        if (edges[i].from == from && edges[i].to == to) return static_cast<int>(i);
    }
    return -1;
}