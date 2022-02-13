CC = g++
CFLAGS = -g3 -O0 -ggdb -Wall
OBJECTS = udpCbr.o udpCbrDriver.o

all: udpCbrGen

udpCbr.o: udpCbr.c
	$(CC) $(CFLAGS) -c udpCbr.c

udpCbrDriver.o: udpCbrDriver.cpp
	$(CC) $(CFLAGS) -c udpCbrDriver.cpp

udpCbrGen: $(OBJECTS)
	$(CC) $(OBJECTS) -o udpCbrGen

clean:
	rm -f *.o udpCbrGen
