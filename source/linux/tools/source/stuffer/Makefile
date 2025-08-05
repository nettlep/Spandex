#                        Makefile for The Stuffer utility

SPANDEX = ../../../dev_kit
TOOLS = ../..

#U_D = n     # Debug options?
#U_R = y     # Release flag?
#U_Z = n     # Z-Buffer flag?
#U_A = y     # ASM?

stuffer: stuffer.o $(SPANDEX)/libspandex.a
	g++ stuffer.o -o stuffer -L$(SPANDEX) -lspandex
	mv stuffer $(TOOLS)
	rm -f *.o

clean:
	rm -f stuffer stuffer.o

stuffer.o: stuffer.c
	g++ -c stuffer.c -O2 -Wall -DU_R -I$(SPANDEX)
