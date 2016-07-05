CC = g++
CFLAGS = -O3

DEBUG ?= 0
ifeq ($(DEBUG), 1)
	CFLAGS += -g -Wall
endif

all: test aligner helloWorld

test: edlib.o tests/test.cpp tests/SimpleEditDistance.h
	$(CC) $(CFLAGS) edlib.o tests/test.cpp -o test -I library

aligner: edlib.o apps/aligner/aligner.cpp
	$(CC) $(CFLAGS) edlib.o apps/aligner/aligner.cpp -o aligner -I library

helloWorld: edlib.o apps/hello-world/helloWorld.c
	$(CC) $(CFLAGS) edlib.o apps/hello-world/helloWorld.c -o helloWorld -I library

edlib.o: library/edlib.cpp library/edlib.h
	$(CC) $(CFLAGS) -c library/edlib.cpp

# etags are used in emacs
etags:
	etags *.cpp *.h

clean:
	rm *.o test aligner helloWorld
