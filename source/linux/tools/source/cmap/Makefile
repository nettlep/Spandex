#                        Makefile for The CBum utility

SPANDEX = ../../../dev_kit
TOOLS = ../..

cmap: cmap.o $(SPANDEX)/libspandex.a
	g++ cmap.o -o cmap -L$(SPANDEX) -lspandex
	mv cmap $(TOOLS)
	rm *.o

clean:
	rm -f $(TOOLS)/cmap *.o

cmap.o: cmap.c
	g++ cmap.c -c -O2 -Wall -I$(SPANDEX)
