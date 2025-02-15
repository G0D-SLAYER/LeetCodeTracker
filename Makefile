# Compiler and flags
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17

# Libraries
LIBS = -lncurses -lsqlite3 -lcrypto

# Target executable
TARGET = tui_program

# Source files
SRCS = tui_program.cpp database.cpp

# Default target
all: $(TARGET)

# Build target
$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRCS) $(LIBS)

# Run the program
run: $(TARGET)
	./$(TARGET)

# Clean up build files
clean:
	rm -f $(TARGET)

.PHONY: all run clean
