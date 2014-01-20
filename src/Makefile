CC = g++
CFLAGS = -O3

DEBUG ?= 0
ifeq ($(DEBUG), 1)
	CFLAGS += -g
endif

all: test

test: myers.o test.cpp
	$(CC) $(CFLAGS) myers.o test.cpp -o test

myers.o: myers.cpp myers.h
	$(CC) $(CFLAGS) -c myers.cpp

clean:
	rm *.o test
