# Makefile for compiling the TUI program

# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -lncurses -lsqlite3

# Source files
SRCS = tui_program.cpp

# Output executable
TARGET = tui_program

# Default target
all: $(TARGET)

# Compile the program
$(TARGET): $(SRCS)
	$(CXX) $(SRCS) -o $(TARGET) $(CXXFLAGS)

# Clean up
clean:
	rm -f $(TARGET)
