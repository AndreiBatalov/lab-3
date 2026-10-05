CXX ?= c++
CPPFLAGS ?=
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -O2
LDFLAGS ?=
LDLIBS ?=

TARGET := build/lab3
SOURCES := main.cpp
OBJECTS := $(SOURCES:%.cpp=build/%.o)
DEPENDENCIES := $(OBJECTS:.o=.d)

.PHONY: all run stage1
all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(LDFLAGS) $(OBJECTS) $(LDLIBS) -o $@

build/%.o: %.cpp | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

build:
	mkdir -p build

run: $(TARGET)
	./$(TARGET)

build/stage1_main.o: stage1/main.cpp stage1/sample_double.h | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

build/sample_double.o: stage1/sample_double.cpp stage1/sample_double.h | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

build/lab3_stage1: build/stage1_main.o build/sample_double.o
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

stage1: build/lab3_stage1
	./build/lab3_stage1

-include $(DEPENDENCIES) build/stage1_main.d build/sample_double.d
