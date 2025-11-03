# ==== Project Settings ====
CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2
TARGET := graph_app

# ==== Source and Object Files ====
SRC := Graph.cpp graph_logging.cpp graph_simulations.cpp graph_operations.cpp main.cpp
OBJ := $(SRC:.cpp=.o)

# ==== Build Rules ====
all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(TARGET)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# ==== Utility Commands ====
clean:
	rm -f $(OBJ) $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
