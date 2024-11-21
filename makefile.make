# Compiler and flags
CXX = clang++          # Fixed to clang++
CXXFLAGS ?= -O2 -std=c++17 -Wall -Wextra # Default compiler flags

# Automatically find all source files
SRC = $(wildcard *.cpp)

# Generate object file names
OBJ = $(SRC:.cpp=.o)

# Output binary (default)
EXE ?= Engine

# Default target
all: $(EXE)

# Rule to build the executable
$(EXE): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Rule to build object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up build files
clean:
	rm -f $(OBJ) $(EXE)

# Phony targets
.PHONY: all clean
