CC = gcc
CXX = g++
CFLAGS = -g3 -O0 -ggdb -Wall
CXXFLAGS = -g3 -O0 -ggdb -Wall
OBJECTS = udpCbr.o udpCbrDriver.o

all: udpCbrGen

udpCbr.o: udpCbr.c
	$(CC) $(CFLAGS) -c udpCbr.c

udpCbrDriver.o: udpCbrDriver.cpp
	$(CXX) $(CXXFLAGS) -c udpCbrDriver.cpp

udpCbrGen: $(OBJECTS)
	$(CXX) $(OBJECTS) -o udpCbrGen

clean:
	rm -f *.o udpCbrGen
