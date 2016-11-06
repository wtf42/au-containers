CXX = g++
CXXFLAGS = -O2 -Wall -Werror -std=c++11 -static
LDFLAGS = 

EXE = $(patsubst src/%.cpp,bin/%,$(wildcard src/aucont_*.cpp))
OBJECTS = $(patsubst src/%.cpp,bin/%.o,$(wildcard src/*.cpp))
COMMON = bin/common.o bin/aucont.o bin/pid_storage.o

all: $(EXE) bin/scripts

$(EXE): bin $(OBJECTS)
	$(CXX) $@.o $(COMMON) -o $@ $(LDFLAGS)

bin/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c -MMD -o $@ $<

include $(wildcard bin/*.d)

bin:
	mkdir -p bin

bin/scripts: bin
	mkdir -p bin/scripts
	cp src/scripts/*.sh bin/scripts

clean:
	rm -rf bin $(EXE)

.PHONY: clean run all
