#                        Makefile for The CUsd utility

SPANDEX = ../../../dev_kit
TOOLS = ../..

#U_D = n     # Debug options?
#U_R = y     # Release flag?
#U_Z = n     # Z-Buffer flag?
#U_A = y     # ASM?

cusd: cusd.o $(SPANDEX)/libspandex.a
	g++ cusd.o -o cusd -L$(SPANDEX) -lspandex
	mv cusd $(TOOLS)
	rm -f *.o

clean:
	rm -f $(TOOLS)/cusd *.o

cusd.o: cusd.c
	g++ -c cusd.c -O2 -Wall -I$(SPANDEX)
