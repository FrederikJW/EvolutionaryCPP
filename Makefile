# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++11 -Wall -Iinclude -Iinclude/strategies

# Source directories
SRCDIR = src
SRCDIR_STRATEGIES = src/strategies

# Header directories
INCDIR = include
INCDIR_STRATEGIES = include/strategies

# Sources
SOURCES = $(wildcard $(SRCDIR)/*.cpp) $(wildcard $(SRCDIR_STRATEGIES)/*.cpp)

# Objects
OBJECTS = $(SOURCES:.cpp=.o)

# Executable
EXEC = EvolutionaryCPP

# Build target
$(EXEC): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Clean target
clean:
	rm -f $(EXEC) $(OBJECTS)
