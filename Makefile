SOURCES := $(shell find . -name '*.cpp')
OBJECTS := $(SOURCES:.cpp=.o)

CFLAGS = -std=c++11 -Wall -O0 -g

ifdef DEBUG
    CFLAGS+=-DDEBUG
endif

all: cpuinfo

cpuinfo: $(OBJECTS)
	$(CXX) $(OBJECTS) -o bin/$@ $(LIBS) $(CFLAGS)

%.o: %.cpp
	$(CXX) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm $(OBJECTS)
	rm -f bin/cpuinfo

.PHONY: clean
