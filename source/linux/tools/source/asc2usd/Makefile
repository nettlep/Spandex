# Makefile for ASC2USD

TOOLS=../..

CC = g++ -Wall -O2

asc2usd: asc2usd.o
	g++ asc2usd.o -o asc2usd
	rm -f *.o
	mv -f asc2usd $(TOOLS)

asc2usd.o: asc2usd.c asc2usd.h

clean:
	rm -f *.o $(TOOLS)/asc2usd
