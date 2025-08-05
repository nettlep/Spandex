#                        Makefile for The CBum utility

SPANDEX = ../../../dev_kit
TOOLS = ../..

#U_D = n     # Debug options?
#U_R = y     # Release flag?
#U_Z = n     # Z-Buffer flag?
#U_A = y     # ASM?

cbum: cbum.o $(SPANDEX)/libspandex.a
	g++ cbum.o -o cbum -L$(SPANDEX) -lspandex
	rm -f *.o
	mv -f cbum $(TOOLS)

clean:
	rm -f *.o $(TOOLS)/cbum

cbum.o: cbum.c
	g++ -c -O2 cbum.c -I$(SPANDEX)
