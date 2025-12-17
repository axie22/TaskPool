CXX := clang++ 									# Use clang++ as the C++ compiler
CXXFLAGS := -std=c++17 -Wall -Wextra -O2		# Compiler flags: C++17 standard, enable all warnings, optimization level 2

TARGET := main									# Name of the output executable
SOURCES := main.cpp threadpool.cpp				# Source files	
OBJECTS := $(SOURCES:.cpp=.o)					# Auto generated object files

.PHONY: all clean								# Declare 'all' and 'clean' as phony targets

all: $(TARGET)									# Default target to build the executable
$(TARGET): $(OBJECTS)							# Link object files to create the executable
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $@

%.o: %.cpp threadpool.h							# Compile source files to object files
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:											# Clean up generated files
	rm -f $(OBJECTS) $(TARGET)