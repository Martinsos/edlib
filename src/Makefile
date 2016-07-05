CXX ?= g++
CXXFLAGS ?= -O3

DEBUG ?= 0
ifeq ($(DEBUG), 1)
	CXXFLAGS += -g -Wall
endif

all: test aligner helloWorld

test: edlib.o tests/test.cpp tests/SimpleEditDistance.h
	$(CXX) $(CXXFLAGS) edlib.o tests/test.cpp -o test -I library

aligner: edlib.o apps/aligner/aligner.cpp
	$(CXX) $(CXXFLAGS) edlib.o apps/aligner/aligner.cpp -o aligner -I library

helloWorld: edlib.o apps/hello-world/helloWorld.c
	$(CXX) $(CXXFLAGS) edlib.o apps/hello-world/helloWorld.c -o helloWorld -I library

edlib.o: library/edlib.cpp library/edlib.h
	$(CXX) $(CXXFLAGS) -c library/edlib.cpp

# etags are used in emacs
etags:
	etags *.cpp *.h

clean:
	rm *.o test aligner helloWorld
