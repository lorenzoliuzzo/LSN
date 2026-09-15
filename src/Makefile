# make                          builds build/simulator
# build/simulator <run_dir>     reads <run_dir>/input.dat, writes <run_dir>/output/
CXX      := g++
CXXFLAGS := -O3 -std=c++17 -Wall -Wextra -Wpedantic
CPPFLAGS := -I. -MMD -MP
BUILD    := build

SOURCES := $(wildcard core/*.cpp lj/*.cpp ising/*.cpp app/*.cpp)
OBJECTS := $(SOURCES:%.cpp=$(BUILD)/%.o)

$(BUILD)/simulator: $(OBJECTS)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(BUILD)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -rf $(BUILD)

-include $(OBJECTS:.o=.d)
