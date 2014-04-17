CC = g++
CFLAGS = -O3

DEBUG ?= 0
ifeq ($(DEBUG), 1)
	CFLAGS += -g
endif

all: test aligner

test: myers.o test.cpp
	$(CC) $(CFLAGS) myers.o test.cpp -o test

aligner: myers.o aligner.cpp
	$(CC) $(CFLAGS) myers.o aligner.cpp -o aligner

myers.o: myers.cpp myers.h
	$(CC) $(CFLAGS) -c myers.cpp

# etags are used in emacs
etags:
	etags *.cpp *.h

clean:
	rm *.o test
