# COMP2521 21T2 Assignment 1

# !!! DO NOT MODIFY THIS FILE !!!

CC = gcc
CFLAGS = -Wall -Werror -g

all: tw stem testDict

tw: tw.o Dict.o stemmer.o
stem: stem.o stemmer.o
testDict: testDict.o Dict.o

tw.o: tw.c Dict.h WFreq.h stemmer.h 
Dict.o: Dict.c Dict.h WFreq.h
stemmer.o: stemmer.c
stem.o: stem.c stemmer.h
testDict.o: testDict.c Dict.h

clean:
	rm -f tw stem testDict *.o core

