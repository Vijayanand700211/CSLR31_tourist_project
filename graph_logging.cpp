#include "graph.h"
#include <iostream>

string LogEntry::toString() const {
    ostringstream oss;
    int mm = simTimeSec / 60;
    int ss = simTimeSec % 60;
    oss << "[" << setw(2) << setfill('0') << mm << ":" << setw(2) << setfill('0') << ss << "] " << message;
    return oss.str();
}

void Graph::pushLog(const string& message) {
    history.emplace_back(LogEntry{simTimeSec, message});
}

void Graph::printLastKLogs(int k) const {
    if (history.empty()) {
        cout << "(No history yet)\n";
        return;
    }
    int n = static_cast<int>(history.size());
    int start = max(0, n - k);
    for (int i = n - 1; i >= start; --i) {
        cout << history[i].toString() << "\n";
    }
}

string Graph::formatTime(int seconds) {
    int mm = seconds / 60;
    int ss = seconds % 60;
    ostringstream oss;
    oss << setw(2) << setfill('0') << mm << ":" << setw(2) << setfill('0') << ss;
    return oss.str();
}