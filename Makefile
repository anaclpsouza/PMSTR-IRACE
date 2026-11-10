CXX := g++
CXXFLAGS := -O3 -march=native -std=c++20

.PHONY: all clean

all: Main target-runner

Main: Main.cpp ObjectiveFunction.cpp Operation.cpp Buscas.cpp ObjectiveFunction.h Operation.h Buscas.h
	$(CXX) $(CXXFLAGS) Main.cpp ObjectiveFunction.cpp Operation.cpp Buscas.cpp -o Main

target-runner: target-runner.cpp
	$(CXX) $(CXXFLAGS) target-runner.cpp -o target-runner

clean:
	rm -f Main target-runner
