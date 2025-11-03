#include "graph.h"
#include <chrono>

// Constructor: Initializes the simulation time and random number generator
Graph::Graph() {
    uint32_t seed = static_cast<uint32_t>(chrono::high_resolution_clock::now().time_since_epoch().count());
    rng.seed(seed);
    simTimeSec = 0;
    leakThreshold = 0.75; // Default leak threshold
}