# Makefile for 3DS2USD

TOOLS=../..

CC = g++ -Wall -O2

3ds2usd: 3ds2usd.o
	g++ 3ds2usd.o -o 3ds2usd
	rm -f *.o
	mv -f 3ds2usd $(TOOLS)

3ds2usd.o: 3ds2usd.c 3ds2usd.h

clean:
	rm -f *.o $(TOOLS)/3ds2usd
