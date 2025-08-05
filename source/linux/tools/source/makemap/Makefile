#                        Makefile for The Makemap utility

SPANDEX = ../../../dev_kit
TOOLS = ../..

#U_D = n     # Debug options?
#U_R = y     # Release flag?
#U_Z = n     # Z-Buffer flag?
#U_A = y     # ASM?

makemap: makemap.o $(SPANDEX)/libspandex.a
	g++ makemap.o -o makemap -L$(SPANDEX) -lspandex
	mv makemap $(TOOLS)
	rm -f *.o

clean:
	rm -f $(TOOLS)/makemap makemap.o

makemap.o: makemap.c
	g++ -O2 -Wall -DU_R -c makemap.c -I$(SPANDEX)
