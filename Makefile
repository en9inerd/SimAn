# Makefile for SimAn - Simulated Annealing Placer
# For macOS with clang++

CXX = clang++
CXXFLAGS = -std=c++11 -Wall -O2 -I.
LDFLAGS = -framework OpenGL -framework GLUT

# Source files
SOURCES = BBox.cpp \
          DataPlace.cpp \
          paramproc.cpp \
          SimAnneal.cpp \
          visualization.cpp \
          simulated_annealing.cpp

# Object files
OBJECTS = BBox.o \
          DataPlace.o \
          paramproc.o \
          SimAnneal.o \
          visualization.o \
          simulated_annealing.o

# Target executable
TARGET = siman

# Default target
all: $(TARGET)

# Link executable
$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

# Compile source files to object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@


# Clean build artifacts
clean:
	rm -f $(OBJECTS) $(TARGET)

# Phony targets
.PHONY: all clean
