CC = gcc
CFLAGS=-W -Wall -Wextra -O3 -I../raylib-5.5/src
LFLAGS=../raylib-5.5/src/libraylib.a -lm -ldl -pthread
EXES=delaunay

all: $(EXES)

delaunay: main.o delaunay.o utils.o
	$(CC) $^ -o delaunay $(LFLAGS)

main.o: main.c
	$(CC) -c $< $(CFLAGS)

delaunay.o: delaunay.c
	$(CC) -c $< $(CFLAGS)

utils.o: utils.c utils.h
	$(CC) -c $< $(CFLAGS)

clean:
	rm -v *.o $(EXES)
