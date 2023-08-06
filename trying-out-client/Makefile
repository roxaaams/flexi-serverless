CXX = g++
CXXFLAGS = -std=c++11
objects = main.o
binaries = main

all: clean main

main: $(objects)
	$(CXX) $(CXXFLAGS) -o main $(objects)

main.o: main.cpp
	$(CXX) $(CXXFLAGS) -c main.cpp

clean:
	rm -f $(binaries) $(objects)
