SOURCES := $(shell find . -name '*.cpp')
OBJECTS := $(SOURCES:.cpp=.o)

FLAGS = -std=c++11 -Wall -O0 -g

all: cpuinfo

cpuinfo: $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@ $(LIBS) $(FLAGS)

%.o: %.cpp
	$(CXX) $(FLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm $(OBJECTS)
	rm -f cpuinfo

.PHONY: clean
