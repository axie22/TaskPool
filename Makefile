CXX      := clang++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2 -pthread
# CXXFLAGS := -std=c++17 -Wall -Wextra -O0 -g -pthread -fsanitize=address,undefined

TARGET   := main
SOURCES  := main.cpp threadpool.cpp benchmarks.cpp
OBJECTS  := $(SOURCES:.cpp=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)
