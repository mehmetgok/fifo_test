CC=g++
CFLAGS=-c -Wall -O2
LIBS=-lpthread -lrt

all: fifot

fifot: main.o fifo.o
	$(CC) main.o fifo.o -o fifot $(LIBS)

main.o: main.cpp
	$(CC) $(CFLAGS) main.cpp

fifo.o: fifo.cpp
	$(CC) $(CFLAGS) fifo.cpp

clean:
	rm *o fifot
