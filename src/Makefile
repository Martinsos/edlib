CXX = clang++
SEQAN_INCLUDE_PATH = ../../seqan-v2.2.0/include/
CFLAGS = -O3 -I$(SEQAN_INCLUDE_PATH) -std=c++14

DEBUG ?= 0
ifeq ($(DEBUG), 1)
	CFLAGS += -g -Wall
endif

all: test aligner

test: edlib.o test.cpp SimpleEditDistance.h
	$(CXX) $(CFLAGS) edlib.o test.cpp -o test

aligner: edlib.o aligner.cpp SimpleEditDistance.h
	$(CXX) $(CFLAGS) edlib.o aligner.cpp -o aligner

edlib.o: edlib.cpp edlib.h
	$(CXX) $(CFLAGS) -c edlib.cpp

# etags are used in emacs
etags:
	etags *.cpp *.h

clean:
	rm *.o test
