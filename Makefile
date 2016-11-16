CC = g++
DEBUG = -g -O0 -pedantic-errors
CFLAGS = -Wall -std=c++11 -c $(DEBUG)
LFLAGS = -Wall $(DEBUG)
LIBS   = -lncurses -lmenu

all: main

main: main.o editor.o
	$(CC) $(LFLAGS) -o editor main.o editor.o $(LIBS)

main.o: main.cpp editor.h
	$(CC) -c $(CFLAGS) -o main.o main.cpp

editor.o: editor.cpp editor.h
	$(CC) -c $(CFLAGS) $(LIBS) -o editor.o editor.cpp

clean:
	rm -rf *.o
	rm -rf editor

