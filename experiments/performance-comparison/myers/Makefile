CC = clang
CFLAGS= -O3 -Wno-everything

all: myers.c main.i parse.i
	$(CC) $(CFLAGS) myers.c -o myers

clean:
	rm myers
