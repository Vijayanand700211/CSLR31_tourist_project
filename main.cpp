#include "graph.h"
#include <iostream>
#include <thread>

using namespace std;

int main() {

    Graph waterSystem;
    //Adding Tank nodes
    waterSystem.addNode(0, "Reservoir", NodeType::Tank, 1e12);
    waterSystem.addNode(1, "Tank 1", NodeType::Tank, 10000);
    waterSystem.addNode(2, "Tank 2", NodeType::Tank, 1000);
    waterSystem.addNode(3, "Tank 3", NodeType::Tank, 1000);
    waterSystem.addNode(4, "Tank 4", NodeType::Tank, 500);
    waterSystem.addNode(5, "Tank 5", NodeType::Tank, 1000);
    //Adding Industry nodes
    waterSystem.addNode(6, "Industry 1", NodeType::Industry, 100000);
    waterSystem.addNode(7, "Industry 2", NodeType::Industry, 100000);
    //Adding Edges between nodes (pipes)
    waterSystem.addEdge(0, 1, 100, 70, true, 1);
    waterSystem.addEdge(0, 2, 100, 70, true, 1);
    waterSystem.addEdge(0, 3, 100, 70, true, 1);
    waterSystem.addEdge(0, 4, 100, 70, true, 1);
    waterSystem.addEdge(0, 5, 100, 70, true, 1);
    waterSystem.addEdge(1, 3, 80, 60, true, 1);
    waterSystem.addEdge(1, 5, 80, 60, true, 1);
    waterSystem.addEdge(2, 4, 80, 50, true, 1);

    // Setting Initial Levels of tanks
    waterSystem.getNodeById(0)->currentLevel = 1e12;
    waterSystem.getNodeById(1)->currentLevel = 10000;
    waterSystem.getNodeById(2)->currentLevel = 500;
    waterSystem.getNodeById(3)->currentLevel = 700;
    waterSystem.getNodeById(4)->currentLevel = 10;
    waterSystem.getNodeById(5)->currentLevel = 1000;
    waterSystem.getNodeById(6)->currentLevel = 1e4;
    waterSystem.getNodeById(7)->currentLevel = 1e5;

    // Setting Initial Valve Status of tanks
    waterSystem.editNodeValveStatus(0, 1);
    waterSystem.editNodeValveStatus(1, 1);
    waterSystem.editNodeValveStatus(2, 1);
    waterSystem.editNodeValveStatus(3, 1);
    waterSystem.editNodeValveStatus(4, 1);
    waterSystem.editNodeValveStatus(5, 1);

    // Simulation parameters
    const int intervalSec = 30;                 //Simulate every 30 seconds
    const int totalSteps = 20;                  //No of steps to simulate
    const double maxReductionPerHour = 10000.0; //max reduction unit/hour
    const double prescribedLevel = 200.0;       //desired level of water in all tanks

    //Starting simulation
    cout << "Starting Simulation" << endl;
    cout << "(Prints every " << intervalSec << " seconds)" << endl;

    for (int step = 0; step < totalSteps; ++step) {
        cout << "\nSimulation Step: " << step + 1 << " Simulated Time: " << waterSystem.simTimeSec << " seconds" << endl;
        cout << "----------------------------------------" << endl;
        waterSystem.simulateStep(intervalSec, 0, maxReductionPerHour, prescribedLevel);
        cout << "----------------------------------------" << endl;
        cout << "Do you want to: \nEdit edge or node? enter e\nView logs? enter l\nClose simulation? enter c\n" << endl;

        char choice; 
        cin >> choice;
        bool finish = false;

        switch (choice) {
            case 'e': {
                cout << "Edit menu" << endl;
                cout << "Node options:" << endl;
                cout << "1. Edit node capacity\n2. Edit node valve status\n" << endl;
                cout << "Edge options:" << endl;
                cout << "3. Toggle edge active\n4. Set edge valve\n5. Edit edge capacity\n6. Edit edge flow rate" << endl;

                int subChoice; 
                cin >> subChoice;
                switch (subChoice) {
                    case 1: {
                        int nodeId; cout << "Enter node id to edit capacity: "; cin >> nodeId;
                        int newCapacity; cout << "Enter new capacity: "; cin >> newCapacity;
                        if (!waterSystem.editNodeCapacity(nodeId, newCapacity)) 
                            cout << "Node with id " << nodeId << " not found." << endl;
                        else
                            cout << "Node " << nodeId << " capacity set to " << newCapacity << endl;
                        break;
                    }
                    case 2: {
                        int nodeId; cout << "Enter node id to edit valve status: "; cin >> nodeId;
                        int newValveStatus; cout << "Enter new valve status: "; cin >> newValveStatus;
                        if (!waterSystem.editNodeValveStatus(nodeId, newValveStatus)) 
                            cout << "Node with id " << nodeId << " not found." << endl;
                        else
                            cout << "Node " << nodeId << " valve status set to " << newValveStatus << endl;
                        break;
                    }
                    case 3: {
                        int from, to;
                        cout << "Enter edge from node id and to node id (e.g. '1 2'): ";
                        cin >> from >> to;
                        int idx = waterSystem.getEdgeIndex(from, to);
                        if (idx == -1) {
                            cout << "Edge from " << from << " to " << to << " not found." << endl;
                            break;
                        }
                        waterSystem.editEdgeStatus(from, to, !waterSystem.edges[idx].active);
                        cout << "Edge " << from << "->" << to << " active set to " << waterSystem.edges[idx].active << endl;
                        break;
                    }
                    case 4: {
                        int from, to;
                        cout << "Enter edge from node id and to node id (e.g. '1 2'): ";
                        cin >> from >> to;
                        int idx = waterSystem.getEdgeIndex(from, to);
                        if (idx == -1) {
                            cout << "Edge from " << from << " to " << to << " not found." << endl;
                            break;
                        }
                        int val; cout << "Enter valve status: "; cin >> val;
                        waterSystem.editEdgeValve(from, to, val);
                        cout << "Edge " << from << "->" << to << " valve status set to " << waterSystem.edges[idx].valveStatus << endl;
                        break;
                    }
                    case 5: {
                        int from, to;
                        cout << "Enter edge from node id and to node id (e.g. '1 2'): ";
                        cin >> from >> to;
                        int idx = waterSystem.getEdgeIndex(from, to);
                        if (idx == -1) {
                            cout << "Edge from " << from << " to " << to << " not found." << endl;
                            break;
                        }
                        double newCapacity; cout << "Enter new capacity: "; cin >> newCapacity;
                        waterSystem.editEdgeCapacity(from, to, newCapacity);
                        cout << "Edge " << from << "->" << to << " capacity set to " << newCapacity << endl;
                        break;
                    }
                    case 6: {
                        int from, to;
                        cout << "Enter edge from node id and to node id (e.g. '1 2'): ";
                        cin >> from >> to;
                        int idx = waterSystem.getEdgeIndex(from, to);
                        if (idx == -1) {
                            cout << "Edge from " << from << " to " << to << " not found." << endl;
                            break;
                        }
                        double newFlowRate; cout << "Enter new flow rate: "; cin >> newFlowRate;
                        waterSystem.editEdgeFlowRate(from, to, newFlowRate);
                        cout << "Edge " << from << "->" << to << " flow rate set to " << newFlowRate << endl;
                        break;
                    }
                    default:
                        cout << "Invalid choice." << endl;
                        break;
                }
                break;
            }
            case 'l': {
                int k; 
                cout << "Enter number of log entries to view: "; 
                cin >> k;
                waterSystem.printLastKLogs(k);
                break;
            }
            case 'c':
                finish = true;
                break;
            default:
                cout << "Invalid choice." << endl;
                break;
        }

        if (finish) break;

        cout << "Has any previously detected leakage been fixed? Enter y/n: ";
        char ch; cin >> ch;
        if (ch == 'y') {
            cout << "Enter edge to mark repaired (from to), or '-1 -1' to mark all edges active: ";
            int from, to; cin >> from >> to;
            if (from == -1 && to == -1) {
                for (auto& e : waterSystem.edges) {
                    e.active = true;
                    e.valveStatus = 1;
                }
                waterSystem.pushLog("User marked all edges repaired/enabled.");
            }
            else{
                int idx = waterSystem.getEdgeIndex(from, to);
                if (idx >= 0) {
                    waterSystem.edges[idx].active = true;
                    waterSystem.edges[idx].valveStatus = 1;
                    waterSystem.pushLog("User marked edge " + to_string(from) + "->" + to_string(to) + " repaired/enabled.");
                    cout << "Edge marked repaired.\n";
                } else {
                    cout << "Edge not found.\n";
                }
            }
        }

        this_thread::sleep_for(chrono::milliseconds(150));
    }

    cout << "Simulation ended" << endl;
    cout << "Total time taken: " << waterSystem.simTimeSec << " seconds" << endl;
    cout << "Here are the final 20 logs: " << endl;
    waterSystem.printLastKLogs(20);
    return 0;
}
