CXX := g++
CXXFLAGS := -O3 -march=native -std=c++20

.PHONY: all clean

all: Main Run 

Main: Main.cpp ObjectiveFunction.cpp Operation.cpp Buscas.cpp ObjectiveFunction.h Operation.h Buscas.h
	$(CXX) $(CXXFLAGS) Main.cpp ObjectiveFunction.cpp Operation.cpp Buscas.cpp -o Main

Run: Run.cpp
	$(CXX) $(CXXFLAGS) Run.cpp -o Run


clean:
	rm -f Main Run 
